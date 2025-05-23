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

class FunctionElementLoopIntegralUserObject;

/**
 * Gets the value from a FunctionElementLoopIntegralUserObject.
 */
class FunctionElementLoopIntegralGetValueTestPostprocessor : public GeneralPostprocessor
{
public:
  FunctionElementLoopIntegralGetValueTestPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual PostprocessorValue getValue() const override;

protected:
  /// User object to get value from
  const FunctionElementLoopIntegralUserObject & _uo;

public:
  static InputParameters validParams();
};
