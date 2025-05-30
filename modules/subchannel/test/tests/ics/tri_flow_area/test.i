[TriSubChannelMesh]
  [subchannel]
    type = SCMTriSubChannelMeshGenerator
    n_cells = 2
    nrings = 3
    flat_to_flat = 0.60
    heated_length = 1.0
    pin_diameter = 0.1
    pitch = 0.13
    dwire = 0.03
    hwire = 0.3
    spacer_k = '0.5'
    spacer_z = '0'
  []
[]

[Variables]
  [S]
  []
[]

[ICs]
  [S_IC]
    type = SCMTriFlowAreaIC
    variable = S
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
