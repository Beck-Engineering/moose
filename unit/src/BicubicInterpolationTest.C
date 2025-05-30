//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "BicubicInterpolation.h"

const double tol = 1e-8;

TEST(BicubicInterpolationTest, sample)
{
  // Test BicubicInterpolation with a function y = x1^2 + 3 x2^2 + 2 * x1 * x2
  const unsigned int n = 9;
  std::vector<double> x1(n), x2(n);
  std::vector<std::vector<double>> y(n);

  for (unsigned int i = 0; i < n; ++i)
  {
    x1[i] = i;
    x2[i] = i;
    y[i].resize(n);
  }

  for (unsigned int i = 0; i < n; ++i)
    for (unsigned int j = 0; j < n; ++j)
      y[i][j] = x1[i] * x1[i] + 3.0 * x2[j] * x2[j] + 2 * x1[i] * x2[j];

  BicubicInterpolation interp(x1, x2, y);

  // Check sampled value and first and second derivatives
  Real p1 = 4.5, p2 = 5.5;
  EXPECT_NEAR(interp.sample(p1, p2), 160.5, tol);
  EXPECT_NEAR(interp.sampleDerivative(p1, p2, 1), 20., tol);
  EXPECT_NEAR(interp.sampleDerivative(p1, p2, 2), 42.0, tol);
  EXPECT_NEAR(interp.sample2ndDerivative(p1, p2, 1), 2.0, tol);
  EXPECT_NEAR(interp.sample2ndDerivative(p1, p2, 2), 6.0, tol);
  EXPECT_NEAR(interp.sample2ndDerivative(p1, p2, 3), 2.0, tol);

  // Check that sampleValueAndDerivatives() returns the same results as above
  double y2, dy2_dx1, dy2_dx2;
  interp.sampleValueAndDerivatives(p1, p2, y2, dy2_dx1, dy2_dx2);
  EXPECT_NEAR(y2, 160.5, tol);
  EXPECT_NEAR(dy2_dx1, 20.0, tol);
  EXPECT_NEAR(dy2_dx2, 42.0, tol);

  // Check that the value and derivatives are correct at grid point
  p1 = 4, p2 = 5;
  EXPECT_NEAR(interp.sample(p1, p2), 131.0, tol);
  EXPECT_NEAR(interp.sampleDerivative(p1, p2, 1), 18.0, tol);
  EXPECT_NEAR(interp.sampleDerivative(p1, p2, 2), 38.0, tol);
  EXPECT_NEAR(interp.sample2ndDerivative(p1, p2, 1), 2.0, tol);
  EXPECT_NEAR(interp.sample2ndDerivative(p1, p2, 2), 6.0, tol);
  EXPECT_NEAR(interp.sample2ndDerivative(p1, p2, 3), 2.0, tol);

  // Check AD routines
  // Check sampled value and first derivatives
  ADReal ad_p1 = 4.5, ad_p2 = 5.5;
  EXPECT_NEAR(interp.sample(ad_p1, ad_p2).value(), 160.5, tol);

  // Check that ADsampleValueAndDerivatives() returns the same results as above
  ADReal ad_y2, ad_dy2_dx1, ad_dy2_dx2;
  interp.sampleValueAndDerivatives(ad_p1, ad_p2, ad_y2, ad_dy2_dx1, ad_dy2_dx2);
  EXPECT_NEAR(ad_y2.value(), 160.5, tol);
  EXPECT_NEAR(ad_dy2_dx1.value(), 20.0, tol);
  EXPECT_NEAR(ad_dy2_dx2.value(), 42.0, tol);

  // Check ChainedReal routines
  // Check sampled value and first derivatives
  ChainedReal cr_p1 = 4.5, cr_p2 = 5.5;
  EXPECT_NEAR(interp.sample(cr_p1, cr_p2).value(), 160.5, tol);

  // Check that ADsampleValueAndDerivatives() returns the same results as above
  ChainedReal cr_y2, cr_dy2_dx1, cr_dy2_dx2;
  interp.sampleValueAndDerivatives(cr_p1, cr_p2, cr_y2, cr_dy2_dx1, cr_dy2_dx2);
  EXPECT_NEAR(cr_y2.value(), 160.5, tol);
  EXPECT_NEAR(cr_dy2_dx1.value(), 20.0, tol);
  EXPECT_NEAR(cr_dy2_dx2.value(), 42.0, tol);
}
