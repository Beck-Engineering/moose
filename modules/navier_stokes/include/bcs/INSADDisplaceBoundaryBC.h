//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADNodalBC.h"

/**
 * Increments the boundary displacement by the product of the surface velocity and the change in
 * time through an implicit Euler disretization
 */
class INSADDisplaceBoundaryBC : public ADNodalBC
{
public:
  static InputParameters validParams();

  INSADDisplaceBoundaryBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// The velocity
  const Moose::Functor<ADRealVectorValue> & _velocity;

  /// The previous timestep value of the displacement
  const Real & _u_old;

  /// What component of velocity/displacement this object is acting on
  const unsigned short _component;

  /// The subdomain ID along which the boundary nodeset, that this object is acting on, is
  /// associated with
  const SubdomainID _sub_id;
};
