//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016-20, Lawrence Livermore National Security, LLC
// and RAJA project contributors. See the RAJA/COPYRIGHT file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#include "RAJA/RAJA.hpp"
#include "gtest/gtest.h"

#include "test-forall-rangesegment.hpp"

// Generate Sequential Type List
using SequentialTypes = list< RAJA::seq_exec, 
                              RAJA::loop_exec,
                              RAJA::simd_exec
                            >;

using SequentialForallTypes = Test<cartesian_product< IdxTypes, ListHost, SequentialTypes >>::Types;

INSTANTIATE_TYPED_TEST_SUITE_P(Sequential, ForallFunctionalTest, SequentialForallTypes);
