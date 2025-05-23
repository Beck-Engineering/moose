//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Executor.h"

class BinaryTestExecutor : public Executor
{
public:
  BinaryTestExecutor(const InputParameters & parameters);

  virtual ~BinaryTestExecutor() {}

  static InputParameters validParams();

protected:
  virtual Result run() override;

  Executor & _inner1;
  Executor & _inner2;
};
