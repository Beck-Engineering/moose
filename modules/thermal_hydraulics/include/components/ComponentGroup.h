//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "THMObject.h"

/**
 * Group of components. Use only for parsing input files
 */
class ComponentGroup : public THMObject
{
public:
  ComponentGroup(const InputParameters & parameters);

public:
  static InputParameters validParams();
};
