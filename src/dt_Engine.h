#pragma once
#ifndef DT_ENGINE
#define DT_ENGINE
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define DT_DESIRED_OGL_VER_MAJOR	4
#define DT_DESIRED_OGL_VER_MINOR	5
#define DT_ENGINE_VER_PATCH			to_string(BUILD_YEAR) + to_string(BUILD_MONTH) + to_string(BUILD_DAY) + to_string(BUILD_HOUR)
#define DT_ENGINE_VER_MINOR			to_string(127) // INCREMENT ON BACKWARDS COMPATIBLE CHANGES
#define DT_ENGINE_VER_MAJOR			to_string(0) // INCREMENT ON INCOMPATIBLE CHANGES
#define GLEW_STATIC

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
class DT_ENGINE_API dt_Engine
{
public:
	// Constructors
	/** Destroys the engine. */
	~dt_Engine();
	/** Zero-initialize the engine. */
	dt_Engine();
	/** Initializes the engine.
	 * @return				true if successfully initialized */
	bool initialize();
	/** Shuts down the engine and ceases all threaded activities ASAP. */
	void shutdown();
	/** Ticks the engine's overall simulation by a frame from the main thread */
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
	EnginePackage *m_package;
};

#define BUILD_YEAR					(__DATE__[9] - '0') * 10 + (__DATE__[10] - '0') 
#define BUILD_MONTH_IS_JAN			(__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_FEB			(__DATE__[0] == 'F')
#define BUILD_MONTH_IS_MAR			(__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define BUILD_MONTH_IS_APR			(__DATE__[0] == 'A' && __DATE__[1] == 'p')
#define BUILD_MONTH_IS_MAY			(__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define BUILD_MONTH_IS_JUN			(__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_JUL			(__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define BUILD_MONTH_IS_AUG			(__DATE__[0] == 'A' && __DATE__[1] == 'u')
#define BUILD_MONTH_IS_SEP			(__DATE__[0] == 'S')
#define BUILD_MONTH_IS_OCT			(__DATE__[0] == 'O')
#define BUILD_MONTH_IS_NOV			(__DATE__[0] == 'N')
#define BUILD_MONTH_IS_DEC			(__DATE__[0] == 'D')
#define BUILD_MONTH					((BUILD_MONTH_IS_JAN) ?  1 : \
									(BUILD_MONTH_IS_FEB) ?  2 : \
									(BUILD_MONTH_IS_MAR) ?  3 : \
									(BUILD_MONTH_IS_APR) ?  4 : \
									(BUILD_MONTH_IS_MAY) ?  5 : \
									(BUILD_MONTH_IS_JUN) ?  6 : \
									(BUILD_MONTH_IS_JUL) ?  7 : \
									(BUILD_MONTH_IS_AUG) ?  8 : \
									(BUILD_MONTH_IS_SEP) ?  9 : \
									(BUILD_MONTH_IS_OCT) ? 10 : \
									(BUILD_MONTH_IS_NOV) ? 11 : \
									(BUILD_MONTH_IS_DEC) ? 12 : 0)
#define BUILD_DAY					((__DATE__[4] >= '0') ? (__DATE__[4] - '0') * 10 : 0) + (__DATE__[5] - '0')
#define BUILD_HOUR					(__TIME__[0] - '0') * 10 + __TIME__[1] - '0'

/*! \mainpage Project Delta
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
 *		- Frustum
 *		- Image_Importer
 *		- Model_Importer
 *		- Transform
 */

#endif // DT_ENGINE