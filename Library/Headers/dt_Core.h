/*
	Delta Core

	- 
*/

#pragma once
#ifndef DT_CORE
#define DT_CORE
#ifdef	DT_CORE_EXPORT
#define DT_CORE_API __declspec(dllexport)
#else
#define	DT_CORE_API __declspec(dllimport)
#endif


#define DT_ENGINE_VER_MAJOR			0
#define DT_ENGINE_VER_MIDDLE		0
#define DT_ENGINE_VER_MINOR			0
#define DT_ENGINE_VER_PATCH			0

#define DT_DESIRED_OGL_VER_MAJOR	4
#define DT_DESIRED_OGL_VER_MINOR	5

using namespace std;

class GLFWwindow;
namespace DELTA {
	DT_CORE_API bool Initialize();
	DT_CORE_API void Shutdown();
	DT_CORE_API void* GetContext();
}

#endif // DT_CORE