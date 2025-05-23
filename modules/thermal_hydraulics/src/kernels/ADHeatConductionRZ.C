//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHeatConductionRZ.h"

registerMooseObject("ThermalHydraulicsApp", ADHeatConductionRZ);

InputParameters
ADHeatConductionRZ::validParams()
{
  InputParameters params = ADHeatConduction::validParams();
  params += RZSymmetry::validParams();
  params.addClassDescription(
      "Adds a heat conduction term in XY coordinates interpreted as cylindrical coordinates");
  return params;
}

ADHeatConductionRZ::ADHeatConductionRZ(const InputParameters & parameters)
  : ADHeatConduction(parameters), RZSymmetry(this, parameters)
{
}

ADRealVectorValue
ADHeatConductionRZ::precomputeQpResidual()
{
  const ADReal circumference = computeCircumference(_q_point[_qp]);
  return circumference * ADHeatConduction::precomputeQpResidual();
}
