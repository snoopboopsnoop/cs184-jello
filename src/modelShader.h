#ifndef MODEL_SHADER_H
#define MODEL_SHADER_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>

using namespace std;
using namespace glm;

#include "shader.h"
#include "mesh.h"

// holds all model's shaders for easier access
class ModelShader {
    public:
        // for drawing cages
        Shader* ptMassShader;
        Shader* springShader;

        // drawing scenes
        Shader* matShader;

        ModelShader() {
            
        }
};



#endif