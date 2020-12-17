/*!
 ******************************************************************************
 *
 * \file
 *
 * \brief   RAJA header file defining a SIMD register abstraction.
 *
 ******************************************************************************
 */

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016-19, Lawrence Livermore National Security, LLC
// and RAJA project contributors. See the RAJA/COPYRIGHT file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#ifdef __AVX512F__

#ifndef RAJA_policy_vector_register_avx512_int32_HPP
#define RAJA_policy_vector_register_avx512_int32_HPP

#include "RAJA/config.hpp"
#include "RAJA/util/macros.hpp"
#include "RAJA/pattern/tensor/TensorRegister.hpp"

// Include SIMD intrinsics header file
#include <immintrin.h>
#include <cmath>


namespace RAJA
{

  template<camp::idx_t SKEW>
  class TensorRegister<avx512_register, int, VectorLayout, camp::idx_seq<16>, camp::idx_seq<0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15>, SKEW> :
    public internal::TensorRegisterBase<TensorRegister<avx512_register, int, VectorLayout, camp::idx_seq<16>, camp::idx_seq<0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15>, SKEW>>
  {
    public:
      using register_policy = avx512_register;
      using self_type = TensorRegister<avx512_register, int, VectorLayout, camp::idx_seq<16>, camp::idx_seq<0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15>, SKEW>;
      using element_type = int;
      using register_type = __m512i;


    private:
      register_type m_value;

      RAJA_INLINE
      __mmask16 createMask(camp::idx_t N) const {
        // Generate a mask
				switch(N){
					case 0:  return __mmask16(0x0000);
					case 1:  return __mmask16(0x0001);
					case 2:  return __mmask16(0x0003);
					case 3:  return __mmask16(0x0007);
					case 4:  return __mmask16(0x000F);
					case 5:  return __mmask16(0x001F);
					case 6:  return __mmask16(0x003F);
					case 7:  return __mmask16(0x007F);
					case 8:  return __mmask16(0x00FF);
          case 9:  return __mmask16(0x01FF);
          case 10: return __mmask16(0x03FF);
          case 11: return __mmask16(0x07FF);
          case 12: return __mmask16(0x0FFF);
          case 13: return __mmask16(0x1FFF);
          case 14: return __mmask16(0x3FFF);
          case 15: return __mmask16(0x7FFF);
          case 16: return __mmask16(0xFFFF);
				}
				return __mmask16(0);
      }

      RAJA_INLINE
      __m512i createStridedOffsets(camp::idx_t stride) const {
        // Generate a strided offset list
				auto vstride = _mm512_set1_epi32(stride);
				auto vseq = _mm512_set_epi32(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
				return _mm512_mullo_epi32(vstride, vseq);
      }

    public:

      static constexpr camp::idx_t s_num_elem = 16;

      /*!
       * @brief Default constructor, zeros register contents
       */
			// AVX512F
      RAJA_INLINE
      TensorRegister() : m_value(_mm512_setzero_epi32()) {
      }

      /*!
       * @brief Copy constructor from underlying simd register
       */
      RAJA_INLINE
      constexpr
      explicit TensorRegister(register_type const &c) : m_value(c) {}


      /*!
       * @brief Copy constructor
       */
      RAJA_INLINE
      constexpr
      TensorRegister(self_type const &c) : m_value(c.m_value) {}

      /*!
       * @brief Copy assignment constructor
       */
      RAJA_INLINE
      self_type &operator=(self_type const &c){
        m_value = c.m_value;
        return *this;
      }

      /*!
       * @brief Construct from scalar.
       * Sets all elements to same value (broadcast).
       */
			// AVX512F
      RAJA_INLINE
      TensorRegister(element_type const &c) : m_value(_mm512_set1_epi32(c)) {}


      /*!
       * @brief Load a full register from a stride-one memory location
       *
       */
      RAJA_INLINE
      self_type &load_packed(element_type const *ptr){
			  // AVX512F
        m_value = _mm512_loadu_epi32(ptr);
        return *this;
      }

      /*!
       * @brief Partially load a register from a stride-one memory location given
       *        a run-time number of elements.
       *
       */
      RAJA_INLINE
      self_type &load_packed_n(element_type const *ptr, camp::idx_t N){
			  // AVX512F
        m_value = _mm512_mask_loadu_epi32(_mm512_setzero_epi32(), createMask(N), ptr);
        return *this;
      }

      /*!
       * @brief Gather a full register from a strided memory location
       *
       */
      RAJA_INLINE
      self_type &load_strided(element_type const *ptr, camp::idx_t stride){
			  // AVX512F
        m_value = _mm512_i32gather_epi32(createStridedOffsets(stride),
				                              ptr,
                                      sizeof(element_type));
        return *this;
      }


      /*!
       * @brief Partially load a register from a stride-one memory location given
       *        a run-time number of elements.
       *
       */
      RAJA_INLINE
      self_type &load_strided_n(element_type const *ptr, camp::idx_t stride, camp::idx_t N){
				// AVX512F
        m_value = _mm512_mask_i32gather_epi32(_mm512_setzero_epi32(),
                                      createMask(N),
                                      createStridedOffsets(stride),
                                      ptr,
                                      sizeof(element_type));
        return *this;
      }


      /*!
       * @brief Store entire register to consecutive memory locations
       *
       */
      RAJA_INLINE
      self_type const &store_packed(element_type *ptr) const{
				// AVX512F
        _mm512_storeu_epi32(ptr, m_value);
        return *this;
      }

      /*!
       * @brief Store entire register to consecutive memory locations
       *
       */
      RAJA_INLINE
      self_type const &store_packed_n(element_type *ptr, camp::idx_t N) const{
				// AVX512F
        _mm512_mask_storeu_epi32(ptr, createMask(N), m_value);
        return *this;
      }

      /*!
       * @brief Store entire register to consecutive memory locations
       *
       */
      RAJA_INLINE
      self_type const &store_strided(element_type *ptr, camp::idx_t stride) const{
				// AVX512F
				_mm512_i64scatter_epi32(ptr,
				                     createStridedOffsets(stride),
														 m_value,
														 sizeof(element_type));
        return *this;
      }


      /*!
       * @brief Store partial register to consecutive memory locations
       *
       */
      RAJA_INLINE
      self_type const &store_strided_n(element_type *ptr, camp::idx_t stride, camp::idx_t N) const{
				// AVX512F
				_mm512_mask_i64scatter_epi32(ptr,
                           				createMask(N),
				                          createStridedOffsets(stride),
																	m_value,
														      sizeof(element_type));
        return *this;
      }

      /*!
       * @brief Get scalar value from vector register
       * @param i Offset of scalar to get
       * @return Returns scalar value at i
       */
      RAJA_INLINE
      element_type get(camp::idx_t i) const
      {
				switch(i){	
					case 0: return _mm512_cvtsi512_si32(_mm512_alignr_epi32(m_value, m_value, 0));
					case 1: return _mm512_cvtsi512_si32(_mm512_alignr_epi32(m_value, m_value, 1));
					case 2: return _mm512_cvtsi512_si32(_mm512_alignr_epi32(m_value, m_value, 2));
					case 3: return _mm512_cvtsi512_si32(_mm512_alignr_epi32(m_value, m_value, 3));
					case 4: return _mm512_cvtsi512_si32(_mm512_alignr_epi32(m_value, m_value, 4));
					case 5: return _mm512_cvtsi512_si32(_mm512_alignr_epi32(m_value, m_value, 5));
					case 6: return _mm512_cvtsi512_si32(_mm512_alignr_epi32(m_value, m_value, 6));
					case 7: return _mm512_cvtsi512_si32(_mm512_alignr_epi32(m_value, m_value, 7));
					case 8: return _mm512_cvtsi512_si32(_mm512_alignr_epi32(m_value, m_value, 8));
					case 9: return _mm512_cvtsi512_si32(_mm512_alignr_epi32(m_value, m_value, 9));
					case 10: return _mm512_cvtsi512_si32(_mm512_alignr_epi32(m_value, m_value, 10));
					case 11: return _mm512_cvtsi512_si32(_mm512_alignr_epi32(m_value, m_value, 11));
					case 12: return _mm512_cvtsi512_si32(_mm512_alignr_epi32(m_value, m_value, 12));
					case 13: return _mm512_cvtsi512_si32(_mm512_alignr_epi32(m_value, m_value, 13));
					case 14: return _mm512_cvtsi512_si32(_mm512_alignr_epi32(m_value, m_value, 14));
					case 15: return _mm512_cvtsi512_si32(_mm512_alignr_epi32(m_value, m_value, 15));
				}
				return 0;
			}


      /*!
       * @brief Set scalar value in vector register
       * @param i Offset of scalar to set
       * @param value Value of scalar to set
       */
      RAJA_INLINE
      self_type &set(camp::idx_t i, element_type value)
      {
				m_value = _mm512_mask_set1_epi32(m_value, 1 << i, value);
        return *this;
      }

      RAJA_HOST_DEVICE
      RAJA_INLINE
      self_type &broadcast(element_type const &value){
        m_value =  _mm512_set1_epi32(value);
        return *this;
      }


      RAJA_HOST_DEVICE
      RAJA_INLINE
      self_type &copy(self_type const &src){
        m_value = src.m_value;
        return *this;
      }

      RAJA_HOST_DEVICE
      RAJA_INLINE
      self_type add(self_type const &b) const {
        return self_type(_mm512_add_epi32(m_value, b.m_value));
      }

      RAJA_HOST_DEVICE
      RAJA_INLINE
      self_type subtract(self_type const &b) const {
        return self_type(_mm512_sub_epi32(m_value, b.m_value));
      }

      RAJA_HOST_DEVICE
      RAJA_INLINE
      self_type multiply(self_type const &b) const {
        return self_type(_mm512_mullo_epi32(m_value, b.m_value));
      }

      RAJA_HOST_DEVICE
      RAJA_INLINE
      self_type divide(self_type const &b, camp::idx_t N = 16) const {
        // AVX512 does not supply an integer divide, so do it manually
        return self_type(_mm512_set_epi32(
            N >= 16 ? get(15)/b.get(15) : 0,
            N >= 15 ? get(14)/b.get(14) : 0,
            N >= 14 ? get(13)/b.get(13) : 0,
            N >= 13 ? get(12)/b.get(12) : 0,
            N >= 12 ? get(11)/b.get(11) : 0,
            N >= 11 ? get(10)/b.get(10) : 0,
            N >= 10 ? get(9)/b.get(9) : 0,
            N >= 9 ? get(8)/b.get(8) : 0,
            N >= 8 ? get(7)/b.get(7) : 0,
            N >= 7 ? get(6)/b.get(6) : 0,
            N >= 6 ? get(5)/b.get(5) : 0,
            N >= 5 ? get(4)/b.get(4) : 0,
            N >= 4 ? get(3)/b.get(3) : 0,
            N >= 3 ? get(2)/b.get(2) : 0,
            N >= 2 ? get(1)/b.get(1) : 0,
            N >= 1 ? get(0)/b.get(0) : 0
            ));
      }


      /*!
       * @brief Sum the elements of this vector
       * @return Sum of the values of the vectors scalar elements
       */
      RAJA_INLINE
      element_type sum(camp::idx_t N = 16) const
      {
				return _mm512_mask_reduce_add_epi32(createMask(N), m_value);
      }


      /*!
       * @brief Returns the largest element
       * @return The largest scalar element in the register
       */
      RAJA_INLINE
      element_type max(camp::idx_t N = 16) const
      {
				return _mm512_mask_reduce_max_epi32(createMask(N), m_value);
      }

      /*!
       * @brief Returns element-wise largest values
       * @return Vector of the element-wise max values
       */
      RAJA_INLINE
      self_type vmax(self_type a) const
      {
        return self_type(_mm512_max_epi32(m_value, a.m_value));
      }

      /*!
       * @brief Returns the largest element
       * @return The largest scalar element in the register
       */
      RAJA_INLINE
      element_type min(camp::idx_t N = 16) const
      {
				return _mm512_mask_reduce_min_epi32(createMask(N), m_value);
      }

      /*!
       * @brief Returns element-wise largest values
       * @return Vector of the element-wise max values
       */
      RAJA_INLINE
      self_type vmin(self_type a) const
      {
        return self_type(_mm512_min_epi32(m_value, a.m_value));
      }
  };



}  // namespace RAJA


#endif

#endif //__AVX512F__
