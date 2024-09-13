//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016-24, Lawrence Livermore National Security, LLC
// and RAJA project contributors. See the RAJA/LICENSE file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#include <cstdlib>
#include <iostream>
#include <limits>

#include "RAJA/RAJA.hpp"

/*
 *  Reduction Example
 *
 *  This example illustrates use of the RAJA reduction types: min, max,
 *  sum, min-loc, and max-loc.
 *
 *  RAJA features shown:
 *    - `forall` loop iteration template method
 *    -  Index range segment
 *    -  Execution policies
 *    -  Reduction types
 *
 * If CUDA is enabled, CUDA unified memory is used.
 */

/*
  Specify the number of threads in a GPU thread block
*/
#if defined(RAJA_ENABLE_CUDA)
constexpr int CUDA_BLOCK_SIZE = 256;
#endif

#if defined(RAJA_ENABLE_HIP)
constexpr int HIP_BLOCK_SIZE = 256;
#endif

#if defined(RAJA_ENABLE_SYCL)
constexpr int SYCL_BLOCK_SIZE = 256;
#endif

int main(int RAJA_UNUSED_ARG(argc), char** RAJA_UNUSED_ARG(argv[]))
{

  std::cout << "\n\nRAJA reductions example...\n";

  // _reductions_array_init_start
//
// Define array length
//
  constexpr int N = 1000000;

//
// Allocate array data and initialize data to alternating sequence of 1, -1.
//
  RAJA::resources::Host host_res;
  int* a = host_res.allocate<int>(N);

  for (int i = 0; i < N; ++i) {
    if ( i % 2 == 0 ) {
      a[i] = 1;
    } else {
      a[i] = -1;
    }
  }

//
// Set a[0] to a different value. Total sum should be 2.
//
  a[0] = 3;

//
// Set min and max loc values
//
  constexpr int minloc_ref = N / 2;
  a[minloc_ref] = -100;

  constexpr int maxloc_ref = N / 2 + 1;
  a[maxloc_ref] = 100;
  // _reductions_array_init_end

//
// Note: with this data initialization scheme, the following results will
//       be observed for all reduction kernels below:
//
//  - the sum will be two
//  - the min will be -100
//  - the max will be 100
//  - the min loc will be N/2
//  - the max loc will be N/2 + 1
//
//

//
// Define index range for iterating over a elements in all examples
//
  // _reductions_range_start
  RAJA::TypedRangeSegment<int> arange(0, N);
  // _reductions_range_end

//
// Define ValLoc Type
//

  using VALLOC_INT = RAJA::expt::ValLoc<int, RAJA::Index_type>;

//
// Define ValLoc Safe Type
//

  using REF_INT_SUM = RAJA::expt::ValOp<int, RAJA::operators::plus>;
  using REF_INT_MIN = RAJA::expt::ValOp<int, RAJA::operators::minimum>;
  using REF_INT_MAX = RAJA::expt::ValOp<int, RAJA::operators::maximum>;
  using REFLOC_INT_MIN = RAJA::expt::ValLocOp<int, RAJA::Index_type, RAJA::operators::minimum>;
  using REFLOC_INT_MAX = RAJA::expt::ValLocOp<int, RAJA::Index_type, RAJA::operators::maximum>;

//----------------------------------------------------------------------------//

  std::cout << "\n Running RAJA sequential reductions...\n";

  // _reductions_raja_seq_start
  using EXEC_POL1   = RAJA::seq_exec;

  int seq_sum = 0;
  int seq_min = std::numeric_limits<int>::max();
  int seq_max = std::numeric_limits<int>::min();
  VALLOC_INT seq_minloc(std::numeric_limits<int>::max(), -1);
  VALLOC_INT seq_maxloc(std::numeric_limits<int>::min(), -1);

  RAJA::forall<EXEC_POL1>(host_res, arange,
    RAJA::expt::Reduce<RAJA::operators::plus>(&seq_sum),
    RAJA::expt::Reduce<RAJA::operators::minimum>(&seq_min),
    RAJA::expt::Reduce<RAJA::operators::maximum>(&seq_max),
    RAJA::expt::Reduce<RAJA::operators::minimum>(&seq_minloc),
    RAJA::expt::Reduce<RAJA::operators::maximum>(&seq_maxloc),
    RAJA::expt::KernelName("RAJA Reduce Seq Kernel"),
    [=](int i, REF_INT_SUM &_seq_sum, REF_INT_MIN &_seq_min, REF_INT_MAX &_seq_max, REFLOC_INT_MIN &_seq_minloc, REFLOC_INT_MAX &_seq_maxloc) {
      _seq_sum += a[i];

      _seq_min.min(a[i]);
      _seq_max.max(a[i]);

      _seq_minloc.minloc(a[i], i);
      _seq_maxloc.maxloc(a[i], i);
    }
  );

  std::cout << "\tsum = " << seq_sum << std::endl;
  std::cout << "\tmin = " << seq_min << std::endl;
  std::cout << "\tmax = " << seq_max << std::endl;
  std::cout << "\tmin, loc = " << seq_minloc.getVal() << " , "
                               << seq_minloc.getLoc() << std::endl;
  std::cout << "\tmax, loc = " << seq_maxloc.getVal() << " , "
                               << seq_maxloc.getLoc() << std::endl;
  // _reductions_raja_seq_end


//----------------------------------------------------------------------------//

#if defined(RAJA_ENABLE_OPENMP)
  std::cout << "\n Running RAJA OpenMP reductions...\n";

  // _reductions_raja_omppolicy_start
  using EXEC_POL2   = RAJA::omp_parallel_for_exec;
  // _reductions_raja_omppolicy_end

  int omp_sum = 0;
  int omp_min = std::numeric_limits<int>::max();
  int omp_max = std::numeric_limits<int>::min();
  VALLOC_INT omp_minloc(std::numeric_limits<int>::max(), -1);
  VALLOC_INT omp_maxloc(std::numeric_limits<int>::min(), -1);

  RAJA::forall<EXEC_POL2>(host_res, arange,
    RAJA::expt::Reduce<RAJA::operators::plus>(&omp_sum),
    RAJA::expt::Reduce<RAJA::operators::minimum>(&omp_min),
    RAJA::expt::Reduce<RAJA::operators::maximum>(&omp_max),
    RAJA::expt::Reduce<RAJA::operators::minimum>(&omp_minloc),
    RAJA::expt::Reduce<RAJA::operators::maximum>(&omp_maxloc),
    RAJA::expt::KernelName("RAJA Reduce OpenMP Kernel"),
    [=](int i, REF_INT_SUM &_omp_sum, REF_INT_MIN &_omp_min, REF_INT_MAX &_omp_max, REFLOC_INT_MIN &_omp_minloc, REFLOC_INT_MAX &_omp_maxloc) {
      _omp_sum += a[i];

      _omp_min.min(a[i]);
      _omp_max.max(a[i]);

      _omp_minloc.minloc(a[i], i);
      _omp_maxloc.maxloc(a[i], i);
    }
  );

  std::cout << "\tsum = " << omp_sum << std::endl;
  std::cout << "\tmin = " << omp_min << std::endl;
  std::cout << "\tmax = " << omp_max << std::endl;
  std::cout << "\tmin, loc = " << omp_minloc.getVal() << " , "
                               << omp_minloc.getLoc() << std::endl;
  std::cout << "\tmax, loc = " << omp_maxloc.getVal() << " , "
                               << omp_maxloc.getLoc() << std::endl;

#endif

//----------------------------------------------------------------------------//

#if defined(RAJA_ENABLE_TARGET_OPENMP)
  std::cout << "\n Running RAJA OpenMP Target reductions...\n";

  RAJA::resources::Omp omp_res;

  // _reductions_raja_omppolicy_start
  using EXEC_POL3   = RAJA::omp_target_parallel_for_exec_nt;
  // _reductions_raja_omppolicy_end

  int omp_t_sum = 0;
  int omp_t_min = std::numeric_limits<int>::max();
  int omp_t_max = std::numeric_limits<int>::min();
  VALLOC_INT omp_t_minloc(std::numeric_limits<int>::max(), -1);
  VALLOC_INT omp_t_maxloc(std::numeric_limits<int>::min(), -1);

  RAJA::forall<EXEC_POL3>(omp_res, arange,
    RAJA::expt::Reduce<RAJA::operators::plus>(&omp_t_sum),
    RAJA::expt::Reduce<RAJA::operators::minimum>(&omp_t_min),
    RAJA::expt::Reduce<RAJA::operators::maximum>(&omp_t_max),
    RAJA::expt::Reduce<RAJA::operators::minimum>(&omp_t_minloc),
    RAJA::expt::Reduce<RAJA::operators::maximum>(&omp_t_maxloc),
    RAJA::expt::KernelName("RAJA Reduce Target OpenMP Kernel"),
    [=](int i, REF_INT_SUM &_omp_t_sum, REF_INT_MIN &_omp_t_min, REF_INT_MAX &_omp_t_max, REFLOC_INT_MIN &_omp_t_minloc, REFLOC_INT_MAX &_omp_t_maxloc) {
      _omp_t_sum += a[i];

      _omp_t_min.min(a[i]);
      _omp_t_max.max(a[i]);

      _omp_t_minloc.minloc(a[i], i);
      _omp_t_maxloc.maxloc(a[i], i);
    }
  );

  std::cout << "\tsum = " << omp_t_sum << std::endl;
  std::cout << "\tmin = " << omp_t_min << std::endl;
  std::cout << "\tmax = " << omp_t_max << std::endl;
  std::cout << "\tmin, loc = " << omp_t_minloc.getVal() << " , "
                               << omp_t_minloc.getLoc() << std::endl;
  std::cout << "\tmax, loc = " << omp_t_maxloc.getVal() << " , "
                               << omp_t_maxloc.getLoc() << std::endl;

#endif


//----------------------------------------------------------------------------//

#if defined(RAJA_ENABLE_CUDA)
  std::cout << "\n Running RAJA CUDA reductions...\n";

  RAJA::resources::Cuda cuda_res;

  int* d_a = cuda_res.allocate<int>(N);
  cuda_res.memcpy(d_a, a, sizeof(int) * N);

  // _reductions_raja_cudapolicy_start
  using EXEC_POL3   = RAJA::cuda_exec<CUDA_BLOCK_SIZE>;
  // _reductions_raja_cudapolicy_end

  int cuda_sum = 0;
  int cuda_min = std::numeric_limits<int>::max();
  int cuda_max = std::numeric_limits<int>::min();
  VALLOC_INT cuda_minloc(std::numeric_limits<int>::max(), -1);
  VALLOC_INT cuda_maxloc(std::numeric_limits<int>::min(), -1);

  RAJA::forall<EXEC_POL3>(cuda_res, arange,
    RAJA::expt::Reduce<RAJA::operators::plus>(&cuda_sum),
    RAJA::expt::Reduce<RAJA::operators::minimum>(&cuda_min),
    RAJA::expt::Reduce<RAJA::operators::maximum>(&cuda_max),
    RAJA::expt::Reduce<RAJA::operators::minimum>(&cuda_minloc),
    RAJA::expt::Reduce<RAJA::operators::maximum>(&cuda_maxloc),
    RAJA::expt::KernelName("RAJA Reduce CUDA Kernel"),
    [=] RAJA_DEVICE (int i, REF_INT_SUM &_cuda_sum, REF_INT_MIN &_cuda_min, REF_INT_MAX &_cuda_max, REFLOC_INT_MIN &_cuda_minloc, REFLOC_INT_MAX &_cuda_maxloc) {
      _cuda_sum += d_a[i];

      _cuda_min.min(d_a[i]);
      _cuda_max.max(d_a[i]);

      _cuda_minloc.minloc(a[i], i);
      _cuda_maxloc.maxloc(a[i], i);
    }
  );

  std::cout << "\tsum = " << cuda_sum << std::endl;
  std::cout << "\tmin = " << cuda_min << std::endl;
  std::cout << "\tmax = " << cuda_max << std::endl;
  std::cout << "\tmin, loc = " << cuda_minloc.getVal() << " , "
                               << cuda_minloc.getLoc() << std::endl;
  std::cout << "\tmax, loc = " << cuda_maxloc.getVal() << " , "
                               << cuda_maxloc.getLoc() << std::endl;
  cuda_res.deallocate(d_a);
#endif

//----------------------------------------------------------------------------//

#if defined(RAJA_ENABLE_HIP)
  std::cout << "\n Running RAJA HIP reductions...\n";

  RAJA::resources::Hip hip_res;

  int* d_a = hip_res.allocate<int>(N);
  hip_res.memcpy(d_a, a, sizeof(int) * N);

  // _reductions_raja_hippolicy_start
  using EXEC_POL3   = RAJA::hip_exec<HIP_BLOCK_SIZE>;
  // _reductions_raja_hippolicy_end

  int hip_sum = 0;
  int hip_min = std::numeric_limits<int>::max();
  int hip_max = std::numeric_limits<int>::min();
  VALLOC_INT hip_minloc(std::numeric_limits<int>::max(), -1);
  VALLOC_INT hip_maxloc(std::numeric_limits<int>::min(), -1);

  RAJA::forall<EXEC_POL3>(hip_res, arange,
    RAJA::expt::Reduce<RAJA::operators::plus>(&hip_sum),
    RAJA::expt::Reduce<RAJA::operators::minimum>(&hip_min),
    RAJA::expt::Reduce<RAJA::operators::maximum>(&hip_max),
    RAJA::expt::Reduce<RAJA::operators::minimum>(&hip_minloc),
    RAJA::expt::Reduce<RAJA::operators::maximum>(&hip_maxloc),
    RAJA::expt::KernelName("RAJA Reduce HIP Kernel"),
    [=] RAJA_DEVICE (int i, REF_INT_SUM &_hip_sum, REF_INT_MIN &_hip_min, REF_INT_MAX &_hip_max, REFLOC_INT_MIN &_hip_minloc, REFLOC_INT_MAX &_hip_maxloc) {
      _hip_sum += d_a[i];

      _hip_min.min(d_a[i]);
      _hip_max.max(d_a[i]);

      _hip_minloc.minloc(d_a[i], i);
      _hip_maxloc.maxloc(d_a[i], i);
    }
  );

  std::cout << "\tsum = " << hip_sum << std::endl;
  std::cout << "\tmin = " << hip_min << std::endl;
  std::cout << "\tmax = " << hip_max << std::endl;
  std::cout << "\tmin, loc = " << hip_minloc.getVal() << " , "
                               << hip_minloc.getLoc() << std::endl;
  std::cout << "\tmax, loc = " << hip_maxloc.getVal() << " , "
                               << hip_maxloc.getLoc() << std::endl;

  hip_res.deallocate(d_a);
#endif

//----------------------------------------------------------------------------//

#if defined(RAJA_ENABLE_SYCL)
  std::cout << "\n Running RAJA SYCL reductions...\n";

  RAJA::resources::Sycl sycl_res;

  int* d_a = sycl_res.allocate<int>(N);
  sycl_res.memcpy(d_a, a, sizeof(int) * N);

  // _reductions_raja_syclpolicy_start
  using EXEC_POL3   = RAJA::sycl_exec<SYCL_BLOCK_SIZE>;
  // _reductions_raja_syclpolicy_end

  int sycl_sum = 0;
  int sycl_min = std::numeric_limits<int>::max();
  int sycl_max = std::numeric_limits<int>::min();
  VALLOC_INT sycl_minloc(std::numeric_limits<int>::max(), -1);
  VALLOC_INT sycl_maxloc(std::numeric_limits<int>::min(), -1);

  RAJA::forall<EXEC_POL3>(sycl_res, arange,
    RAJA::expt::Reduce<RAJA::operators::plus>(&sycl_sum),
    RAJA::expt::Reduce<RAJA::operators::minimum>(&sycl_min),
    RAJA::expt::Reduce<RAJA::operators::maximum>(&sycl_max),
    RAJA::expt::Reduce<RAJA::operators::minimum>(&sycl_minloc),
    RAJA::expt::Reduce<RAJA::operators::maximum>(&sycl_maxloc),
    RAJA::expt::KernelName("RAJA Reduce SYCL Kernel"),
    [=] RAJA_DEVICE (int i, REF_INT_SUM &_sycl_sum, REF_INT_MIN &_sycl_min, REF_INT_MAX &_sycl_max, REFLOC_INT_MIN &_sycl_minloc, REFLOC_INT_MAX &_sycl_maxloc) {
      _sycl_sum += d_a[i];

      _sycl_min.min(d_a[i]);
      _sycl_max.max(d_a[i]);

      _sycl_minloc.minloc(d_a[i], i);
      _sycl_maxloc.maxloc(d_a[i], i);
    }
  );

  std::cout << "\tsum = " << sycl_sum << std::endl;
  std::cout << "\tmin = " << sycl_min << std::endl;
  std::cout << "\tmax = " << sycl_max << std::endl;
  std::cout << "\tmin, loc = " << sycl_minloc.getVal() << " , "
                               << sycl_minloc.getLoc() << std::endl;
  std::cout << "\tmax, loc = " << sycl_maxloc.getVal() << " , "
                               << sycl_maxloc.getLoc() << std::endl;

  sycl_res.deallocate(d_a);
#endif

//----------------------------------------------------------------------------//

//
// Clean up.
//
  host_res.deallocate(a);

  std::cout << "\n DONE!...\n";

  return 0;
}
