//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016-20, Lawrence Livermore National Security, LLC
// and RAJA project contributors. See the RAJA/COPYRIGHT file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

///
/// Source file containing tests for RAJA reducer reset.
///

#include "tests/test-reducer-reset.hpp"

#include "test-reducer-utils.hpp"

#if defined(RAJA_ENABLE_CUDA)
using CudaReducerResetTypes = Test< camp::cartesian_product<
                                                        CudaReducerPolicyList,
                                                        DataTypeList,
                                                        CudaResourceList,
                                                        CudaForoneList
                                                      >
                             >::Types;


INSTANTIATE_TYPED_TEST_CASE_P(CudaResetTest,
                              ReducerResetUnitTest,
                              CudaReducerResetTypes);
#endif
