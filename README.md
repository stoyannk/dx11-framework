*DirectX 11* -based rendering framework designed for **rapid prototyping**.

## General Notes

* The framework is entirely geared towards fast prototyping of graphics techniques and algorithms.
The current version was started at least 3-4 years ago and grew organically.
Some parts are ancient and taken from previous file collections I used before for prototypes.
* It is NOT a game engine, it is NOT a full graphics engine, it shouldn't be used in production.
* It doesn't abstract anything related to graphics to leave as much room to experimentation as possible.
* It is Windows, DirectX 11 only.

## Structure

The sole goal of the framework is to quickly prototype ideas and algorithms for real-time (usually game) rendering.
The framework is currently divided in 4 static libraries:

### AppCore
Contains a base application class that takes care of windows creation, input forwarding, message loop etc.
It's pretty minimal and graphics back-end agnostic.

### AppGrahics

Contains the classes that initialize an Application with a renderer. Currently only a Dx11 rendering app can be created.

### Rendering

Contains all the graphics stuff. Everything is tightly DX11 bound except the loaders/savers.

* The DxRenderer class that holds the DX11 device and the immediate context. It creates the default back-buffer and depth-stencil buffer. It also contains a list of all the rendering routines that will execute in turn every frame.
* DxRenderingRoutine is an abstract class that allows specifying rendering passes. Most of the prototypes I've created with the framework are in essence a bunch of inheritors from this class. The routines are registered with the DxRenderer and called in turn each frame.
* A Camera class for looking around
* Mesh and Subset classes. A mesh is a Vertex Buffer and a collection of subsets. Every subset han an Index buffer, a material and an OOBB and a AABB.
* Texture manager - a simple convenience class for loading, creating and managing textures with different formats.
* Shader manager - a class for compiling and creating shaders from files. It also contains wrappers for easier creation of constant buffers.
* Material shader manager - can inject in the shader information about the type of material that will be drawn. It inserts in the shader code "static bool" variables depending on the properties of the selected material that can be used for static branching later in the shader code. It also contains a map between a compiled shader for a specific material so that we can easily reuse them.
* ScreenQuad - simple class for drawing a full-screen quad
* FrustumCuller - culls subsets provided a view and projection matrix
* DepthCuller - an unfinished class for software occlusion culling
* SoftwareRasterizer - an unfinished and super experimental software rasterizer. I think it can currently just draw a couple of triangles.
* OBJ file loader. Supports most of the OBJ and MTL format. I almost exclusively use the Sponza mesh for testing, so everything used in it is supported.
* Raw file loader. "Raw" files are just memory dumps with vertex, index data and information about the materials and used textures
* Raw file saver - saves a raw mesh.

### Utilities

* Logging - a multi-threaded, fast, easy to use logging system with custom severities, facility information and unlimited arguments for the log message.
* Alignment - base classes and allocators for aligned types
* Assertions - custom asserts
* MathInlines - a couple of math functions to deal with a VS bug explained here.
* Random number generator
* STL allocators supporting a custom allocator
* Some smart pointers for COM interfaces (all D3D objects)

## Usage & Dependencies

The framework depends on Boost(1.55+) and requires Visual Studio 2012+.
To set-up the library you need to configure the property sheets it uses to find it's dependencies.

The Property sheets are located in the "Utilities" folder and are named
"PathProperty_template.props", "Libraries_x86_template.props", "Libraries_x64_template.props".
You must re-name them to "PathProperty.props", "Libraries_x86.props", "Libraries_x64.props" and edit them so that they point to your local Boost build.
The "PathProperty.props" is designed to set the include paths while the other two to the link libraries for x86 and x64.

Contains *glm* as a third-party dependency committed in the repo. The framework itself doesn't use it but it's widely used in some of the demos I made, so it's here.

## Future

I will continue to use the libs for my Dx11 experiments in the future so I'll update it when I need something else or find an issue.
I don't plan to abstract it enough to support OpenGL or other OSes different than Windows.

That said, I already need another framework for easy prototyping OpenGL stuff and Linux testing, so a new "OGL" version will probably be born when
I have some more time to dedicate it.

## License
I'm licensing the framework under the "New BSD License", so you can pretty much do whatever you want with it. If you happen to use something, credit is always appreciated.

Feedback welcome.