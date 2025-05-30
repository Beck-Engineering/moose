//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementPointNeighborLayers.h"
#include "MooseMesh.h"
#include "Conversion.h"
#include "MooseApp.h"

#include "libmesh/point_neighbor_coupling.h"

using namespace libMesh;

registerMooseObject("MooseApp", ElementPointNeighborLayers);

InputParameters
ElementPointNeighborLayers::validParams()
{
  InputParameters params = FunctorRelationshipManager::validParams();

  params.addRangeCheckedParam<unsigned short>(
      "layers",
      1,
      "element_side_neighbor_layers>=1 & element_side_neighbor_layers<=10",
      "The number of additional geometric elements to make available when "
      "using distributed mesh. No effect with replicated mesh.");

  return params;
}

ElementPointNeighborLayers::ElementPointNeighborLayers(const InputParameters & parameters)
  : FunctorRelationshipManager(parameters), _layers(getParam<unsigned short>("layers"))
{
}

ElementPointNeighborLayers::ElementPointNeighborLayers(const ElementPointNeighborLayers & others)
  : FunctorRelationshipManager(others), _layers(others._layers)
{
}

std::unique_ptr<GhostingFunctor>
ElementPointNeighborLayers::clone() const
{
  return _app.getFactory().copyConstruct(*this);
}

std::string
ElementPointNeighborLayers::getInfo() const
{
  std::ostringstream oss;
  std::string layers = _layers == 1 ? "layer" : "layers";

  oss << "ElementPointNeighborLayers (" << _layers << " " << layers << ')';

  return oss.str();
}

bool
ElementPointNeighborLayers::operator>=(const RelationshipManager & rhs) const
{
  const auto * rm = dynamic_cast<const ElementPointNeighborLayers *>(&rhs);
  if (!rm)
    return false;
  else
    return _layers >= rm->_layers && baseGreaterEqual(*rm);
}

void
ElementPointNeighborLayers::internalInitWithMesh(const MeshBase &)
{
  auto functor = std::make_unique<PointNeighborCoupling>();
  functor->set_n_levels(_layers);

  _functor = std::move(functor);
}
