//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHeatConductionTimeDerivative.h"

registerMooseObject("HeatTransferApp", ADHeatConductionTimeDerivative);

InputParameters
ADHeatConductionTimeDerivative::validParams()
{
  InputParameters params = ADTimeDerivative::validParams();
  params.addClassDescription(
      "AD Time derivative term $\\rho c_p \\frac{\\partial T}{\\partial t}$ of "
      "the heat equation for quasi-constant specific heat $c_p$ and the density $\\rho$.");
  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<MaterialPropertyName>(
      "specific_heat", "specific_heat", "Property name of the specific heat material property");
  params.addParam<MaterialPropertyName>(
      "density_name", "density", "Property name of the density material property");
  return params;
}

ADHeatConductionTimeDerivative::ADHeatConductionTimeDerivative(const InputParameters & parameters)
  : ADTimeDerivative(parameters),
    _specific_heat(getADMaterialProperty<Real>("specific_heat")),
    _density(getADMaterialProperty<Real>("density_name"))
{
}

ADReal
ADHeatConductionTimeDerivative::precomputeQpResidual()
{
  return _specific_heat[_qp] * _density[_qp] * ADTimeDerivative::precomputeQpResidual();
}
