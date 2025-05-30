//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

/**
 * Action for building empty mesh object for hexagonal geometry that is filled by mesh
 * generators
 */
class TriSubChannelBuildMeshAction : public Action
{
public:
  TriSubChannelBuildMeshAction(const InputParameters & params);

  virtual void act();

public:
  static InputParameters validParams();
};
