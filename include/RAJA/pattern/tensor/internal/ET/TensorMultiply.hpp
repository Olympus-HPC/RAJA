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

#ifndef RAJA_pattern_tensor_ET_TensorMultiply_HPP
#define RAJA_pattern_tensor_ET_TensorMultiply_HPP

#include "RAJA/config.hpp"

#include "RAJA/util/macros.hpp"

#include "RAJA/pattern/tensor/internal/ET/ExpressionTemplateBase.hpp"
#include "RAJA/pattern/tensor/internal/ET/MultiplyOperator.hpp"


namespace RAJA
{

  namespace internal
  {

  namespace ET
  {

    // forward decl for FMA contraction
    template<typename LHS_TYPE, typename RHS_TYPE, typename ADD_TYPE>
    class TensorMultiplyAdd;


    template<typename LHS_TYPE, typename RHS_TYPE>
    class TensorMultiply : public TensorExpressionBase<TensorMultiply<LHS_TYPE, RHS_TYPE>> {
      public:
        using self_type = TensorMultiply<LHS_TYPE, RHS_TYPE>;
        using lhs_type = LHS_TYPE;
        using rhs_type = RHS_TYPE;
        using element_type = typename LHS_TYPE::element_type;
        using index_type = typename LHS_TYPE::index_type;
        using multiply_op = MultiplyOperator<LHS_TYPE, RHS_TYPE>;
        using result_type = typename multiply_op::result_type;
        using tile_type = typename multiply_op::tile_type;
        static constexpr camp::idx_t s_num_dims = multiply_op::s_num_dims;


        RAJA_INLINE
        RAJA_HOST_DEVICE
        TensorMultiply(lhs_type const &lhs, rhs_type const &rhs) :
        m_lhs{lhs}, m_rhs{rhs}
        {}


        RAJA_INLINE
        RAJA_HOST_DEVICE
        constexpr
        index_type getDimSize(index_type dim) const {
          return multiply_op::getDimSize(dim, m_lhs, m_rhs);
        }


        template<typename STORAGE, typename TILE_TYPE>
        RAJA_INLINE
        RAJA_HOST_DEVICE
        void eval(STORAGE &result, TILE_TYPE const &tile) const {
          multiply_op::multiply(result, tile, m_lhs, m_rhs);
        }

        /*!
         * Returns the LHS of the operation, used to form contractions
         */
        RAJA_INLINE
        RAJA_HOST_DEVICE
        constexpr
        lhs_type const &getLHS() const {
          return m_lhs;
        }

        /*!
         * Returns the RHS of the operation, used to form contractions
         */
        RAJA_INLINE
        RAJA_HOST_DEVICE
        constexpr
        rhs_type const &getRHS() const {
          return m_rhs;
        }


        /*!
         * operator+ overload that forms a FMA contraction
         */
        template<typename ADD>
        RAJA_INLINE
        RAJA_HOST_DEVICE
        TensorMultiplyAdd<lhs_type, rhs_type, normalize_operand_t<ADD>>
        operator+(ADD const &add) const {
          return TensorMultiplyAdd<lhs_type, rhs_type, normalize_operand_t<ADD>>(m_lhs, m_rhs, normalizeOperand(add));
        }


        RAJA_INLINE
        RAJA_HOST_DEVICE
        void print_ast() const {
          printf("Multiply[");
          multiply_op::print_ast();
          printf("](");
          m_lhs.print_ast();
          printf(", ");
          m_rhs.print_ast();
          printf(")");
        }

      private:
        lhs_type m_lhs;
        rhs_type m_rhs;
    };


    /*
     * Overload for:    arithmetic * tensorexpression

     */
    template<typename LHS, typename RHS,
      typename std::enable_if<std::is_arithmetic<LHS>::value, bool>::type = true,
      typename std::enable_if<std::is_base_of<TensorExpressionConcreteBase, RHS>::value, bool>::type = true>
    RAJA_INLINE
    RAJA_HOST_DEVICE
    auto operator*(LHS const &lhs, RHS const &rhs) ->
    TensorMultiply<typename NormalizeOperandHelper<LHS>::return_type, RHS>
    {
      return TensorMultiply<typename NormalizeOperandHelper<LHS>::return_type, RHS>(NormalizeOperandHelper<LHS>::normalize(lhs), rhs);
    }

  } // namespace ET

  } // namespace internal

}  // namespace RAJA


#endif
