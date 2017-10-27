#include "dt_ASSIMP.h"
#include "Managers\Message_Manager.h"
#define GLEW_STATIC
#include "GL\glew.h"
#include "assimp\Importer.hpp"
#include "assimp\postprocess.h"
#include "assimp\scene.h"

namespace dt_ASSIMP {
	bool Initialize()
	{
		if (glewInit() != GLEW_OK) {
			MSG::Error(OTHER_ERROR, "dt_ASSIMP couldn't initialize!");
			return false;
		}
		else {
			MSG::Statement("dt_ASSIMP successfully loaded.");
			return true;
		}
	}
}
