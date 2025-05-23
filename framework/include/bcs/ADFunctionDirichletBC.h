//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADDirichletBCBaseTempl.h"

/**
 * Boundary condition of a Dirichlet type
 *
 * Sets the values of a nodal variable at nodes to values specified by a function
 */
class ADFunctionDirichletBC : public ADDirichletBCBaseTempl<Real>
{
public:
  static InputParameters validParams();

  ADFunctionDirichletBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpValue() override;

  /// The function describing the Dirichlet condition
  const Function & _function;
};
