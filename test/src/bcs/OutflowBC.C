//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OutflowBC.h"
#include "Function.h"

registerMooseObject("MooseTestApp", OutflowBC);

InputParameters
OutflowBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredParam<RealVectorValue>("velocity", "The velocity vector");
  return params;
}

OutflowBC::OutflowBC(const InputParameters & parameters)
  : IntegratedBC(parameters),

    _velocity(getParam<RealVectorValue>("velocity"))
{
}

Real
OutflowBC::computeQpResidual()
{
  return _test[_i][_qp] * _u[_qp] * _velocity * _normals[_qp];
}

Real
OutflowBC::computeQpJacobian()
{
  return _test[_i][_qp] * _phi[_j][_qp] * _velocity * _normals[_qp];
}
