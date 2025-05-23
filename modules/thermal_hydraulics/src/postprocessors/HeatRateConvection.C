//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatRateConvection.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", HeatRateConvection);

InputParameters
HeatRateConvection::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();

  params.addRequiredCoupledVar("T", "Temperature");
  params.addRequiredParam<FunctionName>("T_ambient", "Ambient temperature function");
  params.addRequiredParam<FunctionName>("htc", "Ambient heat transfer coefficient function");
  params.addParam<MooseFunctorName>("scale", 1.0, "Functor by which to scale the heat flux");

  params.addClassDescription("Integrates a convective heat flux over a boundary.");

  return params;
}

HeatRateConvection::HeatRateConvection(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),

    _T(coupledValue("T")),
    _T_ambient_fn(getFunction("T_ambient")),
    _htc_ambient_fn(getFunction("htc")),
    _scale(getFunctor<Real>("scale"))
{
}

Real
HeatRateConvection::computeQpIntegral()
{
  const Moose::ElemSideQpArg space_arg = {_current_elem, _current_side, _qp, _qrule, _q_point[_qp]};
  return _scale(space_arg, Moose::currentState()) * _htc_ambient_fn.value(_t, _q_point[_qp]) *
         (_T_ambient_fn.value(_t, _q_point[_qp]) - _T[_qp]);
}
