//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructureCylindricalBase.h"

InputParameters
HeatStructureCylindricalBase::validParams()
{
  InputParameters params = HeatStructureBase::validParams();
  params.addParam<bool>(
      "offset_mesh_by_inner_radius", false, "Offset the mesh by the inner radius?");
  return params;
}

HeatStructureCylindricalBase::HeatStructureCylindricalBase(const InputParameters & params)
  : HeatStructureBase(params)
{
}

void
HeatStructureCylindricalBase::setupMesh()
{
  _axial_offset = _inner_radius;

  HeatStructureBase::setupMesh();
}

Real
HeatStructureCylindricalBase::getUnitPerimeter(const ExternalBoundaryType & side) const
{
  switch (side)
  {
    case ExternalBoundaryType::OUTER:
      return 2 * M_PI * (_inner_radius + _total_width);

    case ExternalBoundaryType::INNER:
      return 2 * M_PI * _inner_radius;

    case ExternalBoundaryType::START:
    case ExternalBoundaryType::END:
      return std::numeric_limits<Real>::quiet_NaN();
  }

  mooseError(name(), ": Unknown value of 'side' parameter.");
}

Real
HeatStructureCylindricalBase::computeRadialBoundaryArea(const Real & length, const Real & y) const
{
  return length * 2 * libMesh::pi * (_inner_radius + y);
}

Real
HeatStructureCylindricalBase::computeAxialBoundaryArea(const Real & y_min, const Real & y_max) const
{
  return libMesh::pi * (std::pow(_inner_radius + y_max, 2) - std::pow(_inner_radius + y_min, 2));
}
