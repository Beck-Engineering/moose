//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddFVICAction.h"
#include "FEProblem.h"
#include "MooseTypes.h"
#include "MooseUtils.h"

registerMooseAction("MooseApp", AddFVICAction, "add_fv_ic");

InputParameters
AddFVICAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();

  // This is to help with input file validation.  When ICs are created with
  // this action, they are nested underneath a variable in the input file - so
  // we implicitly already know the variable name from this nesting and users
  // don't need to specify it for us with the parameter.  So we say here that
  // the variable param is provided by the action.
  params.set<std::vector<std::string>>("_object_params_set_by_action") = {"variable"};

  return params;
}

AddFVICAction::AddFVICAction(const InputParameters & params) : MooseObjectAction(params) {}

void
AddFVICAction::act()
{
  std::vector<std::string> elements;
  MooseUtils::tokenize<std::string>(_pars.blockFullpath(), elements);

  // The variable name will be the second to last element in the path name
  std::string & var_name = elements[elements.size() - 2];
  _moose_object_pars.set<VariableName>("variable") = var_name;
  const auto & var = _problem->getVariable(0, var_name);

  if (!var.isFV())
    mooseError(
        "Finite element variables do not support an [FVInitialCondition] subblock, try using a "
        "separate [InitialConditions] block in your input file.");
  else
    _problem->addFVInitialCondition(_type, var_name, _moose_object_pars);
}
