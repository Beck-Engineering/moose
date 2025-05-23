//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MoosePreconditioner.h"

// Forward declarations
class NonlinearSystemBase;
/**
 * Implements a preconditioner designed to map onto PETSc's PCFieldSplit.
 */
class FieldSplitPreconditioner : public MoosePreconditioner
{
public:
  /**
   *  Constructor. Initializes SplitBasedPreconditioner data structures
   */
  static InputParameters validParams();

  FieldSplitPreconditioner(const InputParameters & parameters);

  /**
   * top split
   */
  std::vector<std::string> _top_split;

protected:
  /// The nonlinear system this FSP is associated with (convenience reference)
  NonlinearSystemBase & _nl;
};
