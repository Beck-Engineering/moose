//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADEnergyMeshAdvection.h"
#include "INSADMomentumMeshAdvection.h"

registerMooseObject("NavierStokesApp", INSADEnergyMeshAdvection);

InputParameters
INSADEnergyMeshAdvection::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params += INSADMomentumMeshAdvection::displacementParams();
  params.addClassDescription("This class computes the residual and Jacobian contributions for "
                             "temperature advection from mesh velocity in an ALE simulation.");
  return params;
}

INSADEnergyMeshAdvection::INSADEnergyMeshAdvection(const InputParameters & parameters)
  : ADKernelValue(parameters),
    _temperature_advected_mesh_strong_residual(
        getADMaterialProperty<Real>("temperature_advected_mesh_strong_residual"))
{
  INSADMomentumMeshAdvection::setDisplacementParams(*this);
}

ADReal
INSADEnergyMeshAdvection::precomputeQpResidual()
{
  return _temperature_advected_mesh_strong_residual[_qp];
}
