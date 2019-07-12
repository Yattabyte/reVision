#pragma once
#ifndef SHADOWDATA_H
#define SHADOWDATA_H

#include "Modules/Graphics/Common/CameraBuffer.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Shadow/ShadowMap.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <memory>
#include <vector>


/***/
struct ShadowData {
	ShadowMap shadowFBO;
	std::vector<std::tuple<float, float*, int, CameraBuffer::CamStruct*>> shadowsToUpdate;
	float shadowSize = 1.0f, shadowSizeRCP = 1.0f;
};

#endif SHADOWDATA_H