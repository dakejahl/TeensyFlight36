declare_PID_Component(
    EXAMPLE_APPLICATION
    NAME fitting-example
    DIRECTORY fitting_example
)

declare_PID_Component_Dependency(
    COMPONENT fitting-example
    NATIVE ellipsoid-fit
)
