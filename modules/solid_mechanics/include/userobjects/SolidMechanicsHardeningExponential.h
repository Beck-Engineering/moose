//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SolidMechanicsHardeningModel.h"

/**
 * Exponential hardening
 * The value = _val_res + (val_0 - val_res)*exp(-rate*internal_parameter)
 * Note that while this is C-infinity, it produces unphysical results for
 * internal_parameter<0, which can cause numerical problems.
 */
class SolidMechanicsHardeningExponential : public SolidMechanicsHardeningModel
{
public:
  static InputParameters validParams();

  SolidMechanicsHardeningExponential(const InputParameters & parameters);

  virtual Real value(Real intnl) const override;

  virtual Real derivative(Real intnl) const override;

  virtual std::string modelName() const override;

private:
  /// The value = _val_res + (val_0 - val_res)*exp(-rate*internal_parameter)
  Real _val_0;

  /// The value = _val_res + (val_0 - val_res)*exp(-rate*internal_parameter)
  Real _val_res;

  /// The value = _val_res + (val_0 - val_res)*exp(-rate*internal_parameter)
  Real _rate;
};
