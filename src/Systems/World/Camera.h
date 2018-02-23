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
#include "Utilities\Frustum.h"
#include "GL\glew.h"
#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include <shared_mutex>

using namespace std;
using namespace glm;


/** A means for sharing camera data with both the program and the GPU in one data structure. */
struct Camera_Buffer
{
	// Public Constructor
	/** Zero-Initializer constructor. */
	Camera_Buffer() {
		pMatrix = mat4(1.0f);
		vMatrix = mat4(1.0f);
		pMatrix_Inverse = mat4(1.0f);
		vMatrix_Inverse = mat4(1.0f);
		EyePosition = vec3(0.0f);
		Dimensions = vec2(1.0f);
		NearPlane = 0.01f;
		FarPlane = 1.0f;
		FOV = 1.0f;
		Gamma = 1.0f;
	}


	// Public Attributes
	mat4 pMatrix;
	mat4 vMatrix;
	mat4 pMatrix_Inverse;
	mat4 vMatrix_Inverse;
	vec3 EyePosition; float padding;
	vec2 Dimensions;
	float NearPlane;
	float FarPlane;
	float FOV;
	float Gamma;
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
	 * @param	near_plane		the distance of the near plane (defaults to 0.01f)
	 * @param	far_plane		the distance of the far plane (defaults to 10.0f)
	 * @param	horizontal_FOV	the horizontal FOV (defaults to 90) */ 
	Camera(const vec3 & position = vec3(0.0F), const vec2 & size = vec2(1.0f), const float & near_plane = 0.01f, const float & far_plane = 10.0f, const float & horizontal_FOV = 90.0f);
	/** Copy constructor. */
	Camera(Camera const & other);
	/** Assignment operator. */
	void operator=(Camera const & other);


	// Public Methods
	/** Bind this camera's shader storage buffer object (SSBO).
	 * @brief				this makes the camera visible to all shaders at spot 1. */
	void Bind();
	/** Change the camera's position in space.
	 * @param	p			the new position value to use */
	void setPosition(const vec3 & p) { lock_guard<shared_mutex> wguard(data_mutex); m_cameraBuffer.EyePosition = p; };
	/** Change the camera's orientation in space.
	 * @param	q			new quaternion value to use */
	void setOrientation(const quat & q) { lock_guard<shared_mutex> wguard(data_mutex); m_orientation = q; };
	/** Change the camera's screen dimensions.
	 * @brief				this is mostly used here for aspect ratio purposes.
	 * @param	d			the new screen dimensions */
	void setDimensions(const vec2 & d) { lock_guard<shared_mutex> wguard(data_mutex); m_cameraBuffer.Dimensions = d; };
	/** Change the camera's clipping plane / near-plane 
	 * @brief				this is the closest point the camera can see.
	 * @param	n			the new near-plane value */
	void setNearPlane(const float & n) { lock_guard<shared_mutex> wguard(data_mutex); m_cameraBuffer.NearPlane = n; };
	/** Change the camera's draw distance / far-plane 
	 * @brief				this is the furthest point the camera can see.
	 * @param	f			the new far-plane value */
	void setFarPlane(const float & f) { lock_guard<shared_mutex> wguard(data_mutex); m_cameraBuffer.FarPlane = f; };
	/** Change the camera's horizontal FOV.  
	 * @param	fov			the new horizontal FOV to use */
	void setHorizontalFOV(const float & fov) { lock_guard<shared_mutex> wguard(data_mutex); m_cameraBuffer.FOV = fov; };
	/** Change the camera's gamma.
	 * @param	gamma		the new gamma value */
	void setGamma(const float & gamma) { lock_guard<shared_mutex> wguard(data_mutex); m_cameraBuffer.Gamma = gamma; };
	/** Change the perspective and viewing matrices.
	 * @param	pMatrix		new perspective matrix
	 * @param	vMatrix		new viewing matrix */
	void setMatrices(const mat4 & pMatrix, const mat4 &vMatrix);
	/** Return a reference to this camera's visibility token.
	 * @return				reference to the visibility token */
	Visibility_Token &GetVisibilityToken() { return m_vistoken; };
	/** Returns a copy of this camera's data buffer.
	 * @return				copy of the camera's data buffer */
	Camera_Buffer getCameraBuffer() const { return m_cameraBuffer; };
	/** Change whether this camera is active or not.
	 * @param	b			bool for turning on/off the camera */
	void enableRendering(const bool & b) { render_enabled = b; };
	/** Retrieve whether or not this camera is active.
	 * @return				true if this camera has rendering enabled, false otherwise */
	bool shouldRender() const { return render_enabled; }
	/** Returns a copy of the viewing frustum.
	 * @return				copy of the viewing frustum */
	Frustum getFrustum() const { return m_frustum; };
	/** Retrieves the mutex associated with data changes. 
	 * @return				the data mutex */
	shared_mutex & getDataMutex() const { return data_mutex; };
	/** Recalculates matrices and sends data to the GPU. */
	void update();


private:
	// Private Attributes
	mutable shared_mutex data_mutex;
	GLuint ssboCameraID;
	Camera_Buffer m_cameraBuffer;
	quat m_orientation;
	Visibility_Token m_vistoken;
	Frustum m_frustum;
	bool render_enabled;
};

#endif // CAMERA