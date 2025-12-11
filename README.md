# COMS 336 Renderer

## Build and Run
Requires GCC and Make. Developed on Windows 11 using MSYS2, but presumably will work on Linux/Mac too.

```
make all
./build/render.exe
```

## Third Party Libraries (/lib)
- nlohmann's JSON parsing library
- TinyOBJLoader
  - RapidOBJ is a newer, faster version of TinyOBJLoader but it doesn't support the Windows/MSYS combo. It's not Windows or Linux so it doesn't know what to do

## Future Improvements
- Potential idea: Refactor json files into objects list and index list. Could allow more reusability
- Refactor camera into a fixed image plane size that subdivides into some number of pixels
  - Makes our resolution very flexible, makes the scene coordinates smaller and not resolution-dependent

## Tasks
- [x] A camera with configurable position, orientation, and field of view
- [x] Anti-aliasing
- [x] Ray/sphere intersections
- [x] Ray/triangle intersections
- [x] The ability to load textures (file format(s) of your choice; may use third-party libraries)
      - Done through tinyobjloader
- [ ] Textured spheres and triangles
- [x] The ability to load and render triangle meshes (file format(s) of your choice; may use third-party libraries for loading)
      - Done through tinyobjloader
- [ ] A spatial subdivision acceleration structure of your choice
- [x] Specular, diffuse, and dielectric materials (per first volume of Ray Tracing in One Weekend series)
- [x] Emissive materials (lights)

Required extras (C-):
- [ ] Importance sampling (15)
- [x] Parallelization (10)

Non required extras:
- [x] Object instancing: being able to copy/rescale/rotate the same object throughout the scene (10)
      - Done with scene JSON and preventing multiple loads of the same OBJ file.
- [ ] BRDFs (30)
- [x] Quads (10)