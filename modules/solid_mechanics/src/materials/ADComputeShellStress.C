//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeShellStress.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "ADComputeIsotropicElasticityTensorShell.h"

#include "libmesh/quadrature.h"
#include "libmesh/utility.h"
#include "libmesh/enum_quadrature_type.h"
#include "libmesh/fe_type.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"

registerMooseObject("SolidMechanicsApp", ADComputeShellStress);

InputParameters
ADComputeShellStress::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute in-plane stress using elasticity for shell");
  params.addRequiredParam<std::string>("through_thickness_order",
                                       "Quadrature order in out of plane direction");
  return params;
}

ADComputeShellStress::ADComputeShellStress(const InputParameters & parameters)
  : Material(parameters)
{
  // get number of quadrature points along thickness based on order
  std::unique_ptr<libMesh::QGauss> t_qrule = std::make_unique<libMesh::QGauss>(
      1, Utility::string_to_enum<Order>(getParam<std::string>("through_thickness_order")));
  _t_points = t_qrule->get_points();
  _elasticity_tensor.resize(_t_points.size());
  _stress.resize(_t_points.size());
  _stress_old.resize(_t_points.size());
  _strain_increment.resize(_t_points.size());
  _local_transformation_matrix.resize(_t_points.size());
  _covariant_transformation_matrix.resize(_t_points.size());
  _global_stress.resize(_t_points.size());
  _local_stress.resize(_t_points.size());
  for (unsigned int t = 0; t < _t_points.size(); ++t)
  {
    _elasticity_tensor[t] =
        &getADMaterialProperty<RankFourTensor>("elasticity_tensor_t_points_" + std::to_string(t));
    _stress[t] = &declareADProperty<RankTwoTensor>("stress_t_points_" + std::to_string(t));
    _stress_old[t] =
        &getMaterialPropertyOldByName<RankTwoTensor>("stress_t_points_" + std::to_string(t));
    _strain_increment[t] =
        &getADMaterialProperty<RankTwoTensor>("strain_increment_t_points_" + std::to_string(t));
    // rotation matrix and stress for output purposes only
    _local_transformation_matrix[t] =
        &getMaterialProperty<RankTwoTensor>("local_transformation_t_points_" + std::to_string(t));
    _covariant_transformation_matrix[t] = &getMaterialProperty<RankTwoTensor>(
        "covariant_transformation_t_points_" + std::to_string(t));
    _global_stress[t] =
        &declareProperty<RankTwoTensor>("global_stress_t_points_" + std::to_string(t));
    _local_stress[t] =
        &declareProperty<RankTwoTensor>("local_stress_t_points_" + std::to_string(t));
  }
}

void
ADComputeShellStress::initQpStatefulProperties()
{
  // initialize stress tensor to zero
  for (unsigned int i = 0; i < _t_points.size(); ++i)
    (*_stress[i])[_qp].zero();
}

void
ADComputeShellStress::computeQpProperties()
{
  for (unsigned int i = 0; i < _t_points.size(); ++i)
  {
    (*_stress[i])[_qp] =
        (*_stress_old[i])[_qp] + (*_elasticity_tensor[i])[_qp] * (*_strain_increment[i])[_qp];

    for (unsigned int ii = 0; ii < 3; ++ii)
      for (unsigned int jj = 0; jj < 3; ++jj)
        _unrotated_stress(ii, jj) = MetaPhysicL::raw_value((*_stress[i])[_qp](ii, jj));
    (*_global_stress[i])[_qp] = (*_covariant_transformation_matrix[i])[_qp].transpose() *
                                _unrotated_stress * (*_covariant_transformation_matrix[i])[_qp];
    (*_local_stress[i])[_qp] = (*_local_transformation_matrix[i])[_qp] * (*_global_stress[i])[_qp] *
                               (*_local_transformation_matrix[i])[_qp].transpose();
  }
}
