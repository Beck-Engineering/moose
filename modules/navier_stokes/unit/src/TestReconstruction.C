//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "Registry.h"
#include "MooseMesh.h"
#include "NavierStokesApp.h"
#include "AppFactory.h"
#include "Factory.h"
#include "InputParameters.h"
#include "MeshGeneratorMesh.h"
#include "MooseError.h"
#include "CastUniquePointer.h"
#include "GeneratedMeshGenerator.h"
#include "MooseFunctor.h"
#include "PolynomialFit.h"
#include "FaceInfo.h"
#include "MooseTypes.h"
#include "CellCenteredMapFunctor.h"
#include "Reconstructions.h"
#include "MooseUtils.h"

#include "libmesh/elem.h"
#include "libmesh/tensor_value.h"
#include "libmesh/point.h"
#include "libmesh/vector_value.h"
#include "libmesh/utility.h"

#include <memory>
#include <vector>
#include <memory>

void
testReconstruction(const Moose::CoordinateSystemType coord_type)
{
  const char * argv[2] = {"foo", "\0"};

  MultiMooseEnum coord_type_enum("XYZ RZ RSPHERICAL", "XYZ");
  coord_type_enum = (coord_type == Moose::COORD_XYZ) ? "XYZ" : "RZ";

  std::vector<unsigned int> num_elem = {64, 128, 256};
  std::vector<Real> weller_errors;
  std::vector<Real> moukalled_errors;
  std::vector<Real> tano_errors;
  std::vector<Real> tano_errors_twice;
  std::vector<Real> h(num_elem.size());
  for (const auto i : index_range(num_elem))
    h[i] = 1. / num_elem[i];

  for (const auto i : index_range(num_elem))
  {
    const auto nx = num_elem[i];
    auto app = AppFactory::createAppShared("NavierStokesUnitApp", 1, (char **)argv);
    auto * factory = &app->getFactory();
    std::string mesh_type = "MeshGeneratorMesh";

    std::shared_ptr<MeshGeneratorMesh> mesh;
    {
      InputParameters params = factory->getValidParams(mesh_type);
      mesh = factory->create<MeshGeneratorMesh>(mesh_type, "moose_mesh", params);
    }

    app->actionWarehouse().mesh() = mesh;

    {
      std::unique_ptr<MeshBase> lm_mesh;
      InputParameters params = factory->getValidParams("GeneratedMeshGenerator");
      params.set<unsigned int>("nx") = nx;
      params.set<unsigned int>("ny") = nx;
      params.set<MooseEnum>("dim") = "2";
      auto mesh_gen =
          factory->create<GeneratedMeshGenerator>("GeneratedMeshGenerator", "mesh_gen", params);
      lm_mesh = mesh_gen->generate();
      mesh->setMeshBase(std::move(lm_mesh));
    }

    mesh->prepare(nullptr);
    mesh->setCoordSystem({}, coord_type_enum);
    mooseAssert(mesh->getAxisymmetricRadialCoord() == 0,
                "This should be 0 because we haven't set anything.");
    const auto & all_fi = mesh->allFaceInfo();
    mesh->buildFiniteVolumeInfo();
    mesh->computeFiniteVolumeCoords();
    std::vector<const FaceInfo *> faces(all_fi.size());
    for (const auto i : index_range(all_fi))
      faces[i] = &all_fi[i];

    auto & lm_mesh = mesh->getMesh();

    CellCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>> u(
        *mesh, "u", /*extrapoalted_bd=*/true);
    for (auto * const elem : lm_mesh.active_element_ptr_range())
    {
      const auto centroid = elem->vertex_average();
      const auto value = RealVectorValue(-std::sin(centroid(0)) * std::cos(centroid(1)),
                                         std::cos(centroid(0)) * std::sin(centroid(1)));
      u[elem->id()] = value;
    }

    CellCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>>
        up_weller(*mesh, "up_weller", /*extrapoalted_bd=*/true);
    CellCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>>
        up_moukalled(*mesh, "up_moukalled", /*extrapoalted_bd=*/true);
    CellCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>>
        up_tano(*mesh, "up_tano", /*extrapoalted_bd=*/true);

    for (const auto & fi : all_fi)
    {
      auto moukalled_reconstruct = [&fi](auto & functor, auto & container)
      {
        auto face = Moose::FaceArg(
            {&fi, Moose::FV::LimiterType::CentralDifference, true, false, nullptr, nullptr});
        const RealVectorValue uf(functor(face, Moose::currentState()));
        const Point surface_vector = fi.normal() * fi.faceArea();
        auto product = (uf * fi.dCN()) * surface_vector;

        container[fi.elem().id()] += product * fi.gC() / fi.elemVolume();
        if (fi.neighborPtr())
          container[fi.neighbor().id()] +=
              std::move(product) * (1. - fi.gC()) / fi.neighborVolume();
      };

      moukalled_reconstruct(u, up_moukalled);
    }

    Moose::FV::interpolateReconstruct(up_weller, u, 1, true, faces, Moose::currentState());
    Moose::FV::interpolateReconstruct(up_tano, u, 1, false, faces, Moose::currentState());

    Real error = 0;
    Real weller_error = 0;
    Real tano_error = 0;
    Real moukalled_error = 0;
    const auto current_h = h[i];
    for (auto * const elem : lm_mesh.active_element_ptr_range())
    {
      const auto elem_id = elem->id();
      auto elem_arg = Moose::ElemArg{elem, false};
      const RealVectorValue analytic(u(elem_arg, Moose::currentState()));

      auto compute_elem_error =
          [elem_id, current_h, elem, &analytic](auto & container, auto & error)
      {
        auto & current = libmesh_map_find(container, elem_id);
        const auto diff = analytic - current;
        error += diff * diff * current_h * current_h;

        // Test CellCenteredMapFunctor ElemPointArg overload
        const auto elem_point_eval = container(
            Moose::ElemPointArg({elem, elem->vertex_average(), false}), Moose::currentState());
        for (const auto d : make_range(Moose::dim))
          EXPECT_TRUE(MooseUtils::absoluteFuzzyEqual(current(d), elem_point_eval(d)));
      };

      compute_elem_error(up_weller, weller_error);
      compute_elem_error(up_moukalled, moukalled_error);
      compute_elem_error(up_tano, tano_error);
    }
    error = std::sqrt(error);
    weller_error = std::sqrt(weller_error);
    moukalled_error = std::sqrt(moukalled_error);
    tano_error = std::sqrt(tano_error);
    weller_errors.push_back(weller_error);
    moukalled_errors.push_back(moukalled_error);
    tano_errors.push_back(tano_error);

    up_tano.clear();
    Moose::FV::interpolateReconstruct(up_tano, u, 2, false, faces, Moose::currentState());

    tano_error = 0;
    for (auto * const elem : lm_mesh.active_element_ptr_range())
    {
      const auto elem_id = elem->id();
      const auto elem_arg = Moose::ElemArg{elem, false};
      const RealVectorValue analytic(u(elem_arg, Moose::currentState()));

      auto compute_elem_error = [elem_id, current_h, &analytic](auto & container, auto & error)
      {
        auto & current = libmesh_map_find(container, elem_id);
        const auto diff = analytic - current;
        error += diff * diff * current_h * current_h;
      };
      compute_elem_error(up_tano, tano_error);
    }
    tano_error = std::sqrt(tano_error);
    tano_errors_twice.push_back(tano_error);

    CellCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> unrestricted_error_test(
        *mesh, "not_restricted", /*extrapoalted_bd=*/true);
    try
    {
      unrestricted_error_test(Moose::ElemArg({lm_mesh.elem_ptr(0), false}), Moose::currentState());
      EXPECT_TRUE(false);
    }
    catch (std::runtime_error & e)
    {
      EXPECT_TRUE(std::string(e.what()).find("not_restricted") != std::string::npos);
      EXPECT_TRUE(std::string(e.what()).find("Make sure to fill") != std::string::npos);
    }

    CellCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> restricted_error_test(
        *mesh, {1}, "is_restricted", /*extrapoalted_bd=*/true);
    try
    {
      restricted_error_test(Moose::ElemArg({lm_mesh.elem_ptr(0), false}), Moose::currentState());
      EXPECT_TRUE(false);
    }
    catch (std::runtime_error & e)
    {
      EXPECT_TRUE(std::string(e.what()).find("is_restricted") != std::string::npos);
      EXPECT_TRUE(std::string(e.what()).find("0") != std::string::npos);
      EXPECT_TRUE(
          std::string(e.what()).find(
              "that subdomain id is not one of the subdomain ids the functor is restricted to") !=
          std::string::npos);
    }
  }

  std::for_each(h.begin(), h.end(), [](Real & h_elem) { h_elem = std::log(h_elem); });

  auto expect_errors = [&h](auto & errors_arg, Real expected_error)
  {
    std::for_each(
        errors_arg.begin(), errors_arg.end(), [](Real & error) { error = std::log(error); });
    PolynomialFit fit(h, errors_arg, 1);
    fit.generate();

    const auto & coeffs = fit.getCoefficients();
    EXPECT_NEAR(coeffs[1], expected_error, .05);
  };

  expect_errors(weller_errors, coord_type == Moose::COORD_RZ ? 1.5 : 2);
  expect_errors(moukalled_errors, 2);
  expect_errors(tano_errors, 2);
  expect_errors(tano_errors_twice, 2);
}

TEST(TestReconstruction, Cartesian) { testReconstruction(Moose::COORD_XYZ); }

TEST(TestReconstruction, Cylindrical) { testReconstruction(Moose::COORD_RZ); }
