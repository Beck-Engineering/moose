//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AddVariableAction.h"

class AddElementalFieldAction : public AddVariableAction
{
public:
  static InputParameters validParams();

  AddElementalFieldAction(const InputParameters & params);

  virtual void act() override;
  void init() override;
};
