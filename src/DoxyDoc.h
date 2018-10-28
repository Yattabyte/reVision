/*! \mainpage reVision
 * 
 *  \section overview_sec Overview
 * 
 *  This project is a WIP game engine, with the primary focus being on real-time 3D rendering.
 *	The engine supports OpenGL 4.5 + GLSL 460, deferred rendering, and a PBR workflow (materials and shaders).
 * 
 *  \section getting_started_sec Getting Started
 *	This project isn't in a stable state yet, but you will find the most stable version under the branch "master".
 *	The branch "active" is updated frequently and contains the most recent builds.
 *
 *	The project is designed with CMake in mind, so clone the desired branch and run CMake to generate the project solution.
 *
 *	Note: the engine requires a graphics card capable of supporting OpenGL 4.5 and GLSL 460.
 *
 *  \section libraries_sec Required Libraries
 * 
 *	This project requires the following (free) libraries:
 *	- [ASSIMP - Model Importing](http://www.assimp.org/)
 *	- [Bullet - Physics Back-end](http://bulletphysics.org/wordpress/)
 *	- [FreeImage - Image Importing](http://freeimage.sourceforge.net/)
 *	- [GLEW - OpenGL](http://glew.sourceforge.net/)
 *	- [GLFW - Windowing](http://www.glfw.org/)
 *	- [GLM - Mathematics](https://glm.g-truc.net/0.9.9/index.html)
 *
 *  \section installing_sec Installation
 *
 *  - Step 1: Download [CMake](https://cmake.org/)
 *	- Step 2: Run CMake\n
 *		
 *	Here, we want to configure it for our project.\n
 *	First, fill in the "Where is the source code" field to the main project directory, and the second "build" field to where you want to build the project.\n
 *	Second, fill in the main directories for all the required external libraries, like assimp, bullet, etc.\n
 *	Third, hit the configure button and choose the compiler you want to generate the solution for. Then hit the generate button after.\n
 *
 *	- Step 3: Build CMake\n
 *
 *	Here, we want to configure it for our project.\n
 *	The project comes as a single solution for the engine, and a separate solution for generating the optional Doxygen documentation.
 *	To avoid duplicating the engine assets for multiple builds (debug, release, x32/x64, etc) they are kept within the 'app' folder. If the executable doesn't have it set already, change it to start in the app folder.
 *
 *	\section versioning_sec	Versioning
 *
 *	We version our project in the following format: (Major #).(Minor #).(Patch Letter)
 *	For example, Version: 1.2.C, where the major version is 1, minor version is 2, and the patch version is C.

 *	\section license_sec License
 *
 *	This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
 *
 *	\section ack_sec Acknowledgments
 *
 *	Thanks to everyone who has helped me over the years!
 *
 *	Special thanks to programming members of [Facepunch](https://forum.facepunch.com/f/), the helpful community on the Stack Exchange, [OGLDev](http://ogldev.atspace.co.uk/index.html), [Learn OpenGL](https://learnopengl.com), and the many other people who run their own tutorial series and programming blogs.
 */

/*! \page assets Assets
 * \section assets_sec Engine Assets
 * Engine assets are an encapsulation of a file or set of related files that are found within the application subdirectory.\n
 * Different file types need to be handled in different ways.
 * Assets are designed to be ultra-efficient.\n
 * We sort and store them within an asset manager, and we share them whenever possible to avoid redundant disk reads.\n
 * Lastly, all assets supported thus far support multithreading, as to avoid locking the main thread.\n
 * Here's a list of all asset types the engine currently supports.\n
 *
 *	- Asset_Collider
 *	- Asset_Config
 *	- Asset_Cubemap
 *	- Asset_Image
 *	- Asset_Level
 *	- Asset_Material
 *	- Asset_Mesh
 *	- Asset_Model
 *	- Asset_Primitive
 *	- Asset_Shader
 *	- Asset_Shader_Geometry
 *	- Asset_Shader_Pkg
 *	- Asset_Texture
 *
 * \section newAssets New Assets
 * All assets have provide the following functionality - a static Create(...) function, and an overridden virtual initialize(...) function:
 * 
 *	- Shared_Asset_Shader	Asset_Shader::Create(	Engine * engine, const std::string & filename, const bool & threaded	)
 *	- void Asset_Shader::initialize(	Engine * engine, const std::string & relativePath	)
 */

 /*! \page ecs ECS
 * \section ent_sec	Engine ECS
 * The engine currently supports the following components:
 *		- Player_Component
 *		- Collider_Component
 *		- LightDirectional_Component
 *		- LightDirectionalShadow_Component
 *		- LightPoint_Component
 *		- LightPointShadow_Component
 *		- LightSpot_Component
 *		- LightSpotShadow_Component
 *		- Prop_Component
 *		- Reflector_Component
 *		- Skeleton_Component
 *		- Transform_Component
 *		<br>
 *
 * These components interact through many different systems:
 *		- LightDirectional_System (rendering)
 *		- LightPoint_System (rendering)
 *		- LightSpot_System (rendering)
 *		- PropRendering_System (rendering)
 *		- PropShadowing_System (rendering)
 *		- Reflector_System  (rendering)
 *		- PlayerMovement_System (update the player transform)
 *		- SkeletonAnimation_System (update bone transforms when animating)
 *		- TransformSync_System (update transform information for every other component type)
 */

 /*! \page managers Managers
 * \section mgr_sec Engine Managers
 * This section contains all the managers the Engine owns:
 *	- Asset_Manager
 *		- Used to create, find, and share assets (specialized representations of files from disk).
 *	- Material_Manager
 *		- Holds PBR materials, exposing them to shaders, sharing them with multiple models/surfaces.
 *	- Message_Manager
 *		- Used to print messages/error reports to the console/command line.
 *	- Model_Manager
 *		- Creates/destroys models, exposing them to shaders.
 */

 /*! \page modules Modules
  * \section mdul_sec Engine Modules
  * This section contains all the modules currently implemented:
  *	- Graphics_Module
  *		- Used to render a physically based 3D scene, renders components using systems and effects.
  *	- Physics_Module
  *		- Basic physics implementation, handles collision events and updates components.
  *	- World_Module
  *		- Handles loading the world and determining when the world has changed.
  */

 /*! \page utilities Utilities
 * \section util_sec Engine Utilities
 * This section contains some helper tools and objects used all throughout the engine.
 * The most predominant ones include:
 *		- MappedChar
 *			- Maps a value to a const char * key.
 *		- Transform
 *			- Holds a position, rotation, and scale, and calculates a transformational matrix representing that state.
 *		- IO Classes:
 *			- Image_IO
 *				- Image Importing using FreeImage.
 *			- Level_IO
 *				- Level Importing.
 *			- Mesh_IO
 *				- Model Importing using ASSIMP.
 *			- Text_IO
 *				- Plaintext importing.
 *		- OpenGL Helper Classes:
 *			- DynamicBuffer
 *				- Buffer Object Encapsulation, but can change in size.
 *			- FBO_Base
 *				- Frame Buffer Object interface.
 *			- StaticBuffer
 *				- Buffer Object Encapsulation, static in size.
 *			- StaticMappedBuffer
 *				- Buffer Object Encapsulation, static in size, mapped to local memory.
 *			- VectorBuffer
 *				- Like the std::vector class, encapsulates a dynamic buffer in a templated fashion, allowing adding/removing of elements as the way to interact with.
 */