//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SlopeLimitingBase.h"
#include "SlopeReconstructionBase.h"

/**
 * Base class for multi-dimensional slope limiting to limit
 * the slopes of cell average variables
 */
class SlopeLimitingMultiDBase : public SlopeLimitingBase
{
public:
  static InputParameters validParams();

  SlopeLimitingMultiDBase(const InputParameters & parameters);

protected:
  /// slope reconstruction user object
  const SlopeReconstructionBase & _rslope;
};
