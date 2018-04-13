#pragma once
#ifndef MODELMANAGER
#define MODELMANAGER
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define GLEW_STATIC
#define NUM_VERTEX_ATTRIBUTES 6
#define NUM_BONES_PER_VEREX 4

#include "GL\glew.h"
#include "glm\glm.hpp"
#include <shared_mutex>
#include <vector>

using namespace std;
using namespace glm;
struct GeometryInfo;

/**
* The uniform struct used by the geometry SSBO
*/


/**
 * An encapsulation of an OpenGL framebuffer.
 * Requires the implementer to manage the size of the frame buffer, and also add render targets.
 **/
class DT_ENGINE_API ModelManager
{
public:
	// (de)Constructors
	/** Destroy the model manager. */
	~ModelManager();
	/** Construct the model manager. */
	ModelManager();

	
	// Public Functions
	/** Initialzie the model manager. */
	void initialize();
	/** Submit some continuous geometric data into this buffer. 
	 * @param	data	the data to submit
	 * @param	offset	the offset of the data (gets updated) 
	 * @param	count	the count of the data (gets updated) */
	void registerGeometry(const GeometryInfo & data, GLint &offset, GLint &count);
	/** Remove some continuous geometric data from this buffer. 
	 * @param	data	the data to remove
	 * @param	offset	the offset of the data 
	 * @param	count	the count of the data */
	void unregisterGeometry(const GeometryInfo & data, const GLint &offset, const GLint &count);
	/** Update the buffer's VAO from the main thread. */
	void update();
	/** Retreive this buffer's VAO ID */
	const GLuint & getVAO() const;


private:
	// Private Functions
	/** Expand this buffer's underlying container (performs a copy operation and generates a new buffer). */
	void expandToFit(const size_t &arraySize);


	// Private Attributes
	GLuint m_vaoID;
	GLuint m_vboIDS[NUM_VERTEX_ATTRIBUTES];
	GLuint m_maxCapacity;
	GLuint m_currentSize;
	bool m_outOfDate;	
	shared_mutex m_mutex;
};

struct VertexBoneData
{
	int IDs[NUM_BONES_PER_VEREX];
	float Weights[NUM_BONES_PER_VEREX];


	~VertexBoneData();
	VertexBoneData();
	VertexBoneData(const VertexBoneData & vbd);
	void Reset();
	void AddBoneData(const int & BoneID, const float & Weight);
};
struct GeometryInfo {
	vector<vec3> vs, nm, tg, bt;
	vector<vec2> uv;
	vector<VertexBoneData> bones;
};

#endif // MODELMANAGER