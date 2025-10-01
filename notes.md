# COMS 336
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

### Textures
Texture coordinates are expressed as $<u, v>$ in the range [0, 1).




## Antialiasing
Removing the stairstep effect. Must be done by subsampling each pixel, but you can subsample a few different ways:
- Equal division: Split the pixel into an n * n grid. Send a ray out from the center of each subpixel. Works, but you can systematically miss an object because the placement of your rays is fixed. Fixes aliasing, but just makes the stairsteps smaller.
- Random sampling: Send n random rays from random positions in each pixel into the scene. Average the colors. Randomness means you likely won't entirely miss the thing you're trying to look at, and at least won't miss it consistently. When we're rendering a final image we want to use ~1000 subsamples.


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