# Peridynamics Mesh Generator

## Description

The `MeshGeneratorPD` MeshGenerator selectively converts finite element meshes and boundary sets to peridynamics meshes and corresponding boundary sets. For cases when more than one element block exist in the finite element mesh, user can specify which element block to convert to peridynamics elements and whether the converted finite elements will be retained with the newly created peridynamics elements or not.

!syntax parameters /Mesh/MeshGeneratorPD

!syntax inputs /Mesh/MeshGeneratorPD

!syntax children /Mesh/MeshGeneratorPD
