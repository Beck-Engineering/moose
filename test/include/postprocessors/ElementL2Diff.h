//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralVariablePostprocessor.h"

class ElementL2Diff : public ElementIntegralVariablePostprocessor
{
public:
  static InputParameters validParams();

  ElementL2Diff(const InputParameters & parameters);

protected:
  /**
   * Get the L2 Error.
   */
  virtual Real getValue() const override;

  virtual Real computeQpIntegral() override;

  const VariableValue & _u_old;
};
