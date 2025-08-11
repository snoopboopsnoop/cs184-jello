#ifndef PHYSOBJ_H
#define PHYSOBJ_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <vector>

#include "shader.h"
#include "stb_image.h"
#include "camera.h"
#include "model.h"

struct PointMass {
	vec3 Position;
	float mass;
};

class PhysObj {
	public:
		PhysObj(string path) {

		}

		void Draw(Shader& shader) {
			model.Draw(shader);
		}

	private:
		Model model;
		vector<Vertex> pointMasses;

};

#endif