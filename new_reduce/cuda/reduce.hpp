#ifndef NEW_REDUCE_CUDA_REDUCE_HPP
#define NEW_REDUCE_CUDA_REDUCE_HPP


#if defined(RAJA_ENABLE_CUDA)

#include <cuda.h>
#include "RAJA/policy/cuda/MemUtils_CUDA.hpp"

namespace detail {
using cuda_dim_t = dim3;

  template<typename EXEC_POL,
           typename OP,
           typename T>
  camp::concepts::enable_if< std::is_same< EXEC_POL, RAJA::cuda_exec<256> > >
  init(Reducer<OP, T>& red, const RAJA::cuda::detail::cudaInfo & cs) {
    cudaMallocManaged( (void**)(&(red.cudaval)), sizeof(T));//, cudaHostAllocPortable );
    int numThreads = cs.blockDim.x * cs.blockDim.y * cs.blockDim.z;
    printf("num threads : %d\n", numThreads);

    *red.cudaval = Reducer<OP,T>::op::identity();

    red.device_mem.allocate(cs.gridDim.x * cs.gridDim.y * cs.gridDim.z);
    red.device_count = RAJA::cuda::device_zeroed_mempool_type::getInstance().template malloc<unsigned int>(1);
    printf("host cudaval %p\n", (red.cudaval));
    printf("host val %p\n\n", (&red.val));
  }


  //! reduce values in block into thread 0
  template <typename Combiner, typename T>
  RAJA_DEVICE RAJA_INLINE T block_reduce(T val, T identity)
  {
    int numThreads = blockDim.x * blockDim.y * blockDim.z;

    int threadId = threadIdx.x + blockDim.x * threadIdx.y +
                   (blockDim.x * blockDim.y) * threadIdx.z;

    int warpId = threadId % RAJA::policy::cuda::WARP_SIZE;
    int warpNum = threadId / RAJA::policy::cuda::WARP_SIZE;

    T temp = val;

    if (numThreads % RAJA::policy::cuda::WARP_SIZE == 0) {

      // reduce each warp
      for (int i = 1; i < RAJA::policy::cuda::WARP_SIZE; i *= 2) {
        T rhs = RAJA::cuda::impl::shfl_xor_sync(temp, i);
        temp = Combiner{}(temp, rhs);
      }

    } else {

      // reduce each warp
      for (int i = 1; i < RAJA::policy::cuda::WARP_SIZE; i *= 2) {
        int srcLane = threadId ^ i;
        T rhs = RAJA::cuda::impl::shfl_sync(temp, srcLane);
        // only add from threads that exist (don't double count own value)
        if (srcLane < numThreads) {
          temp = Combiner{}(temp, rhs);
        }
      }
    }

    // reduce per warp values
    if (numThreads > RAJA::policy::cuda::WARP_SIZE) {

      // Need to separate declaration and initialization for clang-cuda
      __shared__ unsigned char tmpsd[sizeof(RAJA::detail::SoAArray<T, RAJA::policy::cuda::MAX_WARPS>)];

      // Partial placement new: Should call new(tmpsd) here but recasting memory
      // to avoid calling constructor/destructor in shared memory.
      RAJA::detail::SoAArray<T, RAJA::policy::cuda::MAX_WARPS> * sd = reinterpret_cast<RAJA::detail::SoAArray<T, RAJA::policy::cuda::MAX_WARPS> *>(tmpsd);

      // write per warp values to shared memory
      if (warpId == 0) {
        sd->set(warpNum, temp);
      }

      __syncthreads();

      if (warpNum == 0) {

        // read per warp values
        if (warpId * RAJA::policy::cuda::WARP_SIZE < numThreads) {
          temp = sd->get(warpId);
        } else {
          temp = identity;
        }

        for (int i = 1; i < RAJA::policy::cuda::WARP_SIZE; i *= 2) {
          T rhs = RAJA::cuda::impl::shfl_xor_sync(temp, i);
          temp = Combiner{}(temp, rhs);
        }
      }

      __syncthreads();
    }

    return temp;
  }

  template <typename Combiner, typename OP, typename T>
  RAJA_DEVICE RAJA_INLINE bool grid_reduce(Reducer<OP, T>& red) {

    int numBlocks = gridDim.x * gridDim.y * gridDim.z;
    int numThreads = blockDim.x * blockDim.y * blockDim.z;
    unsigned int wrap_around = numBlocks - 1;

    int blockId = blockIdx.x + gridDim.x * blockIdx.y +
                  (gridDim.x * gridDim.y) * blockIdx.z;

    int threadId = threadIdx.x + blockDim.x * threadIdx.y +
                   (blockDim.x * blockDim.y) * threadIdx.z;

    T temp = block_reduce<Combiner>(*(red.cudaval), Combiner::identity());

    // one thread per block writes to device_mem
    bool lastBlock = false;
    if (threadId == 0) {
      red.device_mem.set(blockId, temp);
      // ensure write visible to all threadblocks
      __threadfence();
      // increment counter, (wraps back to zero if old count == wrap_around)
      unsigned int old_count = ::atomicInc(red.device_count, wrap_around);
      lastBlock = (old_count == wrap_around);
      //printf("gridreduce after blockred temp %f\n", temp);
    }

    // returns non-zero value if any thread passes in a non-zero value
    lastBlock = __syncthreads_or(lastBlock);

    // last block accumulates values from device_mem
    if (lastBlock) {
      temp = Combiner::identity();

      for (int i = threadId; i < numBlocks; i += numThreads) {
        //printf("device mem %f\n", (double)(red.device_mem.get(i)));
        temp = Combiner{}(temp, red.device_mem.get(i));
      }

      temp = block_reduce<Combiner>(temp, Combiner::identity());

      // one thread returns value
      if (threadId == 0) {
        printf("temp val : %f\n", (double)temp);
        //printf("num threads : %d\n", numThreads);
        *(red.cudaval) = temp;
      }
    }

    return lastBlock && threadId == 0;
  }

  // Combine
  template<typename EXEC_POL, typename OP, typename T>
  RAJA_HOST_DEVICE
  camp::concepts::enable_if<std::is_same< EXEC_POL, RAJA::cuda_exec<256>> >
  combine(Reducer<OP, T>& red) {

  // TODO : Check if we still need this?
//#if !defined(RAJA_DEVICE_CODE)
//#else
    bool blah = grid_reduce<Reducer<OP,T>::op>(red);
    if ( blah )
    {
      printf("device cudaval %p\n", (red.cudaval));
      printf("device cudaval %f\n\n", (double)(*red.cudaval));
    }
//#endif
  }
  
  // Resolve
  template<typename EXEC_POL, typename OP, typename T>
  camp::concepts::enable_if< std::is_same< EXEC_POL, RAJA::cuda_exec<256>> >
  resolve(Reducer<OP, T>& red) {
    cudaDeviceSynchronize();
    //cudaMemcpy(red.target, red.cudaval, sizeof(T), cudaMemcpyDeviceToHost);
    cudaMemcpy(&red.val, red.cudaval, sizeof(T), cudaMemcpyDeviceToHost);
    printf("host cudaval %p\n", (red.cudaval));
    printf("host val %p\n\n", (&red.val));
    *red.target = red.val; 
  }

} //  namespace detail
#endif

#endif //  NEW_REDUCE_CUDA_REDUCE_HPP
