# COMS 336 Renderer

## Build and Run
Requires GCC and Make. Developed on Windows 11 using MSYS2, but presumably will work on Linux/Mac too.

```
make all
./build/render.exe
```


## Tasks
- [ ] A camera with configurable position, orientation, and field of view
- [ ] Anti-aliasing
- [ ] Ray/sphere intersections
- [ ] Ray/triangle intersections
- [ ] The ability to load textures (file format(s) of your choice; may use third-party libraries)
- [ ] Textured spheres and triangles
- [ ] The ability to load and render triangle meshes (file format(s) of your choice; may use third-party libraries for loading)
- [ ] A spatial subdivision acceleration structure of your choice
- [ ] Specular, diffuse, and dielectric materials (per first volume of Ray Tracing in One Weekend series)
- [ ] Emissive materials (lights)

Required extras (C-):
- [ ] Importance sampling (15)
- [ ] Parallelization (10)

Non required extras:
- [ ] Object instancing(???) (10)
- [ ] BRDFs (30)