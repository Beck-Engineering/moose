//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMPlanarMean.h"
#include "SolutionHandle.h"
#include "FEProblemBase.h"
#include "Function.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "libmesh/system.h"
#include "SCM.h"

registerMooseObject("SubChannelApp", SCMPlanarMean);
registerMooseObjectRenamed("SubChannelApp", PlanarMean, "06/30/2025 24:00", SCMPlanarMean);

InputParameters
SCMPlanarMean::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Calculates an mass-flow-rate averaged mean of the chosen "
                             "variable on a z-plane at a user defined height over all subchannels");
  params.addRequiredParam<AuxVariableName>("variable", "Variable you want the mean of");
  params.addRequiredParam<Real>("height", "Axial location of plane [m]");
  return params;
}

SCMPlanarMean::SCMPlanarMean(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(SCM::getConstMesh<SubChannelMesh>(_fe_problem.mesh())),
    _variable(getParam<AuxVariableName>("variable")),
    _height(getParam<Real>("height")),
    _mean_value(0)
{
}

void
SCMPlanarMean::execute()
{
  auto nz = _mesh.getNumOfAxialCells();
  auto n_channels = _mesh.getNumOfChannels();
  auto Soln = SolutionHandle(_fe_problem.getVariable(0, _variable));
  auto mdot_soln = SolutionHandle(_fe_problem.getVariable(0, "mdot"));
  auto z_grid = _mesh.getZGrid();
  auto total_length =
      _mesh.getHeatedLength() + _mesh.getHeatedLengthEntry() + _mesh.getHeatedLengthExit();

  auto mass_flow = 0.0;
  auto sum_sol_mass_flow = 0.0;

  // Use outlet mass flow rate for weighting. Print value at the exit of the assembly.
  if (_height >= total_length)
  {
    for (unsigned int i_ch = 0; i_ch < n_channels; i_ch++)
    {
      auto * node_out = _mesh.getChannelNode(i_ch, nz);
      mass_flow += mdot_soln(node_out);
      sum_sol_mass_flow += Soln(node_out) * mdot_soln(node_out);
    }
    _mean_value = sum_sol_mass_flow / mass_flow;
  }
  else
  {
    // Locally average over each axial location in each channel
    for (unsigned int iz = 0; iz < nz; iz++)
    {
      if (_height >= z_grid[iz] && _height < z_grid[iz + 1])
      {
        for (unsigned int i_ch = 0; i_ch < n_channels; i_ch++)
        {
          auto * node_out = _mesh.getChannelNode(i_ch, iz + 1);
          auto * node_in = _mesh.getChannelNode(i_ch, iz);
          auto average_solution = Soln(node_in) + (Soln(node_out) - Soln(node_in)) *
                                                      (_height - z_grid[iz]) /
                                                      (z_grid[iz + 1] - z_grid[iz]);
          auto average_mass_flow = mdot_soln(node_in) + (mdot_soln(node_out) - mdot_soln(node_in)) *
                                                            (_height - z_grid[iz]) /
                                                            (z_grid[iz + 1] - z_grid[iz]);
          mass_flow += average_mass_flow;
          sum_sol_mass_flow += average_solution * average_mass_flow;
        }
        _mean_value = sum_sol_mass_flow / mass_flow;
        break;
      }
    }
  }
}

Real
SCMPlanarMean::getValue() const
{
  return _mean_value;
}
