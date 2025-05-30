//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddComponentAction.h"
#include "THMProblem.h"
#include "THMMesh.h"

registerMooseAction("ThermalHydraulicsApp", AddComponentAction, "THM:add_component");

InputParameters
AddComponentAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.makeParamNotRequired<std::string>("type");
  params.set<std::string>("type") = "ComponentGroup";
  return params;
}

AddComponentAction::AddComponentAction(const InputParameters & params)
  : MooseObjectAction(params), _group(_type == "ComponentGroup")
{
}

void
AddComponentAction::act()
{
  // Error if using an unsupported mesh type, as most components cannot handle working with a
  // regular MooseMesh instead of a THM mesh
  if (!dynamic_cast<THMMesh *>(_mesh.get()))
    mooseError("The Components block cannot be used to add a Component in conjunction with the "
               "Mesh block at this time");

  if (!_group)
  {
    THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
    if (thm_problem)
    {
      _moose_object_pars.set<THMProblem *>("_thm_problem") = thm_problem;

      std::vector<std::string> parts;
      MooseUtils::tokenize<std::string>(_moose_object_pars.blockFullpath(), parts);

      std::stringstream res;
      std::copy(++parts.begin(), parts.end(), std::ostream_iterator<std::string>(res, "/"));

      std::string comp_name = res.str();
      comp_name.pop_back();

      thm_problem->addComponent(_type, comp_name, _moose_object_pars);
    }
  }
}
