//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ScalarKernel.h"

class AlphaCED : public ScalarKernel
{
public:
  static InputParameters validParams();

  AlphaCED(const InputParameters & parameters);

  virtual void reinit() override;
  virtual void computeOffDiagJacobianScalar(unsigned int jvar) override;

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobianScalar(unsigned int jvar);

  Real _value;
};
