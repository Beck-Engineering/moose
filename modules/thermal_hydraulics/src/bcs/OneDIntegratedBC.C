//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OneDIntegratedBC.h"

InputParameters
OneDIntegratedBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredParam<Real>("normal", "Component of outward normal along 1-D direction");
  params.addClassDescription(
      "Base class for integrated boundary conditions along a single direction (1D)");
  return params;
}

OneDIntegratedBC::OneDIntegratedBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _normal(getParam<Real>("normal"))
{
}
