//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddPositionsAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddPositionsAction, "add_positions");

InputParameters
AddPositionsAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Positions object to the simulation.");
  return params;
}

AddPositionsAction::AddPositionsAction(const InputParameters & params) : MooseObjectAction(params)
{
}

void
AddPositionsAction::act()
{
  _problem->addReporter(_type, _name, _moose_object_pars);
}
