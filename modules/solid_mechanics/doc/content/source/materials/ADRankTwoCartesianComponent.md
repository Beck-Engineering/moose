# ADRank Two Tensor Component

!syntax description /Materials/ADRankTwoCartesianComponent

## Description

This is a ADMaterial model used to extract components of a rank-2 tensor in a
Cartesian coordinate system. This can be used regardless of the coordinate
system used by the model.

This ADMaterial model is set up by
[SolidMechanics/QuasiStatic](/Physics/SolidMechanics/QuasiStatic/index.md) automatically
when stress components are requested in the generate_output parameter, but can
also be set up directly by the user.  

The `ADRankTwoCartesianComponent` takes as arguments the values of the
`index_i` and the `index_j` for the single tensor component to save into an
MaterialProperty.  [eq:rank2tensor_component_indices] shows the index values
for each Rank-2 tensor component.

!equation id=eq:rank2tensor_component_indices
\sigma_{ij} \implies \begin{bmatrix}
                      \sigma_{00} & \sigma_{01} & \sigma_{02} \\
                      \sigma_{10} & \sigma_{11} & \sigma_{12} \\
                      \sigma_{20} & \sigma_{21} & \sigma_{22}
                      \end{bmatrix}

!syntax parameters /Materials/ADRankTwoCartesianComponent

!syntax inputs /Materials/ADRankTwoCartesianComponent

!syntax children /Materials/ADRankTwoCartesianComponent
