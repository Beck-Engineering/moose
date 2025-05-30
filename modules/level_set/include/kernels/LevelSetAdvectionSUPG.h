//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelGrad.h"

/**
 * SUPG stabilization for the advection portion of the level set equation.
 */
class LevelSetAdvectionSUPG : public ADKernelGrad
{
public:
  static InputParameters validParams();

  LevelSetAdvectionSUPG(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpResidual() override;

  /// Velocity vector variable
  const ADVectorVariableValue & _velocity;
};
