//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CombinerGenerator.h"

#include "CastUniquePointer.h"
#include "MooseUtils.h"
#include "DelimitedFileReader.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/point.h"
#include "libmesh/node.h"

registerMooseObject("MooseApp", CombinerGenerator);

InputParameters
CombinerGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription(
      "Combine multiple meshes (or copies of one mesh) together into one (disjoint) mesh.  Can "
      "optionally translate those meshes before combining them.");

  params.addRequiredParam<std::vector<MeshGeneratorName>>(
      "inputs",
      "The input MeshGenerators.  This can either be N generators or 1 generator.  If only 1 is "
      "given then 'positions' must also be given.");

  params.addParam<std::vector<Point>>(
      "positions",
      "The (optional) position of each given mesh.  If N 'inputs' were given then this must either "
      "be left blank or N positions must be given.  If 1 input was given then this MUST be "
      "provided.");

  params.addParam<std::vector<FileName>>(
      "positions_file", "Alternative way to provide the position of each given mesh.");

  params.addParam<bool>("avoid_merging_subdomains",
                        false,
                        "Whether to prevent merging subdomains by offsetting ids. The first mesh "
                        "in the input will keep the same subdomains ids, the others will have "
                        "offsets. All subdomain names will remain valid");
  params.addParam<bool>("avoid_merging_boundaries",
                        false,
                        "Whether to prevent merging sidesets by offsetting ids. The first mesh "
                        "in the input will keep the same boundary ids, the others will have "
                        "offsets. All boundary names will remain valid");

  return params;
}

CombinerGenerator::CombinerGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _meshes(getMeshes("inputs")),
    _input_names(getParam<std::vector<MeshGeneratorName>>("inputs")),
    _avoid_merging_subdomains(getParam<bool>("avoid_merging_subdomains")),
    _avoid_merging_boundaries(getParam<bool>("avoid_merging_boundaries"))
{
  if (_input_names.empty())
    paramError("input_names", "You need to specify at least one MeshGenerator as an input.");

  if (isParamValid("positions") && isParamValid("positions_file"))
    mooseError("Both 'positions' and 'positions_file' cannot be specified simultaneously in "
               "CombinerGenerator ",
               _name);

  if (_input_names.size() == 1)
    if (!isParamValid("positions") && !isParamValid("positions_file"))
      paramError("positions",
                 "If only one input mesh is given, then 'positions' or 'positions_file' must also "
                 "be supplied");
}

void
CombinerGenerator::fillPositions()
{
  if (isParamValid("positions"))
  {
    _positions = getParam<std::vector<Point>>("positions");

    // the check in the constructor wont catch error where the user sets positions = ''
    if ((_input_names.size() == 1) && _positions.empty())
      paramError("positions", "If only one input mesh is given, then 'positions' cannot be empty.");

    if (_input_names.size() != 1)
      if (_positions.size() && (_input_names.size() != _positions.size()))
        paramError(
            "positions",
            "If more than one input mesh is provided then the number of positions provided must "
            "exactly match the number of input meshes.");
  }
  else if (isParamValid("positions_file"))
  {
    std::vector<FileName> positions_file = getParam<std::vector<FileName>>("positions_file");

    // the check in the constructor wont catch error where the user sets positions_file = ''
    if ((_input_names.size() == 1) && positions_file.empty())
      paramError("positions_file",
                 "If only one input mesh is given, then 'positions_file' cannot be empty.");

    for (const auto & f : positions_file)
    {
      MooseUtils::DelimitedFileReader file(f, &_communicator);
      file.setFormatFlag(MooseUtils::DelimitedFileReader::FormatFlag::ROWS);
      file.read();

      const std::vector<Point> & data = file.getDataAsPoints();

      if (_input_names.size() != 1)
        if (data.size() && (_input_names.size() != data.size()))
          paramError("positions_file",
                     "If more than one input mesh is provided then the number of positions must "
                     "exactly match the number of input meshes.");

      for (const auto & d : data)
        _positions.push_back(d);
    }
  }
}

std::unique_ptr<MeshBase>
CombinerGenerator::generate()
{
  // Two cases:
  // 1. Multiple input meshes and optional positions
  // 2. One input mesh and multiple positions
  fillPositions();

  // Case 1
  if (_meshes.size() != 1)
  {
    // merge all meshes into the first one
    auto mesh = dynamic_pointer_cast<UnstructuredMesh>(*_meshes[0]);

    if (!mesh)
      paramError("inputs", _input_names[0], " is not a valid unstructured mesh");

    // Move the first input mesh if applicable
    if (_positions.size())
    {
      MeshTools::Modification::translate(
          *mesh, _positions[0](0), _positions[0](1), _positions[0](2));
    }

    // Read in all of the other meshes
    for (MooseIndex(_meshes) i = 1; i < _meshes.size(); ++i)
    {
      auto other_mesh = dynamic_pointer_cast<UnstructuredMesh>(*_meshes[i]);

      if (!other_mesh)
        paramError("inputs", _input_names[i], " is not a valid unstructured mesh");

      // Move It
      if (_positions.size())
      {
        MeshTools::Modification::translate(
            *other_mesh, _positions[i](0), _positions[i](1), _positions[i](2));
      }

      copyIntoMesh(*mesh, *other_mesh);
    }

    mesh->set_isnt_prepared();
    return dynamic_pointer_cast<MeshBase>(mesh);
  }
  else // Case 2
  {
    auto input_mesh = dynamic_pointer_cast<UnstructuredMesh>(*_meshes[0]);

    if (!input_mesh)
      paramError("inputs", _input_names[0], " is not a valid unstructured mesh");

    // Make a copy and displace it in order to get the final mesh started
    auto copy =
        input_mesh->clone(); // This is required because dynamic_pointer_cast() requires an l-value
    auto final_mesh = dynamic_pointer_cast<UnstructuredMesh>(copy);

    if (!final_mesh)
      mooseError("Unable to copy mesh!");

    MeshTools::Modification::translate(
        *final_mesh, _positions[0](0), _positions[0](1), _positions[0](2));

    // Here's the way this is going to work:
    // I'm going to make one more copy of the input_mesh so that I can move it and copy it in
    // Then, after it's copied in I'm going to reset its coordinates by looping over the input_mesh
    // and resetting the nodal positions.
    // This could be done without the copy - you would translate the mesh then translate it back...
    // However, I'm worried about floating point roundoff.  If you were doing this 100,000 times or
    // more then the mesh could "drift" away from its original position.  I really want the
    // translations to be exact each time.
    // I suppose that it is technically possible to just save off a datastructure (map, etc.) that
    // could hold the nodal positions only (instead of a copy of the mesh) but I'm not sure that
    // would really save much... we'll see if it shows up in profiling somewhere
    copy = input_mesh->clone();
    auto translated_mesh = dynamic_pointer_cast<UnstructuredMesh>(copy);

    if (!translated_mesh)
      mooseError("Unable to copy mesh!");

    for (MooseIndex(_meshes) i = 1; i < _positions.size(); ++i)
    {
      // Move
      MeshTools::Modification::translate(
          *translated_mesh, _positions[i](0), _positions[i](1), _positions[i](2));

      // Copy into final mesh
      copyIntoMesh(*final_mesh, *translated_mesh);

      // Reset nodal coordinates
      for (auto translated_node_ptr : translated_mesh->node_ptr_range())
      {
        auto & translated_node = *translated_node_ptr;
        auto & input_node = input_mesh->node_ref(translated_node_ptr->id());

        for (MooseIndex(LIBMESH_DIM) i = 0; i < LIBMESH_DIM; i++)
          translated_node(i) = input_node(i);
      }
    }

    final_mesh->set_isnt_prepared();
    return dynamic_pointer_cast<MeshBase>(final_mesh);
  }
}

void
CombinerGenerator::copyIntoMesh(UnstructuredMesh & destination, const UnstructuredMesh & source)
{
  dof_id_type node_delta = destination.max_node_id();
  dof_id_type elem_delta = destination.max_elem_id();

  unique_id_type unique_delta =
#ifdef LIBMESH_ENABLE_UNIQUE_ID
      destination.parallel_max_unique_id();
#else
      0;
#endif

  // Prevent overlaps by offsetting the subdomains in
  std::unordered_map<subdomain_id_type, subdomain_id_type> id_remapping;
  unsigned int block_offset = 0;
  if (_avoid_merging_subdomains)
  {
    // Note: if performance becomes an issue, this is overkill for just getting the max node id
    std::set<subdomain_id_type> source_ids;
    std::set<subdomain_id_type> dest_ids;
    source.subdomain_ids(source_ids, true);
    destination.subdomain_ids(dest_ids, true);
    mooseAssert(source_ids.size(), "Should have a subdomain");
    mooseAssert(dest_ids.size(), "Should have a subdomain");
    unsigned int max_dest_bid = *dest_ids.rbegin();
    unsigned int min_source_bid = *source_ids.begin();
    _communicator.max(max_dest_bid);
    _communicator.min(min_source_bid);
    block_offset = 1 + max_dest_bid - min_source_bid;
    for (const auto bid : source_ids)
      id_remapping[bid] = block_offset + bid;
  }

  // Copy mesh data over from the other mesh
  destination.copy_nodes_and_elements(source,
                                      // Skipping this should cause the neighbors
                                      // to simply be copied from the other mesh
                                      // (which makes sense and is way faster)
                                      /*skip_find_neighbors = */ true,
                                      elem_delta,
                                      node_delta,
                                      unique_delta,
                                      _avoid_merging_subdomains ? &id_remapping : nullptr);

  // Get an offset to prevent overlaps / wild merging between boundaries
  BoundaryInfo & boundary = destination.get_boundary_info();
  const BoundaryInfo & other_boundary = source.get_boundary_info();

  unsigned int bid_offset = 0;
  if (_avoid_merging_boundaries)
  {
    const auto boundary_ids = boundary.get_boundary_ids();
    const auto other_boundary_ids = other_boundary.get_boundary_ids();
    unsigned int max_dest_bid = boundary_ids.size() ? *boundary_ids.rbegin() : 0;
    unsigned int min_source_bid = other_boundary_ids.size() ? *other_boundary_ids.begin() : 0;
    _communicator.max(max_dest_bid);
    _communicator.min(min_source_bid);
    bid_offset = 1 + max_dest_bid - min_source_bid;
  }

  // Note: the code below originally came from ReplicatedMesh::stitch_mesh_helper()
  // in libMesh replicated_mesh.C around line 1203

  // Copy BoundaryInfo from other_mesh too.  We do this via the
  // list APIs rather than element-by-element for speed.
  for (const auto & t : other_boundary.build_node_list())
    boundary.add_node(std::get<0>(t) + node_delta, bid_offset + std::get<1>(t));

  for (const auto & t : other_boundary.build_side_list())
    boundary.add_side(std::get<0>(t) + elem_delta, std::get<1>(t), bid_offset + std::get<2>(t));

  for (const auto & t : other_boundary.build_edge_list())
    boundary.add_edge(std::get<0>(t) + elem_delta, std::get<1>(t), bid_offset + std::get<2>(t));

  for (const auto & t : other_boundary.build_shellface_list())
    boundary.add_shellface(
        std::get<0>(t) + elem_delta, std::get<1>(t), bid_offset + std::get<2>(t));

  // Check for the case with two block ids sharing the same name
  if (_avoid_merging_subdomains)
  {
    for (const auto & [block_id, block_name] : destination.get_subdomain_name_map())
      for (const auto & [source_id, source_name] : source.get_subdomain_name_map())
        if (block_name == source_name)
          paramWarning("avoid_merging_subdomains",
                       "Not merging subdomains is creating two subdomains with the same name '" +
                           block_name + "' but different ids: " + std::to_string(source_id) +
                           " & " + std::to_string(block_id + block_offset) +
                           ".\n We recommend using a RenameBlockGenerator to prevent this as you "
                           "will get errors reading the Exodus output later.");
  }

  for (const auto & [block_id, block_name] : source.get_subdomain_name_map())
    destination.set_subdomain_name_map().insert(
        std::make_pair<SubdomainID, SubdomainName>(block_id + block_offset, block_name));

  // Check for the case with two boundary ids sharing the same name
  if (_avoid_merging_boundaries)
  {
    for (const auto & [b_id, b_name] : other_boundary.get_sideset_name_map())
      for (const auto & [source_id, source_name] : boundary.get_sideset_name_map())
        if (b_name == source_name)
          paramWarning(
              "avoid_merging_boundaries",
              "Not merging boundaries is creating two sidesets with the same name '" + b_name +
                  "' but different ids: " + std::to_string(source_id) + " & " +
                  std::to_string(b_id + bid_offset) +
                  ".\n We recommend using a RenameBoundaryGenerator to prevent this as you "
                  "will get errors reading the Exodus output later.");
    for (const auto & [b_id, b_name] : other_boundary.get_nodeset_name_map())
      for (const auto & [source_id, source_name] : boundary.get_nodeset_name_map())
        if (b_name == source_name)
          paramWarning(
              "avoid_merging_boundaries",
              "Not merging boundaries is creating two nodesets with the same name '" + b_name +
                  "' but different ids: " + std::to_string(source_id) + " & " +
                  std::to_string(b_id + bid_offset) +
                  ".\n We recommend using a RenameBoundaryGenerator to prevent this as you "
                  "will get errors reading the Exodus output later.");
  }

  for (const auto & [nodeset_id, nodeset_name] : other_boundary.get_nodeset_name_map())
    boundary.set_nodeset_name_map().insert(
        std::make_pair<BoundaryID, BoundaryName>(nodeset_id + bid_offset, nodeset_name));

  for (const auto & [sideset_id, sideset_name] : other_boundary.get_sideset_name_map())
    boundary.set_sideset_name_map().insert(
        std::make_pair<BoundaryID, BoundaryName>(sideset_id + bid_offset, sideset_name));

  for (const auto & [edgeset_id, edgeset_name] : other_boundary.get_edgeset_name_map())
    boundary.set_edgeset_name_map().insert(
        std::make_pair<BoundaryID, BoundaryName>(edgeset_id + bid_offset, edgeset_name));
}
