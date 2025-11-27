# TJE Framework - Computer Graphics Reference

## PDF 1: Basics of Computer Graphics

### GPU and OpenGL Overview
- **OpenGL**: Multi-language, multi-platform API for 2D/3D graphics rendering
- **SDL**: Cross-platform library providing low-level access to audio, keyboard, mouse, joystick, and graphics
- **Framework uses OpenGL via SDL**

### Benefits of Graphics API
- Hardware acceleration (millions of polygons/sec)
- GPU handles rasterization, freeing CPU
- Low-level control for any application type

---

## Rendering Pipeline

### Common API Actions
1. Raster primitives (points, lines, triangles)
2. Apply transformations (project, translate, rotate, scale)
3. Depth testing via Z-Buffer
4. Texturing
5. Blending (transparencies)

### OpenGL Primitives
- `GL_POINTS`, `GL_LINES`, `GL_LINE_STRIP`, `GL_LINE_LOOP`
- `GL_TRIANGLES` (default in framework), `GL_TRIANGLE_STRIP`, `GL_TRIANGLE_FAN`
- `GL_QUADS`, `GL_QUAD_STRIP`, `GL_POLYGON`

### Rendering Steps
```cpp
void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // 1. Clear buffers
    // 2. Send geometry to GPU
    // 3. Execute draw call with primitive type
    glSwapBuffers();  // 4. Display final image
}
```

### OpenGL States
- `glEnable()` / `glDisable()` control pipeline steps
- States persist across draw calls
- `GL_DEPTH_TEST` - enables Z-buffer

---

## Coordinate Spaces & Matrices

### Pipeline: Local → World → View → Clip → Screen

| Space | Description |
|-------|-------------|
| **Local** | Vertices relative to mesh origin (0,0,0) |
| **World** | Vertices relative to scene origin after Model Matrix |
| **View** | Vertices relative to camera position |
| **Clip** | After projection, coordinates in [-1, 1] |
| **Screen** | Final pixel coordinates |

### Model Matrix (M = T × R × S)
- **Translation**: Position in world
- **Rotation**: Orientation (yaw, pitch, roll)
- **Scale**: Size multiplier
- Matrix columns define local X, Y, Z axes

### View Matrix
- Inverse of camera's TRS matrix
- `ViewMatrix = R⁻¹ × T⁻¹`
- Built from Side, Top, Forward vectors

### Projection Matrix
**Perspective** (frustum):
- Field of View (fov): angle width
- Aspect Ratio: width/height
- Near/Far planes

**Orthographic** (box):
- Left, Right, Top, Bottom, Near, Far

---

## Meshes

### Mesh Data Arrays
| Array | Description |
|-------|-------------|
| **Vertices** | 3D coordinates (X, Y, Z) - mandatory |
| **Normals** | Per-vertex normal vectors for lighting |
| **UVs** | Texture coordinates (U, V) in [0, 1] |
| **Colors** | RGBA per vertex (rarely used) |
| **Indices** | Triangle vertex indices (saves memory) |

### Indexing
- Without: vertices repeated per triangle
- With: reuse vertices via index array
- Significant memory savings

### Mesh Formats
| Format | Type | Notes |
|--------|------|-------|
| **OBJ** | ASCII | Geometry only, material separate, easy to parse |
| **ASE** | ASCII | Full scene + materials, heavier |
| **GLTF** | JSON | Full scene, well supported |
| Binary | - | Faster but less portable |

---

## Shaders (GLSL)

### Shader Types
- **Vertex Shader**: Executed per vertex → outputs `gl_Position` (clip-space)
- **Fragment Shader**: Executed per pixel → outputs `gl_FragColor`

### Variable Types
| Type | Description |
|------|-------------|
| `attribute` | Per-vertex input (gl_Vertex, gl_Normal, gl_MultiTexCoord0) |
| `uniform` | Constant from CPU (matrices, textures) |
| `varying` | Interpolated vertex→fragment |

### Built-in Variables
**Vertex Shader:**
- `gl_Vertex` (vec4): vertex position
- `gl_Normal` (vec4): vertex normal
- `gl_MultiTexCoord0` (vec2): texture coordinates
- `gl_Position` (vec4): OUTPUT - clip-space position

**Fragment Shader:**
- `gl_FragColor` (vec4): OUTPUT - final pixel color [0,1]
- `discard`: skip this pixel

### Framework Usage
```cpp
Shader* shader = Shader::Get("shader.vs", "shader.fs");
shader->enable();
shader->setUniform(...);
mesh->render();
shader->disable();
```

---

## Textures

### Purpose
- Add per-pixel detail to low-poly meshes
- Encode colors, normals, height, masks

### Texture Types
- **1D**: Gradients
- **2D**: Standard images
- **3D**: Volumetric (medical imaging)
- **Cubemap**: 6 faces for environment

### Texture Coordinates (UVs)
- 2D values [0, 1] per vertex
- Interpolated across triangle
- Map mesh surface to texture pixels

### Wrap Modes
- `GL_CLAMP_TO_EDGE`: Use border pixel
- `GL_REPEAT`: Tile texture (requires power-of-2 size: 32, 64, 128, 256, 512, 1024...)

### Filtering
- `GL_NEAREST`: Pixelated (faster)
- `GL_LINEAR`: Bilinear interpolation (smoother)

### Mipmaps
- Pre-computed half-size versions
- Reduces aliasing at distance
- GPU auto-selects based on UV discontinuity

### Common Texture Uses
- **Albedo/Diffuse**: Surface color
- **Normal Map**: Per-pixel normals (lighting detail)
- **Height Map**: Terrain elevation
- **Mask**: Blend between textures

---

## Blending

### Setup
```cpp
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
```
- Disabled by default
- Uses alpha channel for transparency

---

## Scene Exporter (Blender → Framework)

### File Format (.scene)
```
#NAME MODEL
scene/meshname/meshname.obj 4x4_matrix_values
empty_name 4x4_matrix_values
```

### Export Process
1. Exports each mesh as OBJ at origin (identity matrix)
2. Stores world transform as 4x4 matrix
3. Applies axis conversion: Blender (Z-up) → OpenGL (Y-up)
4. Supports MESH and EMPTY object types

### Matrix Format
16 comma-separated floats (column-major):
```
m00,m10,m20,m30,m01,m11,m21,m31,m02,m12,m22,m32,m03,m13,m23,m33
```

---

## Key Constants Reference

```cpp
// Depth testing
glEnable(GL_DEPTH_TEST);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// Texture wrap
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

// Texture filter
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

// Blending
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
```

---

## PDF 2: Scene Representation

### Programming Paradigms

#### OOP (Object-Oriented Programming)
- Classes represent objects (Airplane, Enemy, Camera)
- Data + methods packaged together
- Self-contained, independent, modular
- More intuitive for object manipulation

**Pros:** Readable, reusable, easy to extend
**Cons:** Memory waste (duplicated properties), fragmented memory, hard to batch

#### DOP (Data-Oriented Programming)
- Structure around data, functions manipulate containers
- Hardware-friendly, better cache performance
- Works in batches (render 1000 trees at once)

**Pros:** Faster processing, aligned memory, GPU-friendly
**Cons:** Slower to program, more code, less reusable

#### When to Use
- **OOP**: First-time 3D engine, generic solutions, learning
- **DOP**: Performance-critical, thousands of instances, specific cases
- Can mix both paradigms

---

## Entity System (OOP)

### Entity Base Class
```cpp
class Entity {
public:
    Entity();
    virtual ~Entity();

    std::string name;
    Matrix44 model;           // Transform (position, rotation, scale)

    Entity* parent;
    std::vector<Entity*> children;

    virtual void render(Camera* camera);
    virtual void update(float elapsed_time);

    void addChild(Entity* child);
    void removeChild(Entity* child);
    Matrix44 getGlobalMatrix();  // Concatenates parent transforms
};
```

### EntityMesh (Renderable Entity)
```cpp
class EntityMesh : public Entity {
public:
    Mesh* mesh = nullptr;       // Shape (vertices, normals, uvs)
    Material* material = nullptr; // Appearance (texture, shader, color)

    void render(Camera* camera);
    void update(float elapsed_time);
};
```

### Common Entity Types
| Type | Purpose |
|------|---------|
| `Entity` | Base class, non-visual (waypoints, triggers) |
| `EntityMesh` | Visual objects with mesh + material |
| `EntityLight` | Light sources |
| `EntityTrigger` | Event triggers when crossed |
| `EntitySound` | Sound emitters |

---

## Scene Graph (Tree Structure)

### Composite Pattern
- Entity can contain child entities
- Transforms are relative to parent
- Final transform = `model * parent->getGlobalMatrix()`

### Key Behaviors
```cpp
void Entity::render(Camera* camera) {
    // Render self if has visual
    // ...

    // Propagate to children
    for(int i = 0; i < children.size(); i++)
        children[i]->render(camera);
}

Matrix44 Entity::getGlobalMatrix() {
    if (parent)
        return model * parent->getGlobalMatrix();
    return model;
}
```

### World/Scene Class
- Container for all entities (Singleton pattern)
- Has `root` entity as scene tree root
- Utility methods: `getEntitiesInsideArea()`, etc.

```cpp
world.root.appendChild(my_entity);
```

### Important Considerations
- **Render order**: Tree hierarchy determines order (problem for transparency)
- **Destruction**: Must remove from parent's children, destroy child entities too
- **Deferred destruction**: Queue for deletion, process at end of frame

---

## Casting & Memory Management

### Upcasting (Safe)
```cpp
Entity* base_entity = (Entity*) airplane;  // Airplane* → Entity*
```

### Downcasting (Dangerous)
```cpp
Airplane* airplane = (Airplane*) base_entity;  // Could crash!

// Safe check:
if (dynamic_cast<Airplane*>(myEntity) != nullptr) {
    // myEntity is actually an Airplane
}
```

### Deferred Destruction Pattern
```cpp
static std::vector<Entity*> s_to_destroy;

void Entity::destroy() {
    s_to_destroy.push_back(this);
    for(int i = 0; i < children.size(); i++)
        children[i]->destroy();
}

// Per frame (end of update):
for(int i = 0; i < s_to_destroy.size(); i++)
    delete s_to_destroy[i];
s_to_destroy.clear();
```

---

## DOP Techniques

### Array of Structs (AoS) vs Struct of Arrays (SoA)

**AoS (OOP style):**
```cpp
struct Airplane {
    char type;
    Matrix44 model;
    float speed;
};
Airplane airplanes[100];  // Memory: [type,model,speed][type,model,speed]...
```

**SoA (DOP style):**
```cpp
struct Airplanes {
    char type[100];
    Matrix44 model[100];
    float speed[100];
};
Airplanes airplanes;  // Memory: [types...][models...][speeds...]
```
SoA is cache-friendly - accessing all models hits contiguous memory.

### Instance Pooling
Pre-allocate fixed array, track active count:
```cpp
Airplane airplanes[100];
int active_planes = 0;
```

### DOP Batch Rendering
```cpp
void renderAirplanes() {
    shader->enable();
    shader->setUniform("u_viewproj", camera->vp);
    mesh->enableBuffers(shader);

    for(int i = 0; i < active_planes; ++i) {
        shader->setUniform("u_model", airplanes[i].model);
        mesh->drawCall(GL_TRIANGLES);
    }

    mesh->disableBuffers(shader);
    shader->disable();
}
```

### Hybrid: Entity + DOP
```cpp
class Airplane : public Entity {
public:
    static std::vector<Airplane*> planes;  // Static container
    char type;

    Airplane() { planes.push_back(this); }
    ~Airplane() { planes.erase(find(planes.begin(), planes.end(), this)); }

    static void renderAll();   // Batch render all planes
    static void updateAll(float dt);
};
```

---

## Scene Creation Workflow

### Blender Export
1. **Setup**: Disable "Copy on Duplicate" for Meshes in Blender preferences
2. **Build scene**: Import assets, position objects
3. **Export**: Run `scene_exporter.py` script in Scripting tab
4. **Output**: `myscene.scene` file + `scene/` folder with OBJ files

### .scene File Format
```
#NAME MODEL
scene/meshname/meshname.obj m00,m10,m20,m30,m01,m11,m21,m31,m02,m12,m22,m32,m03,m13,m23,m33
empty_name m00,m10,m20,m30,...
```
- First column: mesh path or empty name
- Remaining: 4x4 matrix (16 floats, column-major)

### Loading Scene
```cpp
SceneParser parser;
parser.parseScene("data/myscene.scene", root);
```

---

## PDF 3: Render Optimization

### Draw Calls

**Definition**: Request from application to GPU to render something on screen.

```cpp
// For every entity we want to render:
shader->enable();
shader->setUniform("u_model", model);
shader->setUniform("u_viewprojection", vp);
shader->setUniform("u_texture", texture);

mesh->render( GL_TRIANGLES, shader );  // Draw call

shader->disable();
```

**Draw Call Cost Breakdown:**
1. **CPU**: Gather info (visibility, transform, textures, mesh, shader)
2. **CPU**: Send request to GPU (activate buffers, state, textures, uniforms)
3. **GPU**: Organize data to start rendering
4. **GPU**: Rasterize each triangle
5. **GPU**: Execute fragment shader per pixel

### CPU vs GPU Architecture
- CPU and GPU run **in parallel**, communicate via messages through motherboard
- Each OpenGL call = message to GPU with associated cost
- GPU has a **queue of actions** executed when free
- Goal: Keep both CPU and GPU working at 100% capacity

---

## Culling Techniques

### Distance Culling
Filter objects too far from camera:
```cpp
BoundingBox box = transformBoundingBox(
    getGlobalMatrix(),
    mesh->box
);

float distance = 50.0f;
Vector3 center = box.center;

if( camera->eye.distance(center) > distance )
    return;  // Don't render
```

### Frustum Culling
Test if objects are inside camera's view cone:
```cpp
// Sphere test (faster, less accurate)
float sphere_radius = box.halfsize.length();
if(camera->testSphereInFrustum(center, sphere_radius) != CLIP_INSIDE)
    return;

// Box test (slower, more accurate)
if(camera->testBoxInFrustum(center, box.halfsize) != CLIP_INSIDE)
    return;
```

---

## Pre-computing Data

Store frequently accessed data to avoid recalculation:
```cpp
class Entity {
public:
    Matrix44 model;
    Matrix44 global_model;  // Pre-computed global transform
    BoundingBox aabb;       // Pre-computed bounding box
};
```

### VRAM (Video RAM)
- GPU has dedicated RAM for rendering data
- Upload buffers once (textures, geometry), reuse in draw calls
- Avoid sending same mesh data every frame

---

## Reducing Draw Calls

### Group Rendering
Render objects sharing properties (same mesh, material) together:
```cpp
// Enable shader and buffers ONCE
shader->enable();
shader->setUniform("u_viewprojection", vp);
shader->setUniform("u_texture", texture);
mesh->enableBuffers( shader );

// Only upload what changes per instance
for(int i = 0; i < trees.size(); ++i) {
    shader->setUniform("u_model", trees[i] );
    mesh->drawCall( GL_TRIANGLES );
}

mesh->disableBuffers();
shader->disable();
```

### State Change Costs (NVIDIA data)
| Operation | Changes/sec |
|-----------|-------------|
| Render Target | ~60K/s |
| Program (Shader) | ~300K/s |
| Texture Bindings | ~1.5M/s |
| Uniform Updates | ~10M/s |

---

## Instancing

Render N identical meshes with **single draw call**, varying only transform:

```cpp
std::vector<Matrix44> trees;

shader->enable();
shader->setUniform("u_viewprojection", vp);
shader->setUniform("u_texture", texture);

// Single draw call for ALL trees
mesh->renderInstanced( GL_TRIANGLES, trees.data(), trees.size());

shader->disable();
```

**Important**: Model matrix must be an **attribute** (not uniform):
```glsl
// instanced.vs
attribute vec3 a_vertex;
attribute vec3 a_normal;
attribute vec2 a_uv;
attribute mat4 u_model;  // Per-instance attribute
```

**When useful**: Large instance counts or slow CPU.

---

## Level of Detail (LODs)

### Mesh LODs
Use simplified meshes for distant objects:
```cpp
struct sMeshLOD {
    Mesh* mesh = nullptr;
    float distance = 0.0f;
};

Mesh* lod = mesh;
float distance = camera->eye.distance( position );

// Select LOD based on distance (sorted high to low)
for (int i = 0; i < lods.size(); ++i) {
    if (distance > lods[i].distance) {
        lod = lods[i].mesh;
        break;
    }
}

lod->render( GL_TRIANGLES );
```

**Creating LODs in Blender**: Use "Decimate" modifier, lower ratio to reduce triangles.

### Shader LODs
Use simpler shaders for distant objects (remove fog, light rays, post-processing).
- Only useful for very complex shaders or mobile devices

---

## Overdraw

**Problem**: Fragment shader runs for pixels later occluded by closer geometry.

### Solutions

1. **Render near to far**: Z-Buffer discards occluded pixels before fragment shader runs

2. **Z Prepass**:
   - First pass: Render scene with simple shader to fill Z-Buffer
   - Second pass: Render with full shader, Z-Test set to `GL_EQUAL`
   - Pixels only painted once (exact Z match required)

---

## Optimization Summary

| Technique | Purpose |
|-----------|---------|
| **Culling** | Avoid rendering off-screen objects |
| **Group rendering** | Minimize pipeline state changes |
| **Instancing** | Single draw call for identical meshes |
| **LODs** | Simpler meshes/shaders for distant objects |
| **Near-to-far rendering** | Reduce overdraw |

---

## PDF 4: World Rendering

### Interior vs Outdoor Scenes

| Type | Focus | Challenges |
|------|-------|------------|
| **Interiors** | Object detail, realistic lighting | Many objects, occlusions, indirect light |
| **Outdoors** | Terrain, viewing distance, sky/weather | Large scale, seamless loading |

---

## Interior Scenes

### Map Approaches

**Cells Grid**: Apply 2D game map concepts - render mesh per tile (or textured cubes)

**Sectors** (90s style - Doom, Duke Nukem 3D):
- Connected 2D polygons rendered as 3D
- Simplifies internal calculations

**Modern 3D Editors**: Maya, Blender, Unity, Unreal
- Export large mesh + scene file with object positions
- Many engines have custom tools/formats

### Building by Pieces
- Use modular mesh packs (blocks) to assemble larger assets
- Same blocks repeated = grouped for rendering optimization

### Level Edition Options
1. **Use existing 3D editor** + importer/plugin to export to engine format
2. **Game as editor**: Debug modes to modify and save map changes

---

## Interior Visibility & Occlusions

**Problem**: Many objects in frustum but not visible (behind walls, other floors)

### Solutions

1. **Pre-calculated visibility**: Data structure reporting visible objects per cell (static only)

2. **Portal-based rooms**: Divide map into regions, render current + adjacent rooms
   - Portal = window joining two regions

3. **Occlusion Queries** (most popular):
   - Query GPU: "how many pixels if I render this bounding box?"
   - If ≥1 pixel visible → render objects inside

---

## Interior Lighting

### Lightmaps
- Pre-calculated textures with per-pixel light information
- Only suitable for **static objects**
- Don't solve light equation correctly (depends on observer position)

### Global Illumination
- Light bounces (indirect light) + reflections
- Difficult to solve efficiently on GPU
- Many solutions have artifacts or consume too many resources

---

## Outdoor Scenes

### Height Maps
Images where each pixel = height of terrain point.

```cpp
// Flat mesh + elevate vertices using height map
// Divide map into sectors, load as player moves
```

**Advantages:**
- Easy to create (image editors, World Machine, Terragen)
- Easy to render (generate grid, raise by pixel height)
- Efficient storage (no X,Z needed, easy compression)

**Disadvantages:**
- No overlapping terrain (caves, arches, cliffs)
- Uniform mesh wastes triangles in flat areas
- Large slopes have texture distortion
- Sector edges need "stitching" to avoid discontinuities

### Rendering Planets
Same height map technique, but move vertices away from sphere center (not vertically).

---

## Terrain Texturing

Generate color in shader using multiple tileable textures:
- Send textures (sand, vegetation, rock)
- Interpolate based on vertex height
- Use mask images for creative control
- Normal maps for surface detail

---

## Sky Rendering

### Skybox Setup
```cpp
// Move skybox with camera
skybox->model.setTranslation(camera->eye);

glDisable( GL_DEPTH_TEST );
drawSky();
glEnable( GL_DEPTH_TEST );
```

- Use sphere or cube mesh with face culling
- Render first, disable depth test

### Sky Texture Types

| Type | Description |
|------|-------------|
| **Panoramic** | Map sphere using X,Y as Long,Lat |
| **Cubemap** | 6 flat images for cube faces (`GL_TEXTURE_CUBEMAP`) |

### Loading Cubemaps
```cpp
Texture* cubemap = new Texture();
cubemap->loadCubemap("landscape", {
    "data/textures/skybox/px.png",  // +X
    "data/textures/skybox/nx.png",  // -X
    "data/textures/skybox/ny.png",  // -Y
    "data/textures/skybox/py.png",  // +Y
    "data/textures/skybox/pz.png",  // +Z
    "data/textures/skybox/nz.png"   // -Z
});
```

**Note**: Face order matters when loading cubemaps.

---

## PDF 5: Player Controller

### Terminology

| Term | Definition |
|------|------------|
| **Game Controller** | Physical input device (gamepad, keyboard, mouse, VR controller) |
| **Player Controller** | Software that interprets input and turns it into in-game actions |

### Player + Camera Relationship
- Controllers paired with specific camera systems
- **FPS**: Camera = player's eyes
- **Third Person**: Camera follows player
- Both camera (`lookAt`) and controller (model matrix) must be updated together

---

## First Person Controller

### Characteristics
- Camera = player's eyes
- Mouse controls rotation (yaw/pitch)
- WASD movement
- Immersion and precision
- Limited spatial awareness

### Camera Update (Yaw/Pitch)
```cpp
// Update based on mouse delta
yaw -= Input::mouse_delta.x * dt * speed;
pitch -= Input::mouse_delta.y * dt * speed;

// Restrict pitch to avoid flipping
float limitAngle = M_PI * 0.4;
pitch = clamp(pitch, -limitAngle, limitAngle);

// Create rotation matrix
Matrix44 mYaw;
mYaw.setRotation(yaw, Vector3(0, 1, 0));
Matrix44 mPitch;
mPitch.setRotation(pitch, Vector3(-1, 0, 0));
Matrix44 mRotation = mPitch * mYaw;

// Extract front direction
Vector3 front = mRotation.frontVector().normalize();

// Update camera
Vector3 eye = playerPosition;
Vector3 center = playerPosition + front;
camera->lookAt(eye, center, up);
```

---

## Third Person Controller

### Characteristics
- Camera follows from behind/over shoulder
- Independent camera rotation allowed
- Character is visible and important
- Better spatial awareness
- More complex camera management

### Camera Difference from FPS
```cpp
// Eye: orbit distance behind player (reverse of front)
Vector3 eye = playerPosition - front * orbitDistance;

// Center: player position
Vector3 center = playerPosition;

camera->lookAt(eye, center, up);
```

---

## Movement (Both FPS & Third Person)

```cpp
// 2D movement using yaw only
Matrix44 mYaw;
mYaw.setRotation(yaw, Vector3(0, 1, 0));

// Extract direction vectors
Vector3 front = mRotation.frontVector();
Vector3 right = -mRotation.rightVector();

// Build movement direction from input
Vector3 move_dir;
if(Input::isKeyPressed(SDL_SCANCODE_W))
    move_dir += front;
if(Input::isKeyPressed(SDL_SCANCODE_D))
    move_dir += right;

// Normalize and apply speed
move_dir.normalize();
position += move_dir * speed * dt;
```

---

## Other Controller Types

### Top-Down Controller
- Fixed overhead camera
- Movement types:
  - **Screen**: Move relative to screen direction (LoL)
  - **World**: Move relative to world axis (Diablo)
- Click-to-move or direct 2D input

### Point and Click Controller
- Mouse clicks on destinations/objects
- Movement + interaction combined
- Simple, good for indirect control
- Less responsive for action games

### Vehicle Controllers
- Cars, airplanes, skates, etc.
- Custom camera systems
- Physics integration required

---

## Player Rotation

### Basic Rotation
```cpp
model.rotate(camera->yaw, Vector3(0,1,0));
```

### Smooth Rotation (Interpolated)
```cpp
// Only update when moving
// Interpolate for smooth turn animation
stored_yaw = lerpAngleRad(stored_yaw, new_yaw, lerp_factor);
model.rotate(stored_yaw, Vector3(0,1,0));
```

---

## Airplane Controller

```cpp
// Always move forward
position += model.frontVector() * speed * dt;

// Rotate based on input
model.rotate(roll_input, model.frontVector());
model.rotate(pitch_input, model.rightVector());

// Camera follows with rotated UP vector
Vector3 rotatedUp = model.rotateVector(Vector3(0,1,0));

// Smooth camera follow with interpolation
new_eye = lerp(old_eye, new_eye, explerpFactor(decay, dt));
camera->lookAt(new_eye, center, rotatedUp);
```

**Tip**: Use `explerpFactor()` for frame-rate independent interpolation.

---

## Key Takeaways

- Game controller = physical device
- Player controller = software bridge between input and game world
- Controller choice defines player experience and level design constraints
- **Always design controller + camera together**

---

## PDF 6: 3D Collisions (Coldet)

### Introduction

Two engine layers needed for player movement:
1. **Collision Detection**: Determine if geometric entities intersect
2. **Physics**: Motion after collision (forces, material properties) - separate process

**Important**: Collisions ≠ Physics. Collision detection only checks overlap.

---

## Collision Primitives

### Simple Primitives (Fast)
| Primitive | Definition | Use Case |
|-----------|------------|----------|
| **Ray** | Origin point + direction | Objects between two points |
| **Sphere** | Center + radius | Distance-based checks |
| **Bounding Box** | Center + half-size | Low-cost mesh wrapping |

### Expensive Shapes
| Shape | Description |
|-------|-------------|
| **Mesh** | Full vertices/faces (millions of triangles) |
| **Convex Hull** | Simplified convex mesh containing all points |

### Primitive Complexity Tradeoff
```
← Faster Tests                    Better Bound →
Sphere → AABB → OBB → Convex Hull → Full Mesh
```

---

## Ray Collisions

### Common Use Cases
- **Movement**: Can object move in direction?
- **Picking**: Which object did user click?
- **Visibility**: Line of sight between objects

### Intersection Point
Two modes with different costs:
1. **Any hit**: Stop at first collision (faster)
2. **Closest hit**: Find all intersections, return nearest (required for explosions, decals)

---

## Bounding Boxes

### Types
- **AABB** (Axis-Aligned): Aligned to world coordinate axes
- **OBB** (Oriented): Aligned to object's local axes

### AABB from Rotated Object
Transform 8 corners by model matrix, recompute box from transformed points.
- Slightly larger than original
- Simplifies collision math

### Storage Format
```cpp
struct BoundingBox {
    Vector3 center;    // (MAX - MIN) / 2
    Vector3 halfsize;  // From center to corner (MAX_X, MAX_Y, MAX_Z)
};
```

---

## Sphere Collision

### Creating Bounding Sphere from Mesh
```cpp
// Use bounding box center + halfsize magnitude as radius
Vector3 center = mesh->box.center;
float radius = mesh->box.halfsize.length();
```

### Sphere-to-Sphere Test
```cpp
float dist = center1.distance(center2);
bool collides = dist < (radius1 + radius2);
```
Remember: Transform center by object's model matrix!

---

## Spatial Optimization

### Problem
Testing N objects against each other = O(N²) complexity

### Spatial Containers
Store entities grouped by location:
```cpp
struct MapArea {
    BoundingBox limits;
    std::vector<Entity*> entities;
};
MapArea areas[10][10];  // Only test nearby cells
```

### Spatial Trees (QuadTree/Octree)
- Automatically subdivide when density is high
- Each branch = area containing objects
- High object count → split into child branches

---

## Mesh Collision with Octrees

Mesh stored in octree structure for fast lookup:
- Triangles grouped by spatial position
- Not optimal for rendering, but ideal for collisions
- Mesh stored twice in memory (render + collision)

### Ray-Mesh Collision Algorithm

**Setup:**
- Ray in world coordinates
- Mesh in local coordinates (with octree)
- Model matrix for world transform

**Process:**
1. Transform ray to local space (multiply by inverse model matrix)
2. Test ray vs mesh bounding box (early exit if miss)
3. Traverse octree, test only relevant nodes
4. For each triangle hit, keep closest to ray origin
5. Transform collision point back to world space

**Optimization**: Calculate matrix inverse once, test many rays.

---

## Coldet Library

### Framework Integration
```cpp
// mesh.h - Already provided
void* collision_model;
bool createCollisionModel(bool is_static);
bool testRayCollision(Matrix44 model, Vector3 ray_origin,
    Vector3 ray_direction, Vector3& collision, Vector3& normal,
    float max_ray_dist, bool in_object_space);
bool testSphereCollision(Matrix44 model, Vector3 center,
    float radius, Vector3& collision, Vector3& normal);
```

### Ray Collision Example
```cpp
Mesh* mesh = entity->mesh;
Vector3 col_point, col_normal;
float max_ray_dist = 100.f;

if(mesh->testRayCollision(
    entity->model,
    ray_origin,
    ray_dir,
    col_point,
    col_normal,
    max_ray_dist,
    false  // world space
)) {
    // Collision detected at col_point
}
```

### Sphere Collision Example
```cpp
Vector3 character_center = position + Vector3(0, 1, 0);

if(mesh->testSphereCollision(
    entity->model,
    character_center,
    1.0f,  // radius
    col_point,
    col_normal
)) {
    // Collision detected
}
```

---

## EntityCollider Class

```cpp
class EntityCollider : public EntityMesh {
    bool is_dynamic = false;
public:
    int layer = NONE;
    // Methods to manage collisions based on type
};
```

### Filter Layers (Bit Masks)
```cpp
enum {
    NONE     = 0,
    FLOOR    = 1 << 0,
    WALL     = 1 << 1,
    PLAYER   = 1 << 2,
    ENEMY    = 1 << 3,
    SCENARIO = WALL | FLOOR,  // Combined layers
    ALL      = 0xFF
};

// Usage with AND operator
if (entity->layer & SCENARIO) {
    // Entity is part of scenario
}
```

---

## Collision Data Storage

```cpp
struct sCollisionData {
    Vector3 col_point;
    Vector3 col_normal;
    float distance = 1e10f;
    bool collided = false;
    EntityCollider* collider = nullptr;
};
```

### Framework Collision Class
```cpp
// Static methods
Collision::TestEntitySphere(...);
Collision::TestSceneRay(...);
```

---

## Dynamic-Static Collision

### Movement Detection
Cast ray from previous position to current position:
```cpp
Vector3 ray_origin = prev_position;
Vector3 ray_dir = (new_position - prev_position).normalize();
float max_dist = prev_position.distance(new_position);

// Test against all static objects
```

**Critical**: Set `max_ray_dist` to frame movement distance!

---

## Player Collision Patterns

### Wall Sliding
```cpp
// Project velocity onto wall surface
Vector3 newDir = velocity.dot(collisionNormal) * collisionNormal;
velocity.x -= newDir.x;
velocity.z -= newDir.z;
velocity.y -= newDir.y;  // For slopes
```

### Ground Check (isGrounded)
```cpp
// Ray from player head to below feet
sCollisionData data;
if(Collision::TestSceneRay(position + head_offset,
    Vector3(0, -1, 0), max_ray_dist, FLOOR, data)) {
    floor_position = data.col_point;
    return true;
}
return false;
```

### Position Adjustment
```cpp
if(isGrounded(position, offset, floor_position)) {
    if(position.y < floor_position.y + offset) {
        position.y = floor_position.y;
    }
} else {
    // Apply gravity
}
```
**Tip**: Use offset to avoid flickering at surface boundary.

### Third-Person Camera Collision
Cast ray from player to camera eye position:
```cpp
// If ray hits geometry, move camera to collision point
if(Collision::TestSceneRay(player_pos, cam_dir, orbit_dist, SCENARIO, data)) {
    camera->eye = data.col_point;  // Closest intersection
}
```

---

## Debug Rendering

```cpp
#define RENDER_DEBUG

void Player::render(Camera* camera) {
    EntityMesh::render(camera);

#ifdef RENDER_DEBUG
    Shader* shader = Shader::Get("basic.vs", "flat.fs");
    Mesh* sphere = Mesh::Get("data/meshes/sphere.obj");

    Matrix44 m;
    m.setTranslation(getGlobalMatrix().getTranslation());
    m.translate(0.0f, height, 0.0f);
    m.scale(sphere_radius, sphere_radius, sphere_radius);

    shader->enable();
    shader->setUniform("u_color", Vector4(0, 1, 0, 1));
    shader->setUniform("u_model", m);
    sphere->render(GL_LINES);
    shader->disable();
#endif
}
```

---

## Best Practices

| Practice | Reason |
|----------|--------|
| **Avoid mesh-to-mesh collisions** | Extremely expensive |
| **Separate static/dynamic objects** | Avoid static-static tests |
| **Use simplest primitive possible** | Sphere before box before mesh |
| **Use heuristics first** | Bounding box test before mesh test |
| **Limit ray distance** | Avoid infinite ray tests |
| **Profile collision costs** | Know where to optimize |

---

## PDF 7: Artificial Intelligence (AI)

### Introduction

Any game with non-player agents needs an AI system. Unlike graphics (with standard algorithms), **AI is the least orthodox field** - each game implements what suits it best.

**Three Core AI Components:**
| Component | Purpose |
|-----------|---------|
| **Perception** | Collecting useful information about the world |
| **Behavior** | Determining what to do based on a goal |
| **Realization** | Performing the action until objective is completed |

---

## Perception

Information an entity needs about its world:
- Relevant elements in view
- Known places on the map
- Route to reach target
- State of the world

### Line of Sight

Two checks required:
1. **Cone of vision**: Is object aligned with agent's gaze?
2. **Occlusion**: Is any object blocking the view?

**Step 1 - Vision Cone Check:**
```cpp
// Check if angle between front vector and toTarget is within FOV
Vector3 front = entity->model.frontVector();
Vector3 toTarget = (target_pos - entity_pos).normalize();
float angle = acos(front.dot(toTarget));
bool inCone = angle < max_fov_angle;
```

**Step 2 - Occlusion Check:**
```cpp
// Cast ray from agent to target
// If ray hits scenario geometry before target = occluded
if(Collision::TestSceneRay(agent_pos, toTarget, dist_to_target, SCENARIO, data)) {
    // Target is occluded
}
```

**Note**: Simple raycast approach has flaws - players can exploit it.

---

## Behavior

Deciding **what action to perform each time** based on available information.

### Programming Methods

| Method | Description | Trade-off |
|--------|-------------|-----------|
| **State Machine** | States + transitions, check conditions to change state | Must program all transitions |
| **Behavior Tree** | Re-evaluate tree from root each time | Slower but fewer transitions to manage |

---

### State Machine (SM)

Each state defines a behavior. While in a state:
1. Execute that behavior
2. Check if conditions lead to neighboring state
3. If condition met → transition

```cpp
enum eActions { SEARCH, ATTACK, FLEE };

void AIController::update(float seconds_elapsed)
{
    if (state == SEARCH)
    {
        if (inLineOfSight())
            state = ATTACK;
        else
            searchTarget();
    }
    else if (state == ATTACK)
    {
        if (ammoAvailable())
            shoot();
        else
            state = FLEE;
    }
}
```

**Problem**: Many states = many transitions to program.

---

### Behavior Trees

- **No explicit state** - tree is re-evaluated from root each frame
- **Branch nodes**: Evaluate conditions, go to children if true
- **Leaf nodes**: Define actions to perform
- **Priority**: Leftmost branches have higher priority

**Node Types:**
| Node | Symbol | Function |
|------|--------|----------|
| Selector | ? | Try children until one succeeds |
| Sequence | → | Execute children in order, stop on failure |
| Condition | (green) | Check a condition |
| Action | (red) | Perform an action |

**Advantage**: Reduces transition management work.

---

## Realization (Actions)

Once behavior is determined, execute it:
- Some actions are simple (e.g., shoot)
- Others require breaking down (e.g., search for enemy)
- **Navigation** is most common - finding path from A to B

---

## Navigation

### Simple Approach
Orient toward target, move in that direction.
**Problem**: Agent gets stuck on obstacles.

### Autonomous Navigation
Equip agent with sensors (e.g., three rays detecting environment).
- Uses **local information** only
- Can get blocked
- Good for driving games (avoid walls, no pathfinding needed)

---

## Environment Simplification

Help agents by providing extra environment information.

| Dimension | Representation |
|-----------|----------------|
| **2D** | Grid where each cell = area + passable flag |
| **3D** | Graph where nodes = waypoints, edges = paths |

---

## Pathfinding

**Approach**: Explore paths from origin, expand path that gets closer to destination.

### Algorithms
- **A*** (most common)
- **IDA***
- **Dijkstra**
- **Jump Point Search**

### 2D Pathfinding (Pathfinders Library)

```cpp
// Map: array W*H of bytes (0=blocked, 1=walkable)
uint8* map = new uint8[W * H];
// Fill map with walkability data...

int output[100];  // Stores path (max 100 steps)

int path_steps = AStarFindPathNoTieDiag(
    start.x, start.y,    // origin (integers)
    target.x, target.y,  // target (integers)
    map,                 // pointer to map data
    W, H,                // map dimensions
    output,              // path storage
    100);                // max steps

if (path_steps != -1) {
    for (int i = 0; i < path_steps; ++i) {
        int x = output[i] % W;
        int y = floor(output[i] / W);
        // Use x, y coordinates
    }
}
```

**Note**: Can use for 3D if scene has no height variation.

---

## 3D Pathfinding with Waypoints

3D grids consume too many resources → use **waypoint graphs**.

```cpp
class Waypoint {
public:
    Vector3 position;
    std::vector<Waypoint*> neighbours;
};

class Graph {
public:
    std::vector<Waypoint*> waypoints;
    std::vector<Waypoint*> findPath(Waypoint* a, Waypoint* b);
};
```

### Using PathFinder Library (Sahnvour/PathFinder)

```cpp
// Create waypoint nodes (WayPoint inherits from DijkstraNode)
std::vector<WayPoint> wp_nodes;
wp_nodes.resize(points.size());

// Link nodes (example: linear path)
for (int i = 0; i < points.size() - 1; ++i) {
    wp_nodes[i].position = points[i];
    wp_nodes[i + 1].position = points[i + 1];
    wp_nodes[i].addLink(
        &wp_nodes[i + 1],
        (points[i] - points[i + 1]).length()
    );
}

// Find path using Dijkstra
PathFinder<WayPoint> p;
std::vector<WayPoint*> solution;
p.setStart(wp_nodes[0]);
p.setGoal(wp_nodes[wp_nodes.size() - 1]);
bool r = p.findPath<Dijkstra>(solution);
```

### Following the Path

```cpp
if (path.size()) {
    Vector3 origin = model.getTranslation();
    Vector3 target = path[waypoint_index].position;

    // Orient before moving!
    lookAtTarget(target, seconds_elapsed);
    model.translate(0.f, 0.f, seconds_elapsed);

    float distance_to_target = (target - origin).length();
    if (distance_to_target < 0.1f) {
        // Reached waypoint, move to next
        waypoint_index++;
    }
}
```

**Note**: May need player controller collision logic for AI too!

---

## Navigation Meshes (NavMesh)

**Problem with waypoints**: Agents move along lines (like ants).

**NavMesh**: Each triangle = walkable area
- Agent moves within triangle area
- Can move to adjacent triangles sharing an edge
- Convert mesh to graph → apply pathfinding

**Creation**: Tools or procedural generation.

---

## Entity Orientation

### 2D Orientation

```cpp
Vector2 toTarget = normalize(B - A);
float angle_in_rad = atan2(toTarget.y, toTarget.x);
```

**Important**: Interpolating angles is not trivial!
- Shortest path from 10° to 350° should NOT go through 180°
- Use smooth yaw changes over time

### 3D Orientation

**Step 1 - Calculate angle:**
```cpp
Vector3 front = entity->model.frontVector().normalize();
Vector3 toTarget = (target - entity_pos).normalize();

// Clamp to avoid NaN from acos
float dot = clamp(front.dot(toTarget), -1.0f, 1.0f);
float angle_in_rad = acos(dot);
```

**Step 2 - Calculate rotation axis:**
```cpp
// Skip if angle ≈ 0 (already looking at target)
if (angle_in_rad > 0.001f) {
    Vector3 axis = front.cross(toTarget).normalize();

    // Convert axis to local space
    Matrix44 inv = entity->getGlobalMatrix();
    inv.inverse();
    axis = inv.rotateVector(axis);

    // Apply rotation (scale by delta_time for smooth turn)
    entity->model.rotate(angle_in_rad * delta_time, axis);
}
```

**Warning**: `acos` returns NaN if value outside [-1, +1] range!

---

## Best Practices

| Practice | Reason |
|----------|--------|
| **Goal is to fool the player** | Doesn't need actual intelligence |
| **Hard-code simple behaviors** | Complex data structures often unnecessary |
| **Use 2D pathfinding when possible** | If no height variation, avoid 3D pathfinding |
| **Give AI more info than player** | Compensates for limited intelligence |

---

## PDF 8: Assets for Games

### Introduction

Creating 3D models requires a **3D artist profile** - someone who knows tools and processes. Without this profile, a 3D game has limited creativity.

**Key Insight**: It's possible to make a 3D game using 2D assets!

---

## 3D Sprites (Billboarding)

Use 2D assets in 3D games by rendering **planes aligned with the camera**.

| Use Case | Benefit |
|----------|---------|
| Characters | Most complex to create in 3D |
| Objects | Quick to produce |
| Particles | Natural fit for billboards |

---

## Third-Party Assets

### Free Repositories

| Source | Description |
|--------|-------------|
| **Kenney** | High-quality free asset packs |
| **Quaternius** | Low-poly 3D models |
| **Everything assets** | Vertex-painted models |
| **OpenGameArt** | Community uploads (quality varies) |

### Asset Stores

| Platform | Notes |
|----------|-------|
| **Unity Asset Store** | Quality varies with price |
| **SketchFab** | Preview and download, high-quality usually paid |

### Hire an Artist

Communities for commissions:
- polycount.com
- artstation.com

Prices vary by quality, but allows **custom resources matching game criteria**.

### Asset Ripping (Placeholders Only)

Tools extract meshes/textures from existing games (e.g., Ninja Ripper).
**Warning**: Cannot be used in final game - only for development placeholders.

---

## Build Your Own Assets

### Programmer Art

Programmers create **placeholder** assets to test features without waiting for artists.

### Editor Types

| Level | Tools | Use Case |
|-------|-------|----------|
| **Advanced** | Blender, Maya, 3Ds Max | Full professional modeling |
| **Intermediate** | Sketchup, ZBrush | Architecture, characters |
| **Simplified** | Dreams | Lower barrier of entry |

### Blender

- **Open source** professional 3D modelling tool
- Huge community = easy to learn
- **Important**: Even programmers should know basic usage for:
  - Creating placeholders
  - Helping artists export in correct format

### Asset Forge

- Paid editor ($20) - build models by **combining existing parts**
- **Drawback**: Produces meshes with unnecessary internal triangles
- **Free alternative**: prop3D (online version)

### MagicaVoxel

- **Free** voxel-based 3D editor
- Create models like painting in 3D
- Voxels = 3D pixels
- **Trade-off**: Many triangles (no textures), but stylish aesthetic

### Photogrammetry

Generate 3D models from **real objects**:
1. Take many photos from different angles
2. Use reconstruction software to generate mesh

| Advantage | Disadvantage |
|-----------|--------------|
| Very realistic | Very high polygon count |
| Real-world objects | Requires cleanup/decimation |

---

## Best Practices

| Practice | Reason |
|----------|--------|
| **Know at least one 3D tool** | Modify meshes whenever needed |
| **Use billboards for characters** | Avoid complex 3D modeling |
| **Start with free repositories** | Test concepts before investing |
| **Create placeholders early** | Don't wait for final art |
| **Match asset style** | Consistency > individual quality |

---

## PDF 9: Physics Simulation

### Introduction

Beyond collision detection, sometimes we need to compute the **reaction that collisions produce** - how they affect position and velocity.

**Key Insight**: We can often develop a whole game **without** a physics engine, which speeds up development.

---

## 2D vs 3D Physics

| Type | When to Use |
|------|-------------|
| **2D Physics** | Simpler, faster - use if game allows |
| **3D Physics** | Only when truly needed |

**Note**: Using a 2D engine doesn't mean game is 2D - just that physics only uses two dimensions.

---

## Physics Engine Phases

Every physics engine has three phases:

| Phase | Description |
|-------|-------------|
| **Newtonian Physics** | Compute position after delta time (velocity, acceleration, mass) |
| **Collision Detection** | Check if objects overlap, find exact collision point |
| **Collision Resolution** | Separate objects, transfer forces, start again |

---

## Ticks and Delta Time

Physics uses a `tick` or `update` function with delta time.

**Problem**: Large delta time = inaccurate results (objects pass through each other)

**Solution**: Multiple iterations with smaller delta time:
```cpp
// For small fast objects
for (int i = 0; i < 10; ++i)
    world.tick(delta_time / 10.f);
```

---

## Simple Physics Implementation

### Particle Data Structure
```cpp
struct sPhysic {
    Vector3 pos;
    Vector3 vel;
    Vector3 acc;
    float mass;
    float radius;
};
```

### Newtonian Tick
```cpp
void tick(sPhysics& p, float dt)
{
    // Move particle
    p.pos = p.pos + p.vel * dt;

    // Increase momentum
    p.vel = p.vel + p.acc / p.mass;

    // Reset forces
    p.acc.set(0, 0, 0);

    // Friction (prevent energy explosion)
    p.vel = p.vel * 0.99;
}
```

### Collision Detection
```cpp
// Floor collision
if (p.pos.y < p.radius) {
    // Resolve...
}

// Sphere-to-sphere
float dist = distance(p.pos, p2.pos);
float min_dist = p.radius + p2.radius;
if (dist < min_dist) {
    // Resolve...
}
```

### Collision Resolution
```cpp
Vector3 up(0.f, 1.f, 0.f);

// Floor collision
if (p.pos.y < p.radius) {
    p.pos.y = p.radius;  // Separate
    p.vel = reflect(p.vel * 0.9f, up);  // Reflect + energy loss
}

// Sphere-sphere collision
if (dist < min_dist) {
    Vector3 norm = normalize(p2.pos - p.pos);
    float dist_diff = min_dist - dist;

    // Separate both objects
    p.pos = p.pos - norm * dist_diff * 0.5f;
    p2.pos = p2.pos + norm * dist_diff * 0.5f;

    // Reflect velocities
    p.vel = reflect(p.vel, norm) * 0.9f;
    p2.vel = reflect(p2.vel, norm) * 0.9f;
}
```

---

## Third-Party Physics Engines

### World Structure

All objects stored in **World** data structure.

### Object Types

| Type | Description | Also Called |
|------|-------------|-------------|
| **Dynamic** | Move and collide | Rigid Bodies |
| **Static** | Don't move, can be collided | Colliders |

**Performance**: More affected by Rigid Bodies than Colliders.

### Morphology (Shapes)

Shape doesn't need to match visual 100%:
- **Simple shapes** (cubes, spheres, cylinders) = faster collisions
- Many engines don't support triangle meshes
- **Tradeoff**: Too different from visual = player notices

### Recovering Transform

After physics update:
1. Read position and orientation from rigid body
2. Copy to game object's transform

**Important**: Ensure coordinate system and units match (radians vs degrees, vertical axis, etc.)

### Applying Forces

Cannot directly modify position - must **apply impulses**:
- Impulses = small accelerations altering velocity
- Specify: which object, which point on object
- **Center impulse**: Only linear velocity changes
- **Off-center impulse**: Also applies angular force (torque)

---

## 3D Physics Engines (C++)

| Engine | Notes |
|--------|-------|
| **Bullet** | Very complete, difficult to integrate |
| **PhysX** | Very complete and powerful |
| **ReactPhysics** | Simpler alternative |
| **Newton Dynamics** | Another option |

---

## Best Practices

| Practice | Reason |
|----------|--------|
| **Not all games need physics** | Simple calculations often sufficient |
| **Use 2D physics for 3D games** | When height doesn't matter |
| **Build a simple engine first** | Educational, helps understand libraries |
| **Simplify collision shapes** | Performance over accuracy |
| **Add friction factor** | Prevents energy explosion |
| **Multiple tick iterations** | For small fast objects |

---

## PDF 10: GUI and HUDs

### Introduction

Beyond rendering the 3D scene, games need **2D elements**:
- **HUD** (Head-Up Display): In-game information overlay (health, ammo, minimap)
- **GUI**: Menus between game sections

**Key Insight**: Rendering 2D elements uses the same GPU pipeline (Meshes, Shaders, Textures) - only the **projection differs**.

---

## Rendering in 2D

### Clip-Space Coordinates

GPUs expect triangle coordinates in **clip-space [-1, 1]**:
```
     -1,1 ─────────── 1,1
       │               │
       │     0,0       │
       │               │
     -1,-1 ─────────── 1,-1
```

For 2D rendering, skip all transformation matrices - pass vertices directly:
```glsl
// Vertex shader for 2D
void main()
{
    v_uv = a_uv;
    // Don't apply transformations!
    gl_Position = vec4(a_vertex, 1.0);
}
```

---

## Building 2D Quads

### Manual Quad Construction
```cpp
Mesh quad;

// First triangle (CCW winding)
quad.vertices.push_back( Vector3(-1, 1, 0) );
quad.uvs.push_back( Vector2(0, 1) );
quad.vertices.push_back( Vector3(-1, -1, 0) );
quad.uvs.push_back( Vector2(0, 0) );
quad.vertices.push_back( Vector3(1, -1, 0) );
quad.uvs.push_back( Vector2(1, 0) );

// Second triangle
quad.vertices.push_back( Vector3(-1, 1, 0) );
quad.uvs.push_back( Vector2(0, 1) );
quad.vertices.push_back( Vector3(1, -1, 0) );
quad.uvs.push_back( Vector2(1, 0) );
quad.vertices.push_back( Vector3(1, 1, 0) );
quad.uvs.push_back( Vector2(1, 1) );

quad.render(GL_TRIANGLES);
```

### Framework Helper
```cpp
Mesh mesh;
mesh.createQuad(quad_size.x, quad_size.y);  // Option for centered or not
```

**Important**: Vertex order matters for `GL_CULL_FACE` - determines visible side.

---

## Clip-Space vs Viewport-Space

### Clip-Space Drawbacks
- Not comfortable (prefer pixels over ratios)
- Non-square aspect ratios cause deformation

### Solutions
1. Convert coordinates when building mesh (0..W → -1..1)
2. Build in viewport-space, convert in shader

---

## Orthographic Camera for 2D

Use orthographic projection to work in **pixel coordinates**:

```cpp
Camera camera2D;
camera2D.view_matrix = Matrix44();  // Identity
camera2D.setOrthographic(0, window_width, window_height, 0, -1, 1);

shader->setUniform("u_viewprojection", camera2D.viewprojection_matrix);
```

**Shader**: Multiply vertices by view-projection matrix.

---

## OpenGL Flags for 2D

```cpp
glDisable(GL_DEPTH_TEST);   // Not needed - render back-to-front
glDisable(GL_CULL_FACE);    // Show both triangle sides
glEnable(GL_BLEND);         // For transparencies
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // Linear interpolation
```

---

## HUD Rendering Process

Each frame:
1. Clear framebuffer
2. Activate 3D camera
3. Paint 3D scene (Z-Buffer + CullFace enabled)
4. **Disable Z-Buffer and Cull Face**
5. **Activate 2D orthographic camera**
6. **Render HUD quads**

---

## Texture Atlas

Store all HUD icons in **one texture** for efficiency:
- Single draw call for entire HUD
- Use UVs to select specific icon region

### UV Remapping Shader
```glsl
// Remap UVs from 0..1 to atlas region
uv.x = start.x + uv.x * (end.x - start.x);
uv.y = start.y + uv.y * (end.y - start.y);
```

---

## 3D HUD (World-Space UI)

Display 2D elements at 3D positions (health bars above enemies, markers):

### Project 3D to 2D
```cpp
// Convert 3D point to clip-space
Vector4 pos2D = viewprojection_matrix * Vector4(pos3D, 1.0);
pos2D.x /= pos2D.w;  // Perspective divide
pos2D.y /= pos2D.w;

// Framework helper
Vector4 pos2D = camera->project(pos3D, width, height);
```

**Important**: Check clip-space limits to skip rendering when off-screen.

---

## Mini-map Rendering

### Using glViewport
```cpp
// Limit rendering area
glViewport(x, y, minimap_width, minimap_height);

// Clear depth buffer (avoid main scene occlusion)
glClear(GL_DEPTH_BUFFER_BIT);
glEnable(GL_DEPTH_TEST);
```

### Rotating Mini-map with Player
```cpp
Camera cam2D;
cam2D.setPerspective(60.f, 1.f, 0.1, 100.f);

Vector3 center = player_position;
Vector3 eye = center + Vector3(0, height, 0);  // Above player

// Rotate UP vector by player yaw
Matrix44 mYaw;
mYaw.setRotation(yaw, Vector3(0.f, 1.0f, 0.0f));
Vector3 up = mYaw.rotateVector(Vector3(0.0f, 0.0f, 1.0f));

cam2D.lookAt(eye, center, up);
cam2D.enable();
// Render world...
```

**Note**: UP cannot be (0,0,1) when looking down - would match VIEW vector.

---

## GUI Creation

Same rendering as HUD, plus **click detection**.

**Tip**: Render 3D world in background during menus for visual engagement.

---

## Immediate GUI Pattern

Simplifies GUI interaction by combining render + input check:

```cpp
// Renders button AND returns true if clicked previous frame
if (UI::addButton(10, 10, 100, 100, "button.png"))
{
    // Button was pressed!
}
```

**Implementation**: Engine saves last click position, button checks if click fell inside.

---

## 3D GUI

Options:
1. Use **perspective camera** instead of orthographic (enable + clear depth buffer)
2. Render 3D scene, detect input with **ray-plane** and **ray-mesh collisions**

---

## Best Practices

| Practice | Reason |
|----------|--------|
| **Use texture atlas** | Single draw call for HUD |
| **Render HUD last** | After 3D scene, with depth test disabled |
| **Check clip bounds for 3D HUD** | Skip off-screen elements |
| **Clear depth before mini-map** | Prevent main scene occlusion |
| **Use Immediate GUI pattern** | Simplifies button interaction code |

---

## PDF 11: Audio

### Introduction

Audio is critical for games - **a game without audio feels unfinished** regardless of visual quality.

**Advantages over graphics**: Much simpler to implement (load sounds, play when needed).

---

## Audio Purposes

| Purpose | Description |
|---------|-------------|
| **Emotion** | Soundtrack makes scenes epic/dramatic (don't overuse - dissonance is annoying) |
| **Narration** | Voice telling story is more immersive than text |
| **Description** | Conveys object properties (loud gun = powerful) |
| **Feedback** | Confirms player actions received (beep on activate, error sound on invalid) |
| **Information** | Second channel when screen is overwhelmed (enemy approaching) |

---

## Technical Concepts

### Hardware Access
- OS provides pointer to memory buffer for samples
- Buffer read every X milliseconds, sent to sound card
- Our task: define how buffer is filled

### Audio Channels
- **Channels** = tracks for playing sounds (not left/right stereo!)
- One sound per channel
- Limited simultaneous sounds (~16 on older hardware)
- Get channel handle to manipulate volume, pitch, effects

### Output Ports
- Multiple ports possible (front speakers, rear, HDMI)
- Open specific port via `device_id` (-1 = system default)
- Wrong port = sound sent to disconnected output

---

## Integration Steps

**At startup:**
1. Open audio output port
2. Load sounds from disk (WAV/MP3)

**During game:**
1. Create channel
2. Play audio on channel

---

## BASS Library

Popular audio library (proprietary, free for non-commercial).

### Files Provided
- `bass.h` - Function declarations
- `bass.lib` - Compiled library for linking
- `bass.dll` / `libbass.dylib` / `libbass.so` - Runtime library

### Basic Usage
```cpp
// Initialize at startup (-1 = default device)
if (BASS_Init(-1, 44100, 0, 0, NULL) == false) {
    // Error with sound device
}

HSAMPLE hSample;      // Sample handler
HCHANNEL hChannel;    // Channel handler

// Load sample (build a manager - disk loads are slow!)
hSample = BASS_SampleLoad(
    false,              // From internal memory
    "sounds/shot.wav",  // Filepath
    0,                  // Offset
    0,                  // Length
    3,                  // Max playbacks
    0                   // Flags: BASS_SAMPLE_LOOP, BASS_SAMPLE_3D
);

if (hSample == 0) return;  // Error loading

// Get channel and play
hChannel = BASS_SampleGetChannel(hSample, false);
BASS_ChannelPlay(hChannel, true);
```

**Important**: Don't call play every frame - call ONCE!

---

## 3D Audio

Position sounds in 3D space - library calculates realistic playback based on listener position.

### Features
- Volume by distance
- Frequency filtering by orientation
- Doppler effect

### Setup
```cpp
// Load with 3D flag
hSample = BASS_SampleLoad(false, "sound.wav", 0, 0, 3, BASS_SAMPLE_3D);

// Get channel and play
hChannel = BASS_SampleGetChannel(hSample, false);
BASS_ChannelPlay(hChannel, true);

// Set sound position in world
BASS_3DVECTOR position;
BASS_ChannelSet3DPosition(hChannel, &position, NULL, NULL);

// Set listener position (call each frame)
BASS_Set3DPosition(&listener_pos, NULL, &front, &top);

// Apply changes
BASS_Apply3D();
```

**Critical**: 3D sounds must be **mono**, not stereo!

---

## Audio API Design

Design for simplicity:
```cpp
// Simple usage
Audio::Play("shot.wav");

// With options
HCHANNEL sound = Audio::Play("shot.wav", 0.5f, true);  // volume, loop
```

### Audio Class Structure
```cpp
class Audio {
public:
    HSAMPLE sample;

    Audio();   // sample = 0
    ~Audio();  // BASS_SampleFree

    HCHANNEL play(float volume);

    // Manager (like Mesh, Texture, Shader)
    static std::map<std::string, Audio*> sAudiosLoaded;

    static bool Init();
    static Audio* Get(const char* filename);
    static HCHANNEL Play(const char* filename);
    static HCHANNEL Play3D(const char* filename, Vector3 position);
    static bool Stop(HCHANNEL channel);
};
```

---

## Audio Tools

### Sound Sources
| Source | Description |
|--------|-------------|
| **Freesound.org** | Free repository (needs editing) |
| **Kenney** | High-quality free packs |
| **BBC Sounds** | Sound effects library |

### Editing
- **Audacity** - Free audio editor
  - Convert to mono (no point in stereo for game sounds)
  - Remove silences at start/end
  - Add fade in/out

### Creation
- **Bfxr.net** - Create 8-bit sounds
- **Foley** - Record real objects (breaking vegetables for gore sounds)

---

## Best Practices

| Practice | Reason |
|----------|--------|
| **Build audio manager** | Avoid loading same sound twice |
| **Use mono for 3D sounds** | Required for spatial positioning |
| **Remove silences** | Instant playback response |
| **Add fade in/out** | Avoid audio pops |
| **Don't play every frame** | Call play ONCE per sound event |
| **Use audio for feedback** | Confirms player actions |

---

## PDF 12: 3D Animations

### Introduction

Skeletal animation allows characters to move realistically by deforming a mesh based on bone transformations.

**Four Requirements for Animated Characters:**
| Component | Description |
|-----------|-------------|
| **Skinned Mesh** | Mesh with per-vertex bone IDs and weights |
| **Skeleton** | Structure containing body pose (bone hierarchy) |
| **Animation** | Keyframes defining bone transforms over time |
| **Skin Shader** | Applies bone transformations to vertices |

---

## Multiple Bones per Vertex

Each vertex can be affected by multiple bones (typically 4):
- **Skinning**: Decide which bones affect each vertex and by how much
- Process done in vertex shader with bone array and per-vertex data

### Weighted Vertex Position
```cpp
// H: Array of bone matrices (Matrix44[])
// V: Vertex position (Vector3)
// boneA, boneB: Bone IDs affecting vertex (int)
// wA, wB: Weight of each bone influence (float)

Vector4 v4 = Vector4(V, 1.0);

// Final position: sum of weighted bone-transformed positions
Vector4 pos = H[boneA] * v4 * wA +
              H[boneB] * v4 * wB;
```

---

## Animation Blending

### Problem: Sharp Transitions
Switching animations causes visible jumps due to pose disparity.

### Solution: Interpolate Transformations
Blend bone transforms between two skeletons:
- Works well for similar poses
- **Caveats**: May produce impossible poses (arms through body, unnatural movements)

### Blending Between Cycles (Walk/Run)
Synchronize cycle durations to avoid weird mixed animations:
```cpp
timeB = (timeA / durationA) * durationB;
```

---

## Masking

Blend animations on specific body regions only (e.g., walk + wave):

### Layer System
```cpp
enum BODY_LAYERS {
    BODY = 1,
    UPPER_BODY = 2,
    LOWER_BODY = 4,
    LEFT_ARM = 8,
    RIGHT_ARM = 16,
    LEFT_LEG = 32,
    RIGHT_LEG = 64,
    HIPS = 128
};

// Skip bones not in mask
if (bone.layer & masking_layers)
    continue;
```

---

## Forward and Inverse Kinematics

Modify skeleton transforms after animation is applied:

| Type | Description |
|------|-------------|
| **Forward Kinematics** | Set bone transform directly |
| **Inverse Kinematics** | Calculate bone transforms from target position/rotation |

### IK Use Cases
- Head looks toward target
- Hand reaches for object
- Foot rests on step

**Complexity**: Multi-bone IK (shoulder + elbow for hand position) is complex and must avoid impossible poses.

---

## Implementation

### Skinned Mesh Formats
- Common formats (OBJ, ASE, STL) don't support skinning
- Use: **FBX, DAE, GLTF** or custom exporter
- **Recommended**: Create custom exporter in Blender/Maya using Python

### Animation Export
- Standard format: **BVH** (difficult to parse)
- Better: Custom format with uniform resampling
- Resample all tracks so each time instant has all bone transforms

---

## Skin Shader

```glsl
attribute vec4 a_bones;
attribute vec4 a_weights;
uniform mat4 u_bones[128];

void main() {
    // Create skin matrix from weighted bones
    mat4 skin = u_bones[int(a_bones.x)] * a_weights.x +
                u_bones[int(a_bones.y)] * a_weights.y +
                u_bones[int(a_bones.z)] * a_weights.z +
                u_bones[int(a_bones.w)] * a_weights.w;

    // Apply skinning to vertex
    vec4 v = skin * vec4(a_vertex, 1.0);
    v_position = v.xyz;

    // Transform normals too!
    vec4 N = skin * vec4(a_normal, 0.0);
    v_normal = normalize(N);
}
```

---

## Animation Pipeline Steps

1. Load and parse animations from disk
2. Place each animation at the right time
3. Blend to get skeleton in right position
4. Extract final matrices from skeleton → pass to shader
5. Shader applies transformations to each vertex

---

## Rendering with Animations

```cpp
// Load animation
Animation* anim = Animation::Get("running.skanim");
anim->assignTime(time);

// Load skinned mesh
Mesh* mesh = Mesh::Get("male.mesh");

// Enable skinning shader
Shader* shader = Shader::Get("skinning.vs", "textured.fs");
shader->enable();
// Set uniforms: u_viewprojection_matrix, u_model, ...

// Render with animation skeleton
mesh->renderAnimated(GL_TRIANGLES, &anim->skeleton);
```

### renderAnimated() Process
1. Calculate global transform of each bone
2. Create vector with bones in mesh-required order
3. Upload bones to shader as uniform
4. Execute draw call

---

## Blending and Bone Manipulation

```cpp
// Load and set times for both animations
Animation* animA = Animation::Get("idle.skanim");
animA->assignTime(time);
Animation* animB = Animation::Get("walking.skanim");
animB->assignTime(time);

// Create temporary skeleton for blended pose
Skeleton blended_skeleton;

// Blend A and B with 0.5 weight
blendSkeleton(&animA->skeleton, &animB->skeleton,
              0.5, &blended_skeleton);

// Get bone matrix by name (returns reference!)
Matrix44& head = blended_skeleton.getBoneMatrix("mixamorig_Head");

// Transform it (e.g., scale head)
head.scale(2, 2, 2);

// Render with modified skeleton
mesh->renderAnimated(GL_TRIANGLES, &blended_skeleton);
```

---

## Animation Database: Mixamo

Free online service for character animation:
- Upload any mesh → Mixamo skins it automatically
- Choose from large animation repository
- Export as DAE

---

## Mixamo to SKANIM Workflow

1. Choose/upload character in Mixamo
2. Select animation (check "In Place" if needed)
3. Export as **DAE**
4. Load in **AnimDesigner** (drag file to window)
5. Export:
   - Mesh: **.MESH** format
   - Animation: **.SKANIM** format
6. WBIN/ABIN versions auto-generated by framework on load

**Important**: Can't mix animations from very different meshes!

---

## Animator Class

Manages animation blending automatically:

```cpp
// Create animator and play animation
Animator animator;
animator.playAnimation("walking.skanim");

// Add callback at specific time (0.25 seconds)
animator.addCallback("walking.skanim", [&](float t) {
    // Play footstep sound, spawn particles, etc.
}, 0.25f);

// Add callback at specific frame (frame 4)
animator.addCallback("walking.skanim", [&](float t) {
    // Do something at frame 4
}, 4);

// Callback when animation finishes
animator.setOnFinishAnimation([&](std::string animation_name) {
    if (animation_name == "attack.skanim") {
        // Return to idle after attack
    }
});
```

### Features
- Loop/once playback modes
- Configurable transition/blend time
- Frame callbacks (by time or frame number)
- Animation finish callbacks

---

## Best Practices

| Practice | Reason |
|----------|--------|
| **Use custom exporters** | Easier parsing, better control |
| **Resample animations uniformly** | Simplifies time lookup |
| **Transform normals too** | Proper lighting on deformed mesh |
| **Blend similar poses** | Avoids impossible poses |
| **Use masking for partial blends** | Walk + wave upper body |
| **Cache bone matrices** | Avoid recalculating every frame |
| **Use Mixamo for prototyping** | Quick animation library access |

---

*Document will be updated with additional PDFs*
