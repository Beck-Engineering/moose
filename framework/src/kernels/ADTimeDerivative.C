//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADTimeDerivative.h"

registerMooseObject("MooseApp", ADTimeDerivative);

InputParameters
ADTimeDerivative::validParams()
{
  InputParameters params = ADTimeKernelValue::validParams();
  params.addClassDescription("The time derivative operator with the weak form of $(\\psi_i, "
                             "\\frac{\\partial u_h}{\\partial t})$.");
  return params;
}

ADTimeDerivative::ADTimeDerivative(const InputParameters & parameters)
  : ADTimeKernelValue(parameters)
{
}

ADReal
ADTimeDerivative::precomputeQpResidual()
{
  return _u_dot[_qp];
}
