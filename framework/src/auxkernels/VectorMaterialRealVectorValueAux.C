//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorMaterialRealVectorValueAux.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", VectorMaterialRealVectorValueAux);
registerMooseObject("MooseApp", ADVectorMaterialRealVectorValueAux);

template <bool is_ad>
InputParameters
VectorMaterialRealVectorValueAuxTempl<is_ad>::validParams()
{
  auto params = MaterialAuxBaseTempl<RealVectorValue, is_ad, false, RealVectorValue>::validParams();

  params.addClassDescription(
      "Converts a vector-quantity material property into a vector auxiliary variable");

  return params;
}

template <bool is_ad>
VectorMaterialRealVectorValueAuxTempl<is_ad>::VectorMaterialRealVectorValueAuxTempl(
    const InputParameters & parameters)
  : MaterialAuxBaseTempl<RealVectorValue, is_ad, false, RealVectorValue>(parameters)
{
}

template <bool is_ad>
RealVectorValue
VectorMaterialRealVectorValueAuxTempl<is_ad>::getRealValue()
{
  return MetaPhysicL::raw_value(this->_full_value);
}

template class VectorMaterialRealVectorValueAuxTempl<false>;
template class VectorMaterialRealVectorValueAuxTempl<true>;
