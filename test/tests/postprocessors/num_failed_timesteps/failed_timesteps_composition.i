[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  []
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 10
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  [TimeSteppers]
    [constant_1]
      type = ConstantDT
      dt = 0.2
    []
    [constant_2]
      type = ConstantDT
      dt = 0.2
    []
  []
[]

[Problem]
  type = FailingProblem
  fail_steps = '1 1 1 2 4 5'
[]

[Postprocessors]
  [num_failed]
    type = NumFailedTimeSteps
  []
[]

