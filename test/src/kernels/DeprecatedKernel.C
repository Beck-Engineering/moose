//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DeprecatedKernel.h"

registerMooseObjectDeprecated("MooseTestApp", DeprecatedKernel, "01/01/2050 00:00");

InputParameters
DeprecatedKernel::validParams()
{
  InputParameters params = Reaction::validParams();
  params.addParam<Real>("coefficient", 1.0, "Coefficient of the term");
  return params;
}

DeprecatedKernel::DeprecatedKernel(const InputParameters & parameters)
  : Reaction(parameters), _coef(getParam<Real>("coefficient"))
{
}

Real
DeprecatedKernel::computeQpResidual()
{
  return _coef * Reaction::computeQpResidual();
}

Real
DeprecatedKernel::computeQpJacobian()
{
  return _coef * Reaction::computeQpJacobian();
}
