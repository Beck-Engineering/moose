//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"
#include "FaceCenteredMapFunctor.h"
#include "Coupleable.h"

// Forward Declarations
class MooseMesh;

class TestFaceToCellReconstruction : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  TestFaceToCellReconstruction(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override {}

  virtual PostprocessorValue getValue() const override final;

protected:
  PostprocessorValue _reconstruction_error;
  FaceCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>>
      _face_values;
};
