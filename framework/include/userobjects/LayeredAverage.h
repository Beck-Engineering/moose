//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LayeredVolumeAverageBase.h"
#include "ElementIntegralVariableUserObject.h"

/**
 * This UserObject computes averages of a variable storing partial
 * sums for the specified number of intervals in a direction (x,y,z).
 */
class LayeredAverage : public LayeredVolumeAverageBase<ElementIntegralVariableUserObject>
{
public:
  static InputParameters validParams();

  LayeredAverage(const InputParameters & parameters);
};
