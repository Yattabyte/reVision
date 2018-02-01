/*
	dt_Engine

	- The enapsulation of the entire engine
*/



#pragma once
#ifndef DT_ENGINE
#define DT_ENGINE
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define DT_DESIRED_OGL_VER_MAJOR	4
#define DT_DESIRED_OGL_VER_MINOR	5
#define DT_ENGINE_VER_PATCH			to_string(BUILD_YEAR) + to_string(BUILD_MONTH) + to_string(BUILD_DAY) + to_string(BUILD_HOUR)
#define DT_ENGINE_VER_MINOR			to_string(83) // INCREMENT ON BACKWARDS COMPATIBLE CHANGES
#define DT_ENGINE_VER_MAJOR			to_string(0) // INCREMENT ON INCOMPATIBLE CHANGES
#define GLEW_STATIC

#include <map>
#include <shared_mutex>
#include <thread>
#include <vector>

using namespace std;

class Engine_Package;
class Callback_Container;
class GLFWwindow;
class Camera;
class System;

class DT_ENGINE_API dt_Engine
{
public:
	// Destroy the engine
	~dt_Engine();
	// Zero-initialize the engine
	dt_Engine();
	// Initialize the engine
	bool Initialize(const vector<pair<const char*, System*>> &systems);
	// Shutdown the engine
	void Shutdown();
	// Ticks the engine's overall simulation by a frame
	void Update();
	// Check if the engine should close
	bool ShouldClose();
	// Return the camera
	Camera *GetCamera();

private:
	bool m_Initialized;	
	float m_lastTime;	
	Engine_Package *m_package;
	thread *m_UpdaterThread;
	Callback_Container *m_drawDistCallback;
	void Updater_Thread();
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
#endif // DT_ENGINE