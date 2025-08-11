#ifndef CAGE_H
#define CAGE_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>

using namespace std;
using namespace glm;

#include "shader.h"
#include "mesh.h"

struct PointMass {
    vec3 Position;
	vec3 previousPosition;
	vec3 forces;

    float mass;

    PointMass(vec3 pos, float m) {
        Position = pos;
    	previousPosition = pos;

    	forces = vec3(0, 0, 0);
        mass = m;
    }
};

struct Spring {
    // the indices of the two point masses attached
    unsigned int v0;
    unsigned int v1;
	float restLength;

    float k;

	Spring(unsigned int v0, unsigned int v1, float k) {
		this->v0 = v0;
		this->v1 = v1;
		this->k = k;
		// Set to some constant, its a cube so all would the same distance, how far are positions from one another typically
		this->restLength = 1/3;
	}
};

class Cage {
	public:
		vector<PointMass> pts;
		vector<Spring> springs;
		vec3 pos;

		Cage() {
			pts = vector<PointMass>();
			springs = vector<Spring>();
		}

		Cage(vector<PointMass> pts, vector<Spring> springs, vec3 pos) {
			this->pts = pts;
			this->springs = springs;
			this->pos = pos;

			setupMesh();
		}


		void satisfyConstraints(float floorY) {
			for (auto &p : pts) {
				if (p.Position.y + pos.y < floorY) {
					p.Position.y = floorY - pos.y;
				}
			}
		}

		void springCorrectionForces() {
			for (auto &spring : springs) {
				PointMass *pm_a = &pts[spring.v0];
				PointMass *pm_b = &pts[spring.v1];
				vec3 magnitude = pm_a->Position - pm_b->Position;

				// Euclidean Distance between the two point mass positions
				float length = distance(pm_a->Position, pm_b->Position);
				float force = spring.k * (length - spring.restLength);

				///
				// vec3 force_dir = ;
			}
		}

		void applyForces(vec3 gravity) {
			for (auto &pointMass : pts) {
				pointMass.forces += gravity * pointMass.mass;
			}
		}

		void verletStep(float deltaTime, float damping) {
			float deltaTime2 = deltaTime * deltaTime;
			for (auto &point_mass : pts) {
				vec3 acceleration = point_mass.forces / point_mass.mass;

				vec3 temp = point_mass.Position;
				point_mass.Position = point_mass.Position + (1 - damping) * (point_mass.Position - point_mass.previousPosition) +
					(0.5f * acceleration * deltaTime2);
				point_mass.previousPosition = temp;
			}
		}

        void refreshMesh() {
            setupMesh();
        }
		
		void Draw(Shader& massShader, Shader& lineShader)
		{
			mat4 position = mat4(1.0f);
			position = translate(position, this->pos);
			massShader.use();
			massShader.setMat4("model", position);
			DrawMasses();
			lineShader.use();
			lineShader.setMat4("model", position);
			DrawSprings();
		}

		void DrawMasses() {
			// draw mesh
			glBindVertexArray(VAO);
			glPointSize(15.0f);
			glDrawArrays(GL_POINTS, 0, pts.size());
		}

		void DrawSprings() {
			// draw lines ?
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glDrawElements(GL_LINES, idx.size(), GL_UNSIGNED_INT, 0);

			glBindVertexArray(0);
		}

	private:
		unsigned int VAO, VBO, EBO;
        vector<unsigned int> idx;

		void setupMesh() {
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);

            // build idx buffer
            idx.clear();
            for (auto& s : springs) {
                idx.push_back(s.v0);
                idx.push_back(s.v1);
            }

			// bind pointmass vertex data
			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, pts.size() * sizeof(PointMass), &pts[0], GL_STATIC_DRAW);

			// bind ebo spring data 
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int),
				&idx[0], GL_STATIC_DRAW);

			// positions
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PointMass), (void*)0);
			// weight
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(PointMass), (void*)offsetof(PointMass, Position));

			glBindVertexArray(0);
		}
};

class Cube : public Cage {
	public:
		Cube(unsigned int length = 1, unsigned int npl = 1, vec3 pos = vec3(0.0f, 0.0f, 0.0f)) {
			if (npl == 0) {
				cout << "ERROR::CUBE::INVALID_NPL" << endl;
				return;
			}

			this->length = length;
			this->nodesPerLength = npl;
			this->pos = pos;

			construct();
		}

	private:
		int nodesPerLength = 1;
		int length = 1;

		void construct() {
			vector<PointMass> nodes;
			vector<Spring> springs;

			float start = -length / 2.0f;
			int nodesPerEdge = length * nodesPerLength + 1;

			for (int i = 0; i < nodesPerEdge; ++i) {
				for (int j = 0; j < nodesPerEdge; ++j) {
					for (int k = 0; k < nodesPerEdge; ++k) {
						nodes.push_back(PointMass(vec3(start + ((float)i / nodesPerLength),
										start + ((float)j / nodesPerLength),
										start + ((float)k / nodesPerLength)), 1));
						
						// connect to top
						int currIdx = i * (nodesPerEdge * nodesPerEdge) + j * nodesPerEdge + k;
						if (k + 1 != nodesPerEdge) {
							int upIdx = currIdx + 1;
							springs.push_back(Spring(currIdx, upIdx, 1));
						}
						// connect to right
						if (i + 1 != nodesPerEdge) {
							int rightIdx = currIdx + (nodesPerEdge * nodesPerEdge);
							springs.push_back(Spring(currIdx, rightIdx, 1));
						}
						// connect to forward
						if (j + 1 != nodesPerEdge) {
							int forIdx = currIdx + nodesPerEdge;
							springs.push_back(Spring(currIdx, forIdx, 1));
						}
					}
				}
			}

			//for (auto& node : nodes) {
			//	cout << "pt at " << node.Position.x << ", " << node.Position.y << ", " << node.Position.z << endl;
			//}
			//cout << endl;

			this->pts = nodes;
			this->springs = springs;
			
			refreshMesh();
		}
};

#endif