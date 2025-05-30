//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

TEST(MySampleTests, descriptiveTestName)
{
  // compare equality
  EXPECT_EQ(2, 1 + 1);
  EXPECT_DOUBLE_EQ(2 * 3.5, 1.0 * 8 - 1);
}

TEST(MySampleTests, anotherTest) { EXPECT_LE(1, 2); }
