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


## How Rendering Works
Solid angle: how much area an object takes when projected on a sphere centered on the camera. Measured in steradians, there are 4pi steradians in a sphere
- This is a measure of *area*, with no bounds on the shape

The Rendering Equation/Light Transport Equation

$L_o(p_1, \omega_0) = L_e(p_1, \omega_0) + \int_{s^2} f(p_1, \omega_0, \omega_i) \cdot L_i(p_1, \omega_i) \cdot \cos \theta_i \ d \omega_i$

Outgoing light at point p1 and solid angle w = Emitted light + integral_surface of sphere centered on p1 (Bi Directional Scattering Function * Incoming light * foreshortening term)

What this does: Calculate what the light coming out of p1 at a particular solid angle looks like

Because this function is recursive (light bounces and you have to track it), it has infinite dimensions/inputs. We can't solve it exactly but we can approximate

If we're hitting a mirror, we can track the light bounces perfectly. However, we're not always hitting a mirror: if we hit something diffuse (dull) it scatters light around. The best solution here is to just pick ray and go for it. If we pick a different random ray every time it ends up looking natural


## Pinhole cameras
Pinhole cameras work because every ray from the source can only go to one point on the film/sensor/screen. Anything you see through a pinhole camera will be upside down. This is how our raytracer will work initially (you can modify it to work like a camera with a lens)

Wide apertures let in a ton of light but have a shallow depth of field (not very sharp). Small apertures let in very little light but are very sharp (smaller apertures are sharper, until you reach pinhole).


## Color theory
Colors of pigments: Red, Yellow, Blue (orange, green, purple). Older, we can get *most* colors but not all of them. Mixing RYB doesn't give black either.

Colors of light: Red, Green, Blue (yellow, cyan, magenta). Newer, much more precise. We can get true black by mixing CMY and true white by mixing RGB. We use black for printing because it's easier than mixing CMY.


## Basic linalg
Dot product: Commonly used in $\cos \theta = \frac{v \cdot w}{|v||w|}$. 1 = colinear, 0 = orthogonal

Cross product: Gives you a vector orthogonal to both of the input vectors, follows right hand rule. Good for forming bases