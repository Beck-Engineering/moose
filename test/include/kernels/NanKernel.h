//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

/**
 * Kernel that generates NaN
 */
class NanKernel : public Kernel
{
public:
  static InputParameters validParams();

  NanKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:
  unsigned int _timestep_to_nan;

  unsigned int _deprecated_default;
  unsigned int _deprecated_no_default;
};
