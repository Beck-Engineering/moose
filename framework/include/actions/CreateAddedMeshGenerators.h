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
 * Action that calls MeshGeneratorSystem::createAddedMeshGenerators()
 *
 * This constructs the MeshGenerators that are defined by
 * MeshGeneratorSystem::addMeshGenerator() in the correct
 * dependency order.
 */
class CreateAddedMeshGenerators : public Action
{
public:
  static InputParameters validParams();

  CreateAddedMeshGenerators(const InputParameters & params);

  virtual void act() override;
};
