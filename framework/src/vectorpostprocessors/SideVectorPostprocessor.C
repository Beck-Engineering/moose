//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideVectorPostprocessor.h"

InputParameters
SideVectorPostprocessor::validParams()
{
  InputParameters params = SideUserObject::validParams();
  params += VectorPostprocessor::validParams();
  return params;
}

SideVectorPostprocessor::SideVectorPostprocessor(const InputParameters & parameters)
  : SideUserObject(parameters), VectorPostprocessor(this)
{
}
