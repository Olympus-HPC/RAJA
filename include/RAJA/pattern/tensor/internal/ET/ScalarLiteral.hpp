/*!
 ******************************************************************************
 *
 * \file
 *
 * \brief   RAJA header file defining SIMD/SIMT register operations.
 *
 ******************************************************************************
 */

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016-19, Lawrence Livermore National Security, LLC
// and RAJA project contributors. See the RAJA/COPYRIGHT file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#ifndef RAJA_pattern_tensor_ET_ScalarLiteral_HPP
#define RAJA_pattern_tensor_ET_ScalarLiteral_HPP

#include "RAJA/config.hpp"

#include "RAJA/util/macros.hpp"

#include "RAJA/pattern/tensor/internal/ET/ExpressionTemplateBase.hpp"


namespace RAJA
{

  namespace internal
  {

  namespace ET
  {


    template<typename T>
    class TensorScalarLiteral :  public TensorExpressionBase<TensorScalarLiteral<ScalarRegister<T>>> {
      public:
        using self_type = TensorScalarLiteral<T>;
        using tensor_type = ScalarRegister<T>;
        using element_type = T;
        using result_type = T;
        using index_type = RAJA::Index_type;

        static constexpr camp::idx_t s_num_dims = 0;

        RAJA_INLINE
        RAJA_HOST_DEVICE
        constexpr
        index_type getDimSize(index_type ) const {
          return 0;
        }

        RAJA_INLINE
        RAJA_HOST_DEVICE
        explicit
        TensorScalarLiteral(element_type const &value) :
        m_value{value}
        {}


        template<typename STORAGE, typename TILE_TYPE>
        RAJA_INLINE
        RAJA_HOST_DEVICE
        void eval(STORAGE &result, TILE_TYPE const &) const {
          result = m_value;
        }

        RAJA_INLINE
        RAJA_HOST_DEVICE
        void print_ast() const {
          printf("ScalarLiteral(%e)", (double)m_value);
        }

      private:
        element_type m_value;
    };


    /*
     * For arithmetic values, we need to wrap in a constant value ET node
     */
    template<typename RHS>
    struct NormalizeOperandHelper<RHS,
    typename std::enable_if<std::is_arithmetic<RHS>::value>::type>
    {
        using return_type = TensorScalarLiteral<RHS>;

        RAJA_INLINE
        RAJA_HOST_DEVICE
        static
        constexpr
        return_type normalize(RHS const &rhs){
          return return_type(rhs);
        }
    };


  } // namespace ET

  } // namespace internal

}  // namespace RAJA


#endif
