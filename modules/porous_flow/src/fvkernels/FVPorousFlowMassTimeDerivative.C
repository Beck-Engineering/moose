//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVPorousFlowMassTimeDerivative.h"
#include "PorousFlowDictator.h"

registerADMooseObject("PorousFlowApp", FVPorousFlowMassTimeDerivative);

InputParameters
FVPorousFlowMassTimeDerivative::validParams()
{
  InputParameters params = FVTimeKernel::validParams();
  params.addRequiredParam<UserObjectName>("PorousFlowDictator",
                                          "The PorousFlowDictator UserObject");
  params.addParam<unsigned int>("fluid_component", 0, "The fluid component for this kernel");
  params.addClassDescription("Derivative of fluid-component mass with respect to time");
  return params;
}

FVPorousFlowMassTimeDerivative::FVPorousFlowMassTimeDerivative(const InputParameters & parameters)
  : FVTimeKernel(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _num_phases(_dictator.numPhases()),
    _fluid_component(getParam<unsigned int>("fluid_component")),
    _porosity(getADMaterialProperty<Real>("PorousFlow_porosity_qp")),
    _porosity_old(getMaterialPropertyOld<Real>("PorousFlow_porosity_qp")),
    _density(getADMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_qp")),
    _density_old(getMaterialPropertyOld<std::vector<Real>>("PorousFlow_fluid_phase_density_qp")),
    _saturation(getADMaterialProperty<std::vector<Real>>("PorousFlow_saturation_qp")),
    _saturation_old(getMaterialPropertyOld<std::vector<Real>>("PorousFlow_saturation_qp")),
    _mass_fractions(
        getADMaterialProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_qp")),
    _mass_fractions_old(
        getMaterialPropertyOld<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_qp"))
{
  if (_fluid_component >= _dictator.numComponents())
    paramError(
        "fluid_component",
        "The Dictator proclaims that the maximum fluid component index in this simulation is ",
        _dictator.numComponents() - 1,
        " whereas you have used ",
        _fluid_component,
        ". Remember that indexing starts at 0. The Dictator does not take such mistakes lightly.");
}

ADReal
FVPorousFlowMassTimeDerivative::computeQpResidual()
{
  ADReal mass = 0.0;
  Real mass_old = 0.0;

  for (const auto p : make_range(_num_phases))
  {
    mass += _density[_qp][p] * _saturation[_qp][p] * _mass_fractions[_qp][p][_fluid_component];
    mass_old += _density_old[_qp][p] * _saturation_old[_qp][p] *
                _mass_fractions_old[_qp][p][_fluid_component];
  }

  return (_porosity[_qp] * mass - _porosity_old[_qp] * mass_old) / _dt;
}
