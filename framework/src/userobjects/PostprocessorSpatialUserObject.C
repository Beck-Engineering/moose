//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PostprocessorSpatialUserObject.h"

registerMooseObject("MooseApp", PostprocessorSpatialUserObject);

InputParameters
PostprocessorSpatialUserObject::validParams()
{
  InputParameters params = SpatialUserObjectFunctor<GeneralUserObject>::validParams();
  params.addRequiredParam<PostprocessorName>("postprocessor", "The name of the postprocessor");
  params.addClassDescription("User object (spatial) that holds a postprocessor value.");
  return params;
}

PostprocessorSpatialUserObject::PostprocessorSpatialUserObject(const InputParameters & parameters)
  : SpatialUserObjectFunctor<GeneralUserObject>(parameters),
    _value(getPostprocessorValue("postprocessor"))
{
}

void
PostprocessorSpatialUserObject::initialize()
{
}

void
PostprocessorSpatialUserObject::execute()
{
}

void
PostprocessorSpatialUserObject::finalize()
{
}

Real
PostprocessorSpatialUserObject::spatialValue(const Point & /*p*/) const
{
  return _value;
}
