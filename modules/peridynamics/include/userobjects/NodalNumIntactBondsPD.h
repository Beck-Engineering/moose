//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalAuxVariableUserObjectBasePD.h"

/**
 * UserObject class to compute the number of intact bonds for each material point in PD fracture
 * modeling and simulation
 */
class NodalNumIntactBondsPD : public NodalAuxVariableUserObjectBasePD
{
public:
  static InputParameters validParams();

  NodalNumIntactBondsPD(const InputParameters & parameters);

  virtual void computeValue(unsigned int id, dof_id_type dof) override;
};
