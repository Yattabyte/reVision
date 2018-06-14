#pragma once
#ifndef ENGINE_H
#define ENGINE_H
#define DESIRED_OGL_VER_MAJOR	4
#define DESIRED_OGL_VER_MINOR	5
#define GLEW_STATIC
constexpr char ENGINE_VERSION[]	= "0.170";

#include <map>
#include <shared_mutex>
#include <thread>
#include <vector>

using namespace std;
class EnginePackage;
class GLFWwindow;
class Camera;
class System;


/**
 * The main game engine object. Encapsulates the entire engine state.
 * The engine is responsible for storing all the system pointers for use through its life.
 **/
class Engine
{
public:
	// Constructors
	/** Destroys the engine. */
	~Engine();
	/** Zero-initialize the engine. */
	Engine();
	/** Initializes the engine, and makes this context active for the calling thread.
	 * @return	true if successfully initialized */
	bool initialize();
	/** Shuts down the engine and ceases all threaded activities ASAP. */
	void shutdown();
	/** Ticks the engine's overall simulation by a frame from the main thread. */
	void tick();
	/** Ticks the engine's overall simulation by a frame from a secondary thread. */
	void tickThreaded();
	/** Checks if the engine wants to shut down.
	 * @return	true if engine should shut down */
	bool shouldClose();
	/** Returns the main camera belonging to this engine's viewport.
	 * @return	a pointer to the main camera */
	Camera * getCamera();	
	

private:
	// Public Attributes
	bool m_Initialized;	
	float m_lastTime; 
	float m_frameAccumulator;
	int m_frameCount;
	EnginePackage *m_package;
};

/*! \mainpage Project reVision
 * 
 *  \section info_sec Information
 * 
 *  This project is very much a work in progress.
 * 
 *  \section standards_sec Standards/Conventions used
 *  - All member attributes shall be prefixed with 'm_' followed by the rest in camel case, leading with a lowercase character
 *		- vec3 m_currentPosition;
 *	- All member functions are camel cased with a leading lower case character
 *		- void createObject ( ... );
 *	- Static methods are camel cased with a leading upper case character and underscores between words '_'
 *		- static void Load_Asset ( ... );
 *  - All other private member methods or hidden implementations will be lower case and separated by underscores '_'
 *		- void calculate_position_offset( ... );
 *  - Class names are camel cased with a leading upper case character
 * 		- FooBar
 * 		- Entity
 * 		- GeometryBuffer
 *  - However, where fitting, underscores should be used to bring focus to the implementation at play.\n
 * 	  The Left most term should include the newest implementation in the hierarchy
 * 		- Armor_Item
 * 		- Point_Light_Component
 * 		- System_Interface
 *
 * \subsection namespaces Namespaces
 * - Namespaces methods are camel cased with a leading upper case character
 *		- bool FileExistsOnDisk ( ... );
 *
 *  \section dependencies_sec External Dependencies
 * 
 *  - ASSIMP - Model importer: http://assimp.sourceforge.net/
 *  - Bullet - Physics simulator: http://bulletphysics.org/wordpress/
 *  - FreeImage - Texture importer: http://freeimage.sourceforge.net/
 *  - GLEW - OpenGl extension wrangler: http://glew.sourceforge.net/
 *  - GLFW - OpenGL windowing framework: http://www.glfw.org/
 *  - GLM - OpenGL mathematics library: https://glm.g-truc.net/0.9.8/index.html
 */

/*! \page assets Assets
 * \section assets_sec Engine Assets
 * \title Engine Assets
 * This section contains all of the asset types the engine currently supports.\n
 * All assets require doing the following:
 *		- Re-implementing the base asset class
 *		- Overloading the load_asset function in the Asset_Loader namespace
 *		- Define a custom work order class tailored for use in the asset loader (re-implement base work order).
 *		<br>
 *		
 *	They include:
 *		- Asset
 *		- Asset_Collider
 *		- Asset_Config
 *		- Asset_Cubemap
 *		- Asset_Material
 *		- Asset_Model
 *		- Asset_Primitive
 *		- Asset_Shader
 *		- Asset_Shader_Pkg
 *		- Asset_Texture
 */

 /*! \page entities Entities
 * \section ent_sec	Engine Entities
 * This section contains entities and their components.\n
 * There exists only 1 entity class, as all complex entities can be created by adding unique components to them.\n
 * Entities are created by entityCreator classes, controlled by the EntityFactory.\n
 * 
 * Entities implemented so far include:
 *		- Entity (base class)
 *		- SpotLight
 *		- PointLight
 *		- Sun
 *		- Prop
 *		<br>
 *
 * Components implemented so far include:
 *		- Component (base class)
 *		- Geometry_Component (interface)
 *		- Lighting_Component (interface)
 *		- Anim_Model_Component
 *		- Light_Directional_Component
 *		- Light_Point_Component
 *		- Light_Spot_Component
 */

 /*! \page managers Managers
 * \section mgr_sec Engine Managers
 * This section contains Singleton classes.\n
 * Although singletons are frowned upon in OOP, I couldn't think of any scenario in which these \n
 * few systems would ever benefit from being instantiated more than once.\n
 * They include:
 *		- Asset_Manager
 *		- Material_Manager
 *		- Message_Manager
 */

 /*! \page systems Systems
 * \section	sys_sec	Engine Systems
 * This section details systems implemented thus far for the engine.\n
 * The System_Interface details 3 virtual methods all systems inherit: 
 *		- A safe post-context creation initialization function
 *		- A main-loop update function (with delta-time argument)
 *		- A secondary threaded update function (with delta-time argument)
 *		<br>
 *		
 *	***Why 2 update functions?***
 *	The main update function is intended to be used all the essentials, such as rendering and physics.\n
 *	These things are time sensitive, so if anything that needs frequent updating can be offloaded to a second thread, then they can be implemented in the threaded function.\n
 *  For example, visibility calculations are currently offloaded entirely to the second thread.
 *  
 *  The engine currently requires the following base systems:
 *		- System_Graphics
 *		- System_Input
 *		- System_Logic
 *		- System_Preferences
 *		- System_World
 *		<br>
 *	
 *	It is planned to allow swapping out a system of a given category with a different one that implements that system's interface.
 */

 /*! \page utilities Utilities
 * \section util_sec Engine Utilities
 * This section contains some helper tools and objects that don't fit neatly into any other category.\n
 * The rules for this section are pretty slack, however it shouldn't include 1-time use classes that \n
 * could just as easily be nested and void documenting.
 * They include:
 *		- EnginePackage
 *		- File_Reader
 *		- Image_Importer
 *		- Model_Importer
 *		- Transform
 */

#endif // ENGINE_H