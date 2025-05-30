//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVFluxBC.h"
#include "INSFVFullyDevelopedFlowBC.h"

class INSFVVelocityVariable;

/**
 * A class for finite volume fully developed outflow boundary conditions for the momentum equation
 * It advects momentum at the outflow, and may replace outlet pressure boundary conditions
 * when selecting a mean-pressure approach.
 */
class INSFVMomentumAdvectionOutflowBC : public INSFVFluxBC, public INSFVFullyDevelopedFlowBC
{
public:
  static InputParameters validParams();
  INSFVMomentumAdvectionOutflowBC(const InputParameters & params);

  using INSFVFluxBC::gatherRCData;
  void gatherRCData(const FaceInfo &) override;

protected:
  /**
   * A virtual method that can be overridden in PINSFV classes to return a non-unity porosity
   */
  virtual const Moose::FunctorBase<ADReal> & epsFunctor() const { return _unity_functor; }

  ADReal computeSegregatedContribution() override;

  /// Computes the advected quantity which is then used on gatherRCData
  /// and computeSegregatedContribution
  /// @param boundary_face The boundary face argument
  /// @param state The state (time, nonolinar iterate) argument
  ADReal computeAdvectedQuantity(const Moose::FaceArg & boundary_face,
                                 const Moose::StateArg & state);

  /// x-velocity
  const Moose::Functor<ADReal> & _u;
  /// y-velocity
  const Moose::Functor<ADReal> * const _v;
  /// z-velocity
  const Moose::Functor<ADReal> * const _w;

  /// the dimension of the simulation
  const unsigned int _dim;

  /// The density
  const Moose::Functor<ADReal> & _rho;

  /// A unity functor used in the \p epsFunctor virtual method
  const Moose::ConstantFunctor<ADReal> _unity_functor{1};
};
