//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LegacyDynamicTensorMechanicsAction.h"

registerMooseAction("SolidMechanicsApp", LegacyDynamicTensorMechanicsAction, "setup_mesh_complete");

registerMooseAction("SolidMechanicsApp",
                    LegacyDynamicTensorMechanicsAction,
                    "validate_coordinate_systems");

registerMooseAction("SolidMechanicsApp", LegacyDynamicTensorMechanicsAction, "add_kernel");

InputParameters
LegacyDynamicTensorMechanicsAction::validParams()
{
  InputParameters params = DynamicSolidMechanicsPhysics::validParams();
  params.addParam<bool>(
      "use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  return params;
}

LegacyDynamicTensorMechanicsAction::LegacyDynamicTensorMechanicsAction(
    const InputParameters & params)
  : DynamicSolidMechanicsPhysics(params)
{
}

void
LegacyDynamicTensorMechanicsAction::act()
{
  if (_current_task == "add_kernel" || _current_task == "validate_coordinate_systems")
    // note that we do not call SolidMechanicsAction::act() here, because the old
    // behavior is not to add inertia kernels
    QuasiStaticSolidMechanicsPhysics::act();
}
