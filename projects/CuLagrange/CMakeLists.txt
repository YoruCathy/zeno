cmake_minimum_required(VERSION 3.19)

if(NOT ZENO_WITH_CUDA)
  message(FATAL_ERROR "CUDA must be ON when CuLagrange is ON! "
    "Please specify: -DZENO_WITH_CUDA:BOOL=ON -DZENO_WITH_zenvdb:BOOL=ON -DZENO_WITH_ZenoFX:BOOL=ON")
endif()

#[=[
target_sources(zeno PRIVATE
  test1.cu
)
#]=]
set_target_properties(zeno
  PROPERTIES
  CUDA_SEPARABLE_COMPILATION ON
  CUDA_RESOLVE_DEVICE_SYMBOLS ON
)

#[=[
target_sources(zeno PRIVATE
  POC/jit.cu
)
#]=]

# mpm
target_sources(zeno PRIVATE
  mpm/Boundary.cu
  mpm/Generation.cu
  mpm/Generation.cpp
  mpm/Transfer.cu
  mpm/MPMPipeline.cu
  mpm/Maintenance.cu
  mpm/Temp.cu
)

# pbd
target_sources(zeno PRIVATE
  pbd/Constraints.cu
  pbd/SelfCollision.cu
  pbd/KinematicCollision.cu
  # pbd/Pipeline.cu
)

# fem-cloth
target_sources(zeno PRIVATE

  # fem/ClothInit.cu
  # fem/ClothPipeline.cu
  # fem/ClothLinearSystem.cu
  # fem/ClothCollision.cu
  fem/FastClothInit.cu
  fem/FastClothPipeline.cu
  fem/FastClothLinearSystem.cu
  fem/FastClothCollision.cu
  fem/UpdateTargetFromMapping.cu 
  fem/SetIsBCFromPrim.cu
)

# fem-rapid-cloth 
target_sources(zeno PRIVATE 
  fem/rapid_cloth/RapidCloth.cuh 
  fem/rapid_cloth/RapidClothInit.cu
  fem/rapid_cloth/RapidClothLinearSystem.cu
  fem/rapid_cloth/RapidClothCollision.cu
  fem/rapid_cloth/RapidClothPrimitives.cu
  fem/rapid_cloth/RapidClothPipeline.cu 

  fem/ZSParticlesConstraints.cu
)

# fem
target_sources(zeno PRIVATE
  fem/SolverInit.cu
  fem/SolverPipeline.cu
  fem/SolverLinearSystem.cu
  fem/SolverIPC.cu

  fem/ipc/UnifiedPreconditioner.cu
  fem/ipc/UnifiedSolverInit.cu
  fem/ipc/UnifiedSolverPipeline.cu
  fem/ipc/UnifiedSolverLinearSystem.cu
  fem/ipc/UnifiedSolverUtilities.cu
  fem/ipc/UnifiedSolverIPC.cu

  fem/BoundaryCondition.cu
  fem/Collisions.cu

  # fem/Check.cu
  fem/Generation.cpp

  fem/FleshDynamicStepping.cu # CHECK THIS
  fem/collision_energy/vertex_face_collision.hpp
  fem/collision_energy/vertex_face_sqrt_collision.hpp
  # fem/collision_energy/edge_edge_collision.hpp
  # fem/collision_energy/edge_edge_sqrt_collition.hpp
  fem/collision_energy/evaluate_collision.hpp
  fem/collision_energy/collision_utils.hpp
  # fem/TendonDynamicStepping.cu
  # fem/FEMFleshDynamicFast.cu
  # fem/StrainLimitRepulsion.cu
  # fem/KinematicCollisionRepulsion.cu
)

# geometry
target_sources(zeno PRIVATE
  geometry/file_parser/read_vtk_mesh.hpp
  geometry/file_parser/write_vtk_unstructured_mesh.hpp
  geometry/file_parser/vtk_types.hpp
  geometry/linear_system/mfcg.hpp
  geometry/linear_system/active_set.hpp
  geometry/kernel/differential_geometry.hpp
  geometry/kernel/gradient_field.hpp
  geometry/kernel/bary_centric_weights.hpp
  geometry/kernel/compute_characteristic_length.hpp
  geometry/kernel/topology.hpp
  geometry/kernel/calculate_facet_normal.hpp
  geometry/kernel/calculate_bisector_normal.hpp
  geometry/kernel/tiled_vector_ops.hpp
  geometry/kernel/intersection.hpp
  geometry/SolveLaplacian.cu
  geometry/VectorField.cu
  geometry/SetupSkinningWeight.cu
  geometry/Skinning.cu
  geometry/BaryCentricInterpolator.cu
  geometry/BiharmonicBoundedWeight.cu
  geometry/MeshIO.cu
  geometry/Jiggle.cu
  geometry/DeformationField.cu
  # geometry/VectorField.cu
  geometry/CollisionVis.cu # CHECK THIS
  # geometry/CalcSurfaceArea.cu
  geometry/BasicGeoNodes.cu
  geometry/MarkSurfaceTag.cu
  geometry/SurfaceBinder.cu
  geometry/MarkInversion.cu
  geometry/Topology.cu
  geometry/MarkCollisionRegion.cu # CHECK THIS
)
