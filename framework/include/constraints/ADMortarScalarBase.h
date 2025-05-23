//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMortarConstraint.h"

/**
 * This Constraint adds standardized methods for assembling to a primary
 * scalar variable associated with the major variables of the Mortar
 * Constraint object. Essentially, the entire row of the residual and Jacobian
 * associated with this scalar variable will also be assembled here
 * using the loops over mortar segments.
 * This variable is "scalar_variable" in the input file and "kappa"
 * within the source code.
 */
class ADMortarScalarBase : public ADMortarConstraint
{
public:
  static InputParameters validParams();

  ADMortarScalarBase(const InputParameters & parameters);

  // Using declarations necessary to pull in computeResidual with different parameter list and avoid
  // hidden method warning
  using ADMortarConstraint::computeResidual;

  // Using declarations necessary to pull in computeJacobian with different parameter list and avoid
  // hidden method warning
  using ADMortarConstraint::computeJacobian;

  /**
   * The scalar variable that this kernel operates on.
   */
  const MooseVariableScalar & scalarVariable() const
  {
    mooseAssert(_kappa_var_ptr, "kappa pointer should have been set in the constructor");
    return *_kappa_var_ptr;
  }

  /**
   * Computes _var-residuals as well as _kappa-residual
   */
  virtual void computeResidual() override;
  /**
   * Computes d-_var-residual / d-_var and d-_var-residual / d-jvar,
   * as well as d-_kappa-residual / d-_var and d-_kappa-residual / d-jvar
   */
  virtual void computeJacobian() override;

protected:
  /**
   * Method for computing the scalar part of residual at quadrature points
   */
  virtual ADReal computeScalarQpResidual();

  /**
   * Put necessary evaluations depending on qp but independent of test functions here
   */
  virtual void initScalarQpResidual() {}

  /// Whether a scalar variable is declared for this constraint
  const bool _use_scalar;

  /// Whether to compute scalar contributions for this instance
  const bool _compute_scalar_residuals;

  /// (Pointer to) Scalar variable this kernel operates on
  const MooseVariableScalar * const _kappa_var_ptr;

  /// The unknown scalar variable ID
  const unsigned int _kappa_var;

  /// Order of the scalar variable, used in several places
  const unsigned int _k_order;

  /// Reference to the current solution at the current quadrature point
  const ADVariableValue & _kappa;

  /// Used internally to iterate over each scalar component
  unsigned int _h;
};

inline ADReal
ADMortarScalarBase::computeScalarQpResidual()
{
  mooseError(
      "A scalar_variable has been set and compute_scalar_residuals=true, ",
      "but the computeScalarQpResidual method was not overridden. Accidental call of base class?");
  return 0;
}
