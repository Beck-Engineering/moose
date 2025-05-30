//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"

/**
 * Initial condition that accesses _zero
 */
class ZeroIC : public InitialCondition
{
public:
  static InputParameters validParams();

  ZeroIC(const InputParameters & parameters);

  virtual Real value(const Point & p);
};
