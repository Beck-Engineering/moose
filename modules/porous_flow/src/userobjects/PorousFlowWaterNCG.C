//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowWaterNCG.h"
#include "SinglePhaseFluidProperties.h"
#include "Water97FluidProperties.h"
#include "Conversion.h"

registerMooseObject("PorousFlowApp", PorousFlowWaterNCG);

InputParameters
PorousFlowWaterNCG::validParams()
{
  InputParameters params = PorousFlowFluidStateMultiComponentBase::validParams();
  params.addRequiredParam<UserObjectName>("water_fp", "The name of the user object for water");
  params.addRequiredParam<UserObjectName>(
      "gas_fp", "The name of the user object for the non-condensable gas");
  params.addClassDescription("Fluid state class for water and non-condensable gas");
  return params;
}

PorousFlowWaterNCG::PorousFlowWaterNCG(const InputParameters & parameters)
  : PorousFlowFluidStateMultiComponentBase(parameters),
    _water_fp(getUserObject<SinglePhaseFluidProperties>("water_fp")),
    _water97_fp(getUserObject<Water97FluidProperties>("water_fp")),
    _ncg_fp(getUserObject<SinglePhaseFluidProperties>("gas_fp")),
    _Mh2o(_water_fp.molarMass()),
    _Mncg(_ncg_fp.molarMass()),
    _water_triple_temperature(_water_fp.triplePointTemperature()),
    _water_critical_temperature(_water_fp.criticalTemperature()),
    _ncg_henry(_ncg_fp.henryCoefficients())
{
  // Check that the correct FluidProperties UserObjects have been provided
  if (_water_fp.fluidName() != "water")
    paramError("water_fp", "A valid water FluidProperties UserObject must be provided in water_fp");

  // Set the number of phases and components, and their indexes
  _num_phases = 2;
  _num_components = 2;
  _gas_phase_number = 1 - _aqueous_phase_number;
  _gas_fluid_component = 1 - _aqueous_fluid_component;

  // Check that _aqueous_phase_number is <= total number of phases
  if (_aqueous_phase_number >= _num_phases)
    paramError("liquid_phase_number",
               "This value is larger than the possible number of phases ",
               _num_phases);

  // Check that _aqueous_fluid_component is <= total number of fluid components
  if (_aqueous_fluid_component >= _num_components)
    paramError("liquid_fluid_component",
               "This value is larger than the possible number of fluid components",
               _num_components);

  _empty_fsp = FluidStateProperties(_num_components);
}

std::string
PorousFlowWaterNCG::fluidStateName() const
{
  return "water-ncg";
}

void
PorousFlowWaterNCG::thermophysicalProperties(Real pressure,
                                             Real temperature,
                                             Real Xnacl,
                                             Real Z,
                                             unsigned int qp,
                                             std::vector<FluidStateProperties> & fsp) const
{
  // Make AD versions of primary variables then call AD thermophysicalProperties()
  ADReal p = pressure;
  Moose::derivInsert(p.derivatives(), _pidx, 1.0);
  ADReal T = temperature;
  Moose::derivInsert(T.derivatives(), _Tidx, 1.0);
  ADReal Zncg = Z;
  Moose::derivInsert(Zncg.derivatives(), _Zidx, 1.0);
  // X is not used, but needed for consistency with PorousFlowFluidState interface
  ADReal X = Xnacl;
  Moose::derivInsert(X.derivatives(), _Xidx, 1.0);

  thermophysicalProperties(p, T, X, Zncg, qp, fsp);
}

void
PorousFlowWaterNCG::thermophysicalProperties(const ADReal & pressure,
                                             const ADReal & temperature,
                                             const ADReal & /* Xnacl */,
                                             const ADReal & Z,
                                             unsigned int qp,
                                             std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];
  FluidStateProperties & gas = fsp[_gas_phase_number];

  // Check whether the input temperature is within the region of validity
  checkVariables(temperature.value());

  // Clear all of the FluidStateProperties data
  clearFluidStateProperties(fsp);

  FluidStatePhaseEnum phase_state;
  massFractions(pressure, temperature, Z, phase_state, fsp);

  switch (phase_state)
  {
    case FluidStatePhaseEnum::GAS:
    {
      // Set the gas saturations
      gas.saturation = 1.0;

      // Calculate gas properties
      gasProperties(pressure, temperature, fsp);

      break;
    }

    case FluidStatePhaseEnum::LIQUID:
    {
      // Calculate the liquid properties
      const ADReal liquid_pressure = pressure - _pc.capillaryPressure(1.0, qp);
      liquidProperties(liquid_pressure, temperature, fsp);

      break;
    }

    case FluidStatePhaseEnum::TWOPHASE:
    {
      // Calculate the gas and liquid properties in the two phase region
      twoPhaseProperties(pressure, temperature, Z, qp, fsp);

      break;
    }
  }

  // Liquid saturations can now be set
  liquid.saturation = 1.0 - gas.saturation;

  // Save pressures to FluidStateProperties object
  gas.pressure = pressure;
  liquid.pressure = pressure - _pc.capillaryPressure(liquid.saturation, qp);
}

void
PorousFlowWaterNCG::massFractions(const ADReal & pressure,
                                  const ADReal & temperature,
                                  const ADReal & Z,
                                  FluidStatePhaseEnum & phase_state,
                                  std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];
  FluidStateProperties & gas = fsp[_gas_phase_number];

  // Equilibrium mass fraction of NCG in liquid and H2O in gas phases
  ADReal Xncg, Yh2o;
  equilibriumMassFractions(pressure, temperature, Xncg, Yh2o);

  ADReal Yncg = 1.0 - Yh2o;

  // Determine which phases are present based on the value of Z
  phaseState(Z.value(), Xncg.value(), Yncg.value(), phase_state);

  // The equilibrium mass fractions calculated above are only correct in the two phase
  // state. If only liquid or gas phases are present, the mass fractions are given by
  // the total mass fraction Z.
  ADReal Xh2o = 0.0;

  switch (phase_state)
  {
    case FluidStatePhaseEnum::LIQUID:
    {
      Xncg = Z;
      Yncg = 0.0;
      Xh2o = 1.0 - Z;
      Yh2o = 0.0;
      break;
    }

    case FluidStatePhaseEnum::GAS:
    {
      Xncg = 0.0;
      Yncg = Z;
      Yh2o = 1.0 - Z;
      break;
    }

    case FluidStatePhaseEnum::TWOPHASE:
    {
      // Keep equilibrium mass fractions
      Xh2o = 1.0 - Xncg;
      break;
    }
  }

  // Save the mass fractions in the FluidStateMassFractions object
  liquid.mass_fraction[_aqueous_fluid_component] = Xh2o;
  liquid.mass_fraction[_gas_fluid_component] = Xncg;
  gas.mass_fraction[_aqueous_fluid_component] = Yh2o;
  gas.mass_fraction[_gas_fluid_component] = Yncg;
}

void
PorousFlowWaterNCG::gasProperties(const ADReal & pressure,
                                  const ADReal & temperature,
                                  std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];
  FluidStateProperties & gas = fsp[_gas_phase_number];

  const ADReal psat = _water_fp.vaporPressure(temperature);

  const ADReal Yncg = gas.mass_fraction[_gas_fluid_component];
  const ADReal Xncg = liquid.mass_fraction[_gas_fluid_component];

  // NCG density, viscosity and enthalpy calculated using partial pressure
  // Yncg * gas_poreressure (Dalton's law)
  ADReal ncg_density, ncg_viscosity;
  _ncg_fp.rho_mu_from_p_T(Yncg * pressure, temperature, ncg_density, ncg_viscosity);
  ADReal ncg_enthalpy = _ncg_fp.h_from_p_T(Yncg * pressure, temperature);

  // Vapor density, viscosity and enthalpy calculated using partial pressure
  // X1 * psat (Raoult's law)
  ADReal vapor_density, vapor_viscosity;

  _water_fp.rho_mu_from_p_T((1.0 - Xncg) * psat, temperature, vapor_density, vapor_viscosity);
  ADReal vapor_enthalpy = _water_fp.h_from_p_T((1.0 - Xncg) * psat, temperature);

  // Density is just the sum of individual component densities
  gas.density = ncg_density + vapor_density;

  // Viscosity of the gas phase is a weighted sum of the individual viscosities
  gas.viscosity = Yncg * ncg_viscosity + (1.0 - Yncg) * vapor_viscosity;

  // Enthalpy of the gas phase is a weighted sum of the individual enthalpies
  gas.enthalpy = Yncg * ncg_enthalpy + (1.0 - Yncg) * vapor_enthalpy;

  //  Internal energy of the gas phase (e = h - pv)
  mooseAssert(gas.density.value() > 0.0, "Gas density must be greater than zero");
  gas.internal_energy = gas.enthalpy - pressure / gas.density;
}

void
PorousFlowWaterNCG::liquidProperties(const ADReal & pressure,
                                     const ADReal & temperature,
                                     std::vector<FluidStateProperties> & fsp) const
{
  FluidStateProperties & liquid = fsp[_aqueous_phase_number];

  // Calculate liquid density and viscosity if in the two phase or single phase
  // liquid region, assuming they are not affected by the presence of dissolved
  // NCG. Note: the (small) contribution due to derivative of capillary pressure
  // wrt pressure (using the chain rule) is not implemented.
  ADReal liquid_density, liquid_viscosity;
  _water_fp.rho_mu_from_p_T(pressure, temperature, liquid_density, liquid_viscosity);

  liquid.density = liquid_density;
  liquid.viscosity = liquid_viscosity;

  // Enthalpy does include a contribution due to the enthalpy of dissolution
  const ADReal hdis = enthalpyOfDissolution(temperature);

  const ADReal water_enthalpy = _water_fp.h_from_p_T(pressure, temperature);
  const ADReal ncg_enthalpy = _ncg_fp.h_from_p_T(pressure, temperature);

  const ADReal Xncg = liquid.mass_fraction[_gas_fluid_component];
  liquid.enthalpy = (1.0 - Xncg) * water_enthalpy + Xncg * (ncg_enthalpy + hdis);

  //  Internal energy of the liquid phase (e = h - pv)
  mooseAssert(liquid.density.value() > 0.0, "Liquid density must be greater than zero");
  liquid.internal_energy = liquid.enthalpy - pressure / liquid.density;
}

ADReal
PorousFlowWaterNCG::liquidDensity(const ADReal & pressure, const ADReal & temperature) const
{
  return _water_fp.rho_from_p_T(pressure, temperature);
}

ADReal
PorousFlowWaterNCG::gasDensity(const ADReal & pressure,
                               const ADReal & temperature,
                               std::vector<FluidStateProperties> & fsp) const
{
  auto & liquid = fsp[_aqueous_phase_number];
  auto & gas = fsp[_gas_phase_number];

  ADReal psat = _water_fp.vaporPressure(temperature);

  const ADReal Yncg = gas.mass_fraction[_gas_fluid_component];
  const ADReal Xncg = liquid.mass_fraction[_gas_fluid_component];

  ADReal ncg_density = _ncg_fp.rho_from_p_T(Yncg * pressure, temperature);
  ADReal vapor_density = _water_fp.rho_from_p_T((1.0 - Xncg) * psat, temperature);

  // Density is just the sum of individual component densities
  return ncg_density + vapor_density;
}

ADReal
PorousFlowWaterNCG::saturation(const ADReal & pressure,
                               const ADReal & temperature,
                               const ADReal & Z,
                               std::vector<FluidStateProperties> & fsp) const
{
  auto & gas = fsp[_gas_phase_number];
  auto & liquid = fsp[_aqueous_fluid_component];

  // Approximate liquid density as saturation isn't known yet, by using the gas
  // pressure rather than the liquid pressure. This does result in a small error
  // in the calculated saturation, but this is below the error associated with
  // the correlations. A more accurate saturation could be found iteraviely,
  // at the cost of increased computational expense

  // The gas and liquid densities
  const ADReal gas_density = gasDensity(pressure, temperature, fsp);
  const ADReal liquid_density = liquidDensity(pressure, temperature);

  // Set mass equilibrium constants used in the calculation of vapor mass fraction
  const ADReal Xncg = liquid.mass_fraction[_gas_fluid_component];
  const ADReal Yncg = gas.mass_fraction[_gas_fluid_component];

  const ADReal K0 = Yncg / Xncg;
  const ADReal K1 = (1.0 - Yncg) / (1.0 - Xncg);
  const ADReal vapor_mass_fraction = vaporMassFraction(Z, K0, K1);

  // The gas saturation in the two phase case
  const ADReal saturation = vapor_mass_fraction * liquid_density /
                            (gas_density + vapor_mass_fraction * (liquid_density - gas_density));

  return saturation;
}

void
PorousFlowWaterNCG::twoPhaseProperties(const ADReal & pressure,
                                       const ADReal & temperature,
                                       const ADReal & Z,
                                       unsigned int qp,
                                       std::vector<FluidStateProperties> & fsp) const
{
  auto & gas = fsp[_gas_phase_number];

  // Calculate all of the gas phase properties, as these don't depend on saturation
  gasProperties(pressure, temperature, fsp);

  // The gas saturation in the two phase case
  gas.saturation = saturation(pressure, temperature, Z, fsp);

  // The liquid pressure and properties can now be calculated
  const ADReal liquid_pressure = pressure - _pc.capillaryPressure(1.0 - gas.saturation, qp);
  liquidProperties(liquid_pressure, temperature, fsp);
}

void
PorousFlowWaterNCG::equilibriumMassFractions(const ADReal & pressure,
                                             const ADReal & temperature,
                                             ADReal & Xncg,
                                             ADReal & Yh2o) const
{
  // Equilibrium constants for each component (Henry's law for the NCG
  // component, and Raoult's law for water).
  const ADReal Kh = _water97_fp.henryConstant(temperature, _ncg_henry);
  const ADReal psat = _water_fp.vaporPressure(temperature);

  const ADReal Kncg = Kh / pressure;
  const ADReal Kh2o = psat / pressure;

  // The mole fractions for the NCG component in the two component
  // case can be expressed in terms of the equilibrium constants only
  const ADReal xncg = (1.0 - Kh2o) / (Kncg - Kh2o);
  const ADReal yncg = Kncg * xncg;

  // Convert mole fractions to mass fractions
  Xncg = moleFractionToMassFraction(xncg);
  Yh2o = 1.0 - moleFractionToMassFraction(yncg);
}

ADReal
PorousFlowWaterNCG::moleFractionToMassFraction(const ADReal & xmol) const
{
  return xmol * _Mncg / (xmol * _Mncg + (1.0 - xmol) * _Mh2o);
}

void
PorousFlowWaterNCG::checkVariables(Real temperature) const
{
  // Check whether the input temperature is within the region of validity of this equation
  // of state (T_triple <= T <= T_critical)
  if (temperature < _water_triple_temperature || temperature > _water_critical_temperature)
    mooseException(name() + ": temperature " + Moose::stringify(temperature) +
                   " is outside range 273.16 K <= T <= 647.096 K");
}

ADReal
PorousFlowWaterNCG::enthalpyOfDissolution(const ADReal & temperature) const
{
  // Henry's constant
  const ADReal Kh = _water97_fp.henryConstant(temperature, _ncg_henry);

  ADReal hdis = -_R * temperature * temperature * Kh.derivatives()[_Tidx] / Kh / _Mncg;

  // Derivative of enthalpy of dissolution wrt temperature requires the second derivative of
  // Henry's constant wrt temperature. For simplicity, approximate this numerically
  const Real dT = temperature.value() * 1.0e-8;
  const ADReal t2 = temperature + dT;
  const ADReal Kh2 = _water97_fp.henryConstant(t2, _ncg_henry);

  const Real dhdis_dT =
      (-_R * t2 * t2 * Kh2.derivatives()[_Tidx] / Kh2 / _Mncg - hdis).value() / dT;

  hdis.derivatives() = temperature.derivatives() * dhdis_dT;

  return hdis;
}

Real
PorousFlowWaterNCG::totalMassFraction(
    Real pressure, Real temperature, Real /* Xnacl */, Real saturation, unsigned int qp) const
{
  // Check whether the input temperature is within the region of validity
  checkVariables(temperature);

  // As we do not require derivatives, we can simply ignore their initialisation
  const ADReal p = pressure;
  const ADReal T = temperature;

  // FluidStateProperties data structure
  std::vector<FluidStateProperties> fsp(_num_phases, FluidStateProperties(_num_components));
  auto & liquid = fsp[_aqueous_phase_number];
  auto & gas = fsp[_gas_phase_number];

  // Calculate equilibrium mass fractions in the two-phase state
  ADReal Xncg, Yh2o;
  equilibriumMassFractions(p, T, Xncg, Yh2o);

  // Save the mass fractions in the FluidStateMassFractions object to calculate gas density
  const ADReal Yncg = 1.0 - Yh2o;
  liquid.mass_fraction[_aqueous_fluid_component] = 1.0 - Xncg;
  liquid.mass_fraction[_gas_fluid_component] = Xncg;
  gas.mass_fraction[_aqueous_fluid_component] = Yh2o;
  gas.mass_fraction[_gas_fluid_component] = Yncg;

  // Gas density
  const Real gas_density = gasDensity(p, T, fsp).value();

  // Liquid density
  const ADReal liquid_pressure = p - _pc.capillaryPressure(1.0 - saturation, qp);
  const Real liquid_density = liquidDensity(liquid_pressure, T).value();

  // The total mass fraction of ncg (Z) can now be calculated
  const Real Z = (saturation * gas_density * Yncg.value() +
                  (1.0 - saturation) * liquid_density * Xncg.value()) /
                 (saturation * gas_density + (1.0 - saturation) * liquid_density);

  return Z;
}
