//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

/**
 * Computes the change in a post-processor value, or the magnitude of its
 * relative change, over a time step or over the entire transient.
 */
class ChangeOverTimePostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  ChangeOverTimePostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual Real getValue() const override;

protected:
  /// option to compute change with respect to initial value instead of previous time value
  const bool _change_with_respect_to_initial;

  /// option to compute the magnitude of relative change instead of change
  const bool _compute_relative_change;

  /// option to take the absolute value of the change
  const bool _take_absolute_value;

  /// current post-processor value
  const PostprocessorValue & _pps_value;

  /// old post-processor value
  const PostprocessorValue & _pps_value_old;

  /// initial post-processor value
  Real & _pps_value_initial;

  /// This post-processor value
  Real _value;
};
