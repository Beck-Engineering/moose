//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneratedMesh.h"

#include "MooseApp.h"

#include "libmesh/mesh_generation.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/periodic_boundaries.h"
#include "libmesh/periodic_boundary_base.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/node.h"

// C++ includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)
#include <array>

registerMooseObject("MooseApp", GeneratedMesh);

InputParameters
GeneratedMesh::validParams()
{
  InputParameters params = MooseMesh::validParams();

  MooseEnum elem_types(LIST_GEOM_ELEM); // no default

  MooseEnum dims("1=1 2 3");
  params.addRequiredParam<MooseEnum>("dim", dims, "The dimension of the mesh to be generated");

  params.addParam<unsigned int>("nx", 1, "Number of elements in the X direction");
  params.addParam<unsigned int>("ny", 1, "Number of elements in the Y direction");
  params.addParam<unsigned int>("nz", 1, "Number of elements in the Z direction");
  params.addParam<Real>("xmin", 0.0, "Lower X Coordinate of the generated mesh");
  params.addParam<Real>("ymin", 0.0, "Lower Y Coordinate of the generated mesh");
  params.addParam<Real>("zmin", 0.0, "Lower Z Coordinate of the generated mesh");
  params.addParam<Real>("xmax", 1.0, "Upper X Coordinate of the generated mesh");
  params.addParam<Real>("ymax", 1.0, "Upper Y Coordinate of the generated mesh");
  params.addParam<Real>("zmax", 1.0, "Upper Z Coordinate of the generated mesh");
  params.addParam<MooseEnum>("elem_type",
                             elem_types,
                             "The type of element from libMesh to "
                             "generate (default: linear element for "
                             "requested dimension)");
  params.addParam<bool>(
      "gauss_lobatto_grid",
      false,
      "Grade mesh into boundaries according to Gauss-Lobatto quadrature spacing.");
  params.addRangeCheckedParam<Real>(
      "bias_x",
      1.,
      "bias_x>=0.5 & bias_x<=2",
      "The amount by which to grow (or shrink) the cells in the x-direction.");
  params.addRangeCheckedParam<Real>(
      "bias_y",
      1.,
      "bias_y>=0.5 & bias_y<=2",
      "The amount by which to grow (or shrink) the cells in the y-direction.");
  params.addRangeCheckedParam<Real>(
      "bias_z",
      1.,
      "bias_z>=0.5 & bias_z<=2",
      "The amount by which to grow (or shrink) the cells in the z-direction.");

  params.addParamNamesToGroup("dim", "Required");

  params.addClassDescription(
      "Create a line, square, or cube mesh with uniformly spaced or biased elements.");
  return params;
}

GeneratedMesh::GeneratedMesh(const InputParameters & parameters)
  : MooseMesh(parameters),
    _dim(getParam<MooseEnum>("dim")),
    _nx(getParam<unsigned int>("nx")),
    _ny(getParam<unsigned int>("ny")),
    _nz(getParam<unsigned int>("nz")),
    _xmin(getParam<Real>("xmin")),
    _xmax(getParam<Real>("xmax")),
    _ymin(getParam<Real>("ymin")),
    _ymax(getParam<Real>("ymax")),
    _zmin(getParam<Real>("zmin")),
    _zmax(getParam<Real>("zmax")),
    _gauss_lobatto_grid(getParam<bool>("gauss_lobatto_grid")),
    _bias_x(getParam<Real>("bias_x")),
    _bias_y(getParam<Real>("bias_y")),
    _bias_z(getParam<Real>("bias_z")),
    _dims_may_have_changed(false)
{
  if (_gauss_lobatto_grid && (_bias_x != 1.0 || _bias_y != 1.0 || _bias_z != 1.0))
    mooseError("Cannot apply both Gauss-Lobatto mesh grading and biasing at the same time.");

  // All generated meshes are regular orthogonal meshes - until they get modified ;)
  _regular_orthogonal_mesh = true;
}

void
GeneratedMesh::prepared(bool state)
{
  MooseMesh::prepared(state);

  // Fall back on scanning the mesh for coordinates instead of using input parameters for queries
  if (!state)
    _dims_may_have_changed = true;
}

Real
GeneratedMesh::getMinInDimension(unsigned int component) const
{
  if (_dims_may_have_changed)
    return MooseMesh::getMinInDimension(component);

  switch (component)
  {
    case 0:
      return _xmin;
    case 1:
      return _dim > 1 ? _ymin : 0;
    case 2:
      return _dim > 2 ? _zmin : 0;
    default:
      mooseError("Invalid component");
  }
}

Real
GeneratedMesh::getMaxInDimension(unsigned int component) const
{
  if (_dims_may_have_changed)
    return MooseMesh::getMaxInDimension(component);

  switch (component)
  {
    case 0:
      return _xmax;
    case 1:
      return _dim > 1 ? _ymax : 0;
    case 2:
      return _dim > 2 ? _zmax : 0;
    default:
      mooseError("Invalid component");
  }
}

std::unique_ptr<MooseMesh>
GeneratedMesh::safeClone() const
{
  return _app.getFactory().copyConstruct(*this);
}

void
GeneratedMesh::buildMesh()
{
  MooseEnum elem_type_enum = getParam<MooseEnum>("elem_type");

  if (!isParamValid("elem_type"))
  {
    // Switching on MooseEnum
    switch (_dim)
    {
      case 1:
        elem_type_enum = "EDGE2";
        break;
      case 2:
        elem_type_enum = "QUAD4";
        break;
      case 3:
        elem_type_enum = "HEX8";
        break;
    }
  }

  ElemType elem_type = Utility::string_to_enum<ElemType>(elem_type_enum);

  // Switching on MooseEnum
  switch (_dim)
  {
    // The build_XYZ mesh generation functions take an
    // UnstructuredMesh& as the first argument, hence the dynamic_cast.
    case 1:
      MeshTools::Generation::build_line(dynamic_cast<UnstructuredMesh &>(getMesh()),
                                        _nx,
                                        _xmin,
                                        _xmax,
                                        elem_type,
                                        _gauss_lobatto_grid);
      break;
    case 2:
      MeshTools::Generation::build_square(dynamic_cast<UnstructuredMesh &>(getMesh()),
                                          _nx,
                                          _ny,
                                          _xmin,
                                          _xmax,
                                          _ymin,
                                          _ymax,
                                          elem_type,
                                          _gauss_lobatto_grid);
      break;
    case 3:
      MeshTools::Generation::build_cube(dynamic_cast<UnstructuredMesh &>(getMesh()),
                                        _nx,
                                        _ny,
                                        _nz,
                                        _xmin,
                                        _xmax,
                                        _ymin,
                                        _ymax,
                                        _zmin,
                                        _zmax,
                                        elem_type,
                                        _gauss_lobatto_grid);
      break;
  }

  // Apply the bias if any exists
  if (_bias_x != 1.0 || _bias_y != 1.0 || _bias_z != 1.0)
  {
    // Reference to the libmesh mesh
    MeshBase & mesh = getMesh();

    // Biases
    std::array<Real, LIBMESH_DIM> bias = {
        {_bias_x, _dim > 1 ? _bias_y : 1.0, _dim > 2 ? _bias_z : 1.0}};

    // "width" of the mesh in each direction
    std::array<Real, LIBMESH_DIM> width = {
        {_xmax - _xmin, _dim > 1 ? _ymax - _ymin : 0, _dim > 2 ? _zmax - _zmin : 0}};

    // Min mesh extent in each direction.
    std::array<Real, LIBMESH_DIM> mins = {
        {getMinInDimension(0), getMinInDimension(1), getMinInDimension(2)}};

    // Number of elements in each direction.
    std::array<unsigned int, LIBMESH_DIM> nelem = {{_nx, _dim > 1 ? _ny : 1, _dim > 2 ? _nz : 1}};

    // We will need the biases raised to integer powers in each
    // direction, so let's pre-compute those...
    std::array<std::vector<Real>, LIBMESH_DIM> pows;
    for (unsigned int dir = 0; dir < LIBMESH_DIM; ++dir)
    {
      pows[dir].resize(nelem[dir] + 1);
      for (unsigned int i = 0; i < pows[dir].size(); ++i)
        pows[dir][i] = std::pow(bias[dir], static_cast<int>(i));
    }

    // Loop over the nodes and move them to the desired location
    for (auto & node_ptr : mesh.node_ptr_range())
    {
      Node & node = *node_ptr;

      for (unsigned int dir = 0; dir < LIBMESH_DIM; ++dir)
      {
        if (width[dir] != 0. && bias[dir] != 1.)
        {
          // Compute the scaled "index" of the current point.  This
          // will either be close to a whole integer or a whole
          // integer+0.5 for quadratic nodes.
          Real float_index = (node(dir) - mins[dir]) * nelem[dir] / width[dir];

          Real integer_part = 0;
          Real fractional_part = std::modf(float_index, &integer_part);

          // Figure out where to move the node...
          if (std::abs(fractional_part) < TOLERANCE || std::abs(fractional_part - 1.0) < TOLERANCE)
          {
            // If the fractional_part ~ 0.0 or 1.0, this is a vertex node, so
            // we don't need an average.
            //
            // Compute the "index" we are at in the current direction.  We
            // round to the nearest integral value to get this instead
            // of using "integer_part", since that could be off by a
            // lot (e.g. we want 3.9999 to map to 4.0 instead of 3.0).
            int index = round(float_index);

            mooseAssert(index >= static_cast<int>(0) && index < static_cast<int>(pows[dir].size()),
                        "Scaled \"index\" out of range");

            // Move node to biased location.
            node(dir) =
                mins[dir] + width[dir] * (1. - pows[dir][index]) / (1. - pows[dir][nelem[dir]]);
          }
          else if (std::abs(fractional_part - 0.5) < TOLERANCE)
          {
            // If the fractional_part ~ 0.5, this is a midedge/face
            // (i.e. quadratic) node.  We don't move those with the same
            // bias as the vertices, instead we put them midway between
            // their respective vertices.
            //
            // Also, since the fractional part is nearly 0.5, we know that
            // the integer_part will be the index of the vertex to the
            // left, and integer_part+1 will be the index of the
            // vertex to the right.
            node(dir) = mins[dir] +
                        width[dir] *
                            (1. - 0.5 * (pows[dir][integer_part] + pows[dir][integer_part + 1])) /
                            (1. - pows[dir][nelem[dir]]);
          }
          else
          {
            // We don't yet handle anything higher order than quadratic...
            mooseError("Unable to bias node at node(", dir, ")=", node(dir));
          }
        }
      }
    }
  }
}
