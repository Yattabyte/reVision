#pragma once
#ifndef CAMERA
#define CAMERA
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\World\Visibility_Token.h"
#include "Systems\World\ECS\ECS_DEFINES.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Utilities\GL\DynamicBuffer.h"
#include "GL\glew.h"
#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include <shared_mutex>

using namespace std;

struct Camera_VisBuffers {
	#define CAM_NUM_GEOMETRY 2
	#define CAM_GEOMETRY_STATIC 0
	#define CAM_GEOMETRY_DYNAMIC 1
	DynamicBuffer m_buffer_Index[CAM_NUM_GEOMETRY], m_buffer_Culling[CAM_NUM_GEOMETRY], m_buffer_Render[CAM_NUM_GEOMETRY];
};


/**
 * An object that defines where and how a scene should be viewed.
 **/
class DT_ENGINE_API Camera
{
public:
	// Constructors
	/** Destroys the camera. */
	~Camera();
	/** Construct the camera. 
	 * @param	position		the position of the camera (defaults to 0)
	 * @param	size			the size of the screen (defaults to 1)
	 * @param	near_plane		the distance of the near plane (defaults to 0.5f)
	 * @param	far_plane		the distance of the far plane (defaults to 10.0f)
	 * @param	horizontal_FOV	the horizontal FOV (defaults to 90) */ 
	Camera(const vec3 & position = vec3(0.0F), const vec2 & size = vec2(1.0f), const float & near_plane = 0.5f, const float & far_plane = 10.0f, const float & horizontal_FOV = 90.0f);
	/** Copy constructor. */
	Camera(Camera const & other);
	/** Assignment operator. */
	void operator=(Camera const & other);


	// Public Methods
	/** Bind this camera's shader storage buffer object (SSBO).
	 * @brief				this makes the camera visible to all shaders at spot 1. */
	void Bind();
	/** Retrieve the camera's position in world space.
	 * @return				the camera's world position */
	const vec3 getPosition() const { shared_lock<shared_mutex> rguard(data_mutex);  return m_cameraBuffer.EyePosition; }
	/** Change the camera's position in space.
	 * @param	p			the new position value to use */
	void setPosition(const vec3 & p) { unique_lock<shared_mutex> wguard(data_mutex); m_cameraBuffer.EyePosition = p; };
	/** Change the camera's orientation in space.
	 * @param	q			new quaternion value to use */
	void setOrientation(const quat & q) { unique_lock<shared_mutex> wguard(data_mutex); m_orientation = q; };
	/** Change the camera's screen dimensions.
	 * @brief				this is mostly used here for aspect ratio purposes.
	 * @param	d			the new screen dimensions */
	void setDimensions(const vec2 & d) { unique_lock<shared_mutex> wguard(data_mutex); m_cameraBuffer.Dimensions = d; };
	/** Change the camera's clipping plane / near-plane 
	 * @brief				this is the closest point the camera can see.
	 * @param	n			the new near-plane value */
	void setNearPlane(const float & n) { unique_lock<shared_mutex> wguard(data_mutex); m_cameraBuffer.NearPlane = n; };
	/** Change the camera's draw distance / far-plane 
	 * @brief				this is the furthest point the camera can see.
	 * @param	f			the new far-plane value */
	void setFarPlane(const float & f) { unique_lock<shared_mutex> wguard(data_mutex); m_cameraBuffer.FarPlane = f; };
	/** Change the camera's horizontal FOV.  
	 * @param	fov			the new horizontal FOV to use */
	void setHorizontalFOV(const float & fov) { unique_lock<shared_mutex> wguard(data_mutex); m_cameraBuffer.FOV = fov; };
	/** Change the camera's gamma.
	 * @param	gamma		the new gamma value */
	void setGamma(const float & gamma) { unique_lock<shared_mutex> wguard(data_mutex); m_cameraBuffer.Gamma = gamma; };
	/** Change the perspective and viewing matrices.
	 * @param	pMatrix		new perspective matrix
	 * @param	vMatrix		new viewing matrix */
	void setMatrices(const mat4 & pMatrix, const mat4 &vMatrix);
	/** Return a copy of this camera's visibility token.
	 * @return				reference to the visibility token */
	const Visibility_Token getVisibilityToken() const { shared_lock<shared_mutex> rguard(data_mutex);  return m_vistoken; };
	/** Gives this camera a new visibility token.
	 * @param	vis_token	the new visibility token to give to this camera */
	void setVisibilityToken(const Visibility_Token & vis_token);
	/** Returns a copy of this camera's data buffer.
	 * @return				copy of the camera's data buffer */
	const Camera_Buffer getCameraBuffer() const { shared_lock<shared_mutex> rguard(data_mutex);  return m_cameraBuffer; };
	/** Change whether this camera is active or not.
	 * @param	b			bool for turning on/off the camera */
	void enableRendering(const bool & b) { render_enabled = b; };
	/** Retrieve whether or not this camera is active.
	 * @return				true if this camera has rendering enabled, false otherwise */
	bool shouldRender() const { return render_enabled; }
	/** Retrieve the mutex associated with data changes. 
	 * @return				the data mutex */
	shared_mutex & getDataMutex() const { return data_mutex; };
	/** Retrieve the visibility buffers used for rendering. 
	 * @return				this cameras' visibility buffers */
	Camera_VisBuffers & getVisibilityBuffers() { return m_visBuffers; }
	/** Recalculates matrices and sends data to the GPU. */
	void update();


private:
	// Private Attributes
	mutable shared_mutex data_mutex;
	StaticBuffer m_buffer;
	Camera_VisBuffers m_visBuffers;
	Camera_Buffer m_cameraBuffer;
	quat m_orientation;
	Visibility_Token m_vistoken;
	bool render_enabled;
};

#endif // CAMERA