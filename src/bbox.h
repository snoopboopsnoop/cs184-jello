#ifndef BBOX_H
#define BBOX_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>

using namespace std;
using namespace glm;

#include "shader.h"
#include "mesh.h"

struct BBox {
    vec3 min;
    vec3 max;
    vec3 dim; // dimensions of box

    BBox() {
        min = vec3(0.0f, 0.0f, 0.0f);
        max = min;
    }

    BBox(vec3 min, vec3 max) {
        this->min = min;
        this->max = max;
        this->dim = max - min;
    }

};

#endif