# COMS 336
https://raytracing.github.io/

Two common algorithms:
- Depth buffer (rasterization) algorithm (how GPUs do it)
- Ray tracing


## Depth Buffer/Rasterization
Specifically called raster because the rasterization is done very early. Ray tracing does it last in the pipeline.

Convert everything into triangles (because they're always going to be planar), more triangles means more smoothness
- You could use dedicated hardware for spheres, but in raster it's more efficient to just approximate it. It's a waste of die space

Send the GPU the vertices and the connectivity (list of edges and where they connect)

Perform a matrix multiplication (model-view matrix) to transform the triangles from the model coordinates into the scene we're rendering

The rasterizer projects the 3d view into the 2d screen space

Test for visibility using the depth buffer (data structure that holds the depth of every pixel in the scene, starts at inf and if we get a point that's closer then it gets updated. sorta like a framebuffer but only with depth information)
- We can't determine the order of depth based on (for example) the order of objects in a list because if we move the camera the closest object to the camera changes
- Glass is complex, if you want it to be convincing you can add fancy reflections. Ideally you want to do it last

Typical depth complexity (number of depth buffer changes) is > 10 in modern games. Early occlusion/depth tests are a performance trick, usually done by the triangle

Stencil buffer: How HUDs and overlays are made, basically a mask around your HUD. You make a stencil of your HUD in the stencil buffer and only allow writes to the framebuffer outside HUD stencil.

### Rendering Pipeline
1. off-chip memory or host processor
2. vertex processor: vertex transformation (mvp matrix) and lighting (phong/gouraud)
3. vertex cache
4. rasterizer
   1. triangle setup: use connectivity to build triangles from vertices (reorder buffer)
   2. rasterization: map triangles to pixels on a screen
      - For each triangle that comes in, draw a bounding box around the triangle and iterate through the bounding box. Color the pixel if the center of the pixel overlaps with the triangle.
      - Alternative: subdivide the screen space, only iterate through the subdivisions that overlap the triangle. Works better for long thin triangles.
      - Deciding if we're inside the triangle is done with barycentrics or plane equations for the 3 sides
5. uncolored fragment queue
6. fragment processor: apply textures and lighting, calculate the final color
7. colored fragment queue
8. framebuffer operations: depth buffer check, stencil check, transparency
   - Do the depth buffer check here because the fragment processor can modify the position of pixels (displacement maps)
9. framebuffer

After the switch to programmability, we realized that vertex and fragment cores are basically the same and we can use the same hardware with dynamic allocation.


## Ray Tracing
Send a ray from the camera (a point) through a pixel on the screen into the scene. Bounce it around until it hits a light or you time out

Do an intersection test with your ray and every object in the scene. If we hit an object, keep following

If we hit a light, calculate the attenuation from every surface the ray hit to get the right color

A ray is 2 vectors: origin (O) and direction (D). This makes the function for the position of a light particle: $P(t) = O + tD$, where t is where we are along the ray.

### The camera
We set up a pinhole camera at a point in the scene and put the image plane (screen) in front of the camera. It simplifies the math.

We can also use more complex setups that simulate lenses and depth of field (bokeh), but that makes things harder.

Start by choosing an ideal aspect ratio (ideal because pixels aren't exactly square).

Choose a focal length: Focal length is the distance from our pinhole to the image plane

Shoot rays from the camera through the center of each pixel on the image plane into the scene and let them bounce around. If we hit an object, return the color of the object

### Collision detection
#### Ray-sphere
Standard equation of a sphere: $(x - x_0)^2 + (y - y_0)^2 + (z - z_0)^2 = r^2$

Using vector notation with point on the sphere P, center C, and radius R: $|P - C| = R^2 \to (P - C) \cdot (P - C) = R^2$

We can turn this into a nasty quadratic equation: no real roots means we missed, one root means we hit it tangentially, two roots means we went through

#### Ray-triangle
We can check if a point P hit a triangle ABC by calculating the barycentric coordinates.
- Check ray-plane intersection to find t (see below)
- Put t into our ray to find P
- Turn the triangle into 3 sub-triangles: PAB, PAC, PBC. Name them C, B, A respectively
- Calculate the normals as:
  - $n_a = (c - b) x (p - b)$
  - $n_b = (a - c) x (p - c)$
  - $n_c = (b - a) x (p - a)$
- Calculate the coordinates as $\frac{Area_a}{Area_{ABC}}, \frac{Area_b}{Area_{ABC}}, \frac{Area_c}{Area_{ABC}}$
- Alternatively, calculate faster with $\frac{n \cdot n_a}{|n|^2}, \frac{n \cdot n_b}{|n|^2}, \frac{n \cdot n_c}{|n|^2}$
- If P did not intersect ABC, at least one of the barycentric coordinates will be negative
  - The two edge cases (ray in the plane and a barycentric == 0) don't matter in practice
- Coords usually named $\alpha, \beta, \gamma$

To get a normal vector of a triangle face ABC: $n_{abc} = (c - b) x (a - b)$. Make sure that the order is correct to get something right-handed.

To get the area of ABC: $Area = \frac{n_{abc}}{2}$

#### Ray-plane
Defined by a point and a normal vector

We want to calculate the normal vector immediately, it's super useful everywhere

If a vector V intersects a plane made of a point P and normal N: $(V - P) \cdot n = 0$
- Transforming this into our $O + tD$ vector form and simplifying: $t = \frac{(P - O) \cdot n}{D \cdot n}$
- If $D \cdot n$ is 0, that implies the plane is parallel to the ray

### Reflections
Conserving energy: If we split a ray into multiple rays upon collision, make sure we conserve energy. That means dividing your color by $n_{rays}$ to darken it. You can weight this too, as long as the weights add to 1.

#### Specular
Essentially this is a perfect collision. The ray comes intersects the object at a particular angle and it will bounce off at exactly the same angle in the same plane. Think a billiard ball hitting the wall of the table.

#### Diffuse
Lambertian reflection: Place a unit sphere (radius 1) at the end of the unit normal vector from the surface. Pick a random point on the sphere and shoot your reflected ray through it.

We don't want to send multiple rays (scatter rays) from diffuse objects because it makes the computation too hard. With enough rays from the camera and enough random bounces the scattering will handle itself.

`color of output ray = color of material * color of incoming ray`

Perfectly colored/perfectly reflective colors aren't realistic (i.e. hex code 0xFF0000 for red), the real world will be more like 0xFE0000 and you'll lose some light at every bounce.

### Refractions
#### Dielectrics (glass, water, gems)
Snell's law: $\eta \sin \theta = \eta' \sin \theta'$ where $\theta$ is relative to the normal vector of the dielectric surface.
- $\eta_{air}$ = 1.0003 at STP
- $\eta_{glass}$ = 1.3 - 1.7
- If $\eta / \eta' \sin \theta > 1$, there's no solution for $\theta '$ and you have total internal reflection (no refraction).

Refraction slows light

Glass both reflects and refracts. You can handle this by random chance of reflection/refraction or do both reflection and refraction (but conserve energy).
- The random chance follows a formula, but most raytracers approximate it using Schlick's approximation.
```
schlick(cos_angle_indicence, ratio_indices_refraction) {
  t = (1 - rir) / (1 + rir)
  t = t * t
  return t + (1 - t) * pow(1 - cai, 5)
}

if (total_internal_reflection or rand(0, 1) < schlick()) {
  reflect() // using reflection from specular section above
} else {
  refract()
}
```

To calculate the vector coming out of the refraction, where n is the normal vector of the glass:
- $r' = r_{||} + r_{\perp}$
- $r_{||} = \sqrt{1 - |r_{\perp}|^2} * n$
- $r_{\perp} = $

### Lights (Emissive Surfaces)
Lights are just surfaces that set the color of a ray and don't reflect. They can be triangles, quads, spheres, anything.

Lights can be brighter than (255, 255, 255). This makes them able to withstand multiple bounces without completely attenuating.

Lights can be textured like any other surface. That allows you to make a blue sky or different color temperatures.

### Textures
Texture coordinates are expressed as $<u, v>$ in the range [0, 1).

### Acceleration Structures
Don't test collision with every object, only test collision with the objects inside a bounding box (the objects you're likely to hit).

Bounding Volume Hierarchy:
- All geometry must have bounding boxes
- Bounding boxes can contain bounding boxes, it's a tree. Single bounding box for the entire scene, then divide in halves from there. The individual geometries are the leaves.
  - Divide in half based on the longest axis of the bounding box
- Construction
  - Construct bounding boxes for each object and for the full scene
  - Sort the geometry in each dimension
  - Divide the bounding box on the longest dimension such that half of the geometries on are on each side
    - Sub-boxes might cross our division, just add it to both sides of the tree. The ray will go to one of the tree or the other
  - Continue recursively until each bounding box only has one thing inside
- Usage
  - Traverse the tree. If we hit the bounding box at a node, keep going. Check both child nodes and see which one we hit. If we hit both, traverse into both sides


### Importance Sampling
Bias ray reflections so that more of them go to important places (the light).

Monte Carlo integration: We don't know what kind of incoming signal we have, but we want to compute the integral.
- Las Vegas Algorithms: Random algs that will always give the right answer
- Monte Carlo Algorithms: Random algs that will converge on the right answer


## Antialiasing
Removing the stairstep effect. Must be done by subsampling each pixel, but you can subsample a few different ways:
- Equal division: Split the pixel into an n * n grid. Send a ray out from the center of each subpixel. Works, but you can systematically miss an object because the placement of your rays is fixed. Fixes aliasing, but just makes the stairsteps smaller.
- Random sampling: Send n random rays from random positions in each pixel into the scene. Average the colors. Randomness means you likely won't entirely miss the thing you're trying to look at, and at least won't miss it consistently. When we're rendering a final image we want to use ~1000 subsamples.


## Texture Filtering
Reducing aliasing and blocking in your textures using various algorithms. A pixel on the screen may not directly map to a pixel on a particular texture in the scene, so we need to determine the best color for that pixel. This results in artifacting, solved by texture filtering.

Methods:
- Nearest neighbor: Texel with its center closest to the pixel center is the assigned color
- Mipmapping: Caching a stack of scaled down textures, all the way from the original texture to a 1x1 pixel.
  - Not usually used in ray tracing because we can just throw more rays into the scene. Raster algorithms need the correct color immediately
- Linear: Interpolate linearly between the two closest mipmaps
- Bilinear: Interpolate linearly between the 4 closest texels in between the two closest mipmaps
- Anisotropic: To get the color of a road fading out to infinity, you need very little range of X and Y but you need to average miles of Z.

Magnification: A texel takes up more space than a screen pixel, scale up the texels appropriately. Interpolating between sparse textures to fill gaps
- Nearest neighbor: Blocky
- Linear:

Minification: A texel takes up less space than a screen pixel, scale down the texels appropriately. Anti-aliasing a texture that's much larger than necessary
- Nearest neighbor: Shimmery and slow
- Linear:


## Scene Graphs
Networks of nodes in a scene where each subnode receives the transformation applied to its parent. Used to move body parts on models.


## Radiosity
We can precompute the lighting for a room to simplify our computations later. If we divide all of our surfaces into patches and precompute the "global illumination" (the ambient light in the room) we end up with every patch as a light source of a particular brightness. If we walk into the room we can compute the lighting after the fact with Whitted-style ray tracing.


## Shadow Mapping
The dominant algorithm for how GPUs handle fancy shadows today.


## Shadow Volumes
Look better than shadow maps, but are more computationally expensive. Shadows will be entirely alias-free. Used in Doom 3.

Extrude a volume for each primitive (triangle) in the scene that goes away from the light source. If a fragment (pixel) is in a volume then it is shadowed. Otherwise, it's lit. Repeat for every light source.

There are stencil buffer hacks to make this more performant. There are still issues if the camera is in the shadows, see the Carmack reversal.


## How Rendering Works
Solid angle: how much area an object takes when projected on a sphere centered on the camera. Measured in steradians, there are 4pi steradians in a sphere
- This is a measure of *area*, with no bounds on the shape

The Rendering Equation/Light Transport Equation

$L_o(p_1, \omega_0) = L_e(p_1, \omega_0) + \int_{s^2} f(p_1, \omega_0, \omega_i) \cdot L_i(p_1, \omega_i) \cdot \cos \theta_i \ d \omega_i$

Outgoing light at point p1 and solid angle w = Emitted light + integral_surface of sphere centered on p1 (Bi Directional Scattering Function * Incoming light * foreshortening term)

What this does: Calculate what the light coming out of p1 at a particular solid angle looks like

Because this function is recursive (light bounces and you have to track it), it has infinite dimensions/inputs. We can't solve it exactly but we can approximate

If we're hitting a mirror, we can track the light bounces perfectly. However, we're not always hitting a mirror: if we hit something diffuse (dull) it scatters light around. The best solution here is to just pick ray and go for it. If we pick a different random ray every time it ends up looking natural


## Cameras
Pinhole cameras work because every ray from the source can only go to one point on the film/sensor/screen. Anything you see through a pinhole camera will be upside down. This is how our raytracer will work initially (you can modify it to work like a camera with a lens)

Wide apertures let in a ton of light but have a shallow depth of field (not very sharp). Small apertures let in very little light but are very sharp (smaller apertures are sharper, until you reach pinhole).

Perspective camera: things farther away look smaller. Parallel lines converge except those orthogonal to the view direction. In ray tracing, amounts to shooting rays from the camera through the image plane. Because the rays aren't parallel it gives you perspective.

Orthographic projection: everything remains parallel, dimensions aren't warped due to perspective. Primarily for engineering drawings or city builders. In ray tracing, amounts to shooting parallel rays from the image plane rather than using a "camera".


## Color theory
Colors of pigments: Red, Yellow, Blue (orange, green, purple). Older, we can get *most* colors but not all of them. Mixing RYB doesn't give black either.

Colors of light: Red, Green, Blue (yellow, cyan, magenta). Newer, much more precise. We can get true black by mixing CMY and true white by mixing RGB. We use black for printing because it's easier than mixing CMY.


## Basic linalg
Dot product: Commonly used in $\cos \theta = \frac{v \cdot w}{|v||w|}$. 1 = colinear, 0 = orthogonal

Cross product: Gives you a vector orthogonal to both of the input vectors, follows right hand rule. Good for forming bases

### Object Modification
**All modification matrices can be multiplied together an applied at once!**

Rotation matrix: Rotates a vector by an angle. Go google it

Scaling matrix: Scales a vector by some scale factor in x, y, z.

Reflection matrix: Reflect over an axis by scaling by -1. You can also reflect over an arbitrary plane with a more complicated matrix, used for mirrors in raster graphics

Skew matrix: Skew a model

Homogeneous coordinates: Adding a fourth term to our matrices and vectors to implement perspective scaling and translation. The fourth term is initialied to 1 and called w. To convert from homogeneous to standard, divide each term by w.

Translation matrix: Requires a 4x4 matrix. Google it.

Perspective matrix: With an input vector x, y, z where x, y are in the image plane and z is directly out from the camera, finds the x and y that we map to on the screen(!!!!). Essentially divides each coordinate by z.

View matrix: Translates a vector into camera space. Combined with the perspective matrix we can map a vector from the world into the camera and into the screen.

Rotating around an arbitrary vector x, y, z by an angle: Used a TON for moving body parts on models or doing animation. Giant nasty matrix. Quaternions provide a much nicer way of writing this.

Quaternions: Used in graphics for one thing: rotations. Only used because the rotation around an arbitrary vector looks hideous, the matrix version does the same operation. Looks like a 4-vector, consists of $p = <s, u, v, w> = <s, V>$. Make a second one, call it q.
- $pq = <s_p s_q - V_p \cdot V_q, V_p \times V_q + s_p V_q + s_q + V_p>$
- $p^{-1} = <s, -x, -y, -z>$
- Axis and angle to quaternion: $v, \theta$: $<\cos \theta/2, x \sin \theta/2, y \sin \theta/2, z \sin \theta/2>$
- Rotating a vector p around quaternion q: $p = <0, p>, p' = q^{-1} p q$

## Image Manipulation
Convolution: Take an odd by odd dimensioned matrix (3x3, 5x5) and slide it over the input image so it's centered on every pixel once. Multiply the overlapping pixel of the image with the overlapping convolution matrix element. Add together all of the multiplies (dot product style) and save the resulting scalar in a new image in the pixel overlapped by the center of the convolution matrix.

All of the entries in the convolution matrix (kernel) add to 0 to preserve the light energy in the image.

Sobel edge detection matrices (best on black and white):
```
horizontal edges:
[[ 1,  2,  1],
 [ 0,  0,  0],
 [-1, -2, -1]]

vertical edges:
(transpose the matrix above)
```

Sharpening:
```
[[ 0, -1, 0],
 [-1,  4, 1],
 [ 0, -1, 0]]

Gaussian blur:
(1/16) *
[[1, 2, 1],
 [2, 4, 2],
 [1, 2, 1]]