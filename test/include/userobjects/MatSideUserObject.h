//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideUserObject.h"

/*
 * This is for testing error message only. It does nothing.
 */
class MatSideUserObject : public SideUserObject
{
public:
  static InputParameters validParams();

  MatSideUserObject(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}
  virtual void threadJoin(const UserObject &) override {}

protected:
  const MaterialProperty<Real> & _mat_prop;
};
