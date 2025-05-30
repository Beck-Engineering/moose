//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralPostprocessor.h"

// Forward Declarations

/**
 * This postprocessor computes homogenized elastic constants
 */
class AsymptoticExpansionHomogenizationElasticConstants : public ElementIntegralPostprocessor
{
public:
  static InputParameters validParams();

  AsymptoticExpansionHomogenizationElasticConstants(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() const override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  virtual Real computeQpIntegral() override;

private:
  /// Base name of the material system
  const std::string _base_name;

  const std::array<std::array<const VariableGradient *, 3>, 6> _grad;

  const MaterialProperty<RankFourTensor> & _elasticity_tensor;
  const unsigned int _column, _row;
  const std::array<unsigned int, 6> _ik_index;
  const std::array<unsigned int, 6> _jl_index;
  const unsigned _i, _j, _k, _l;
  Real _volume;
  Real _integral_value;
};
