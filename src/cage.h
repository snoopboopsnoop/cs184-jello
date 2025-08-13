#ifndef CAGE_H
#define CAGE_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>
#include <random>

using namespace std;
using namespace glm;

#include "shader.h"
#include "mesh.h"

struct PointMass {
    vec3 Position;
	vec3 previousPosition;
	vec3 forces;
	vec3 previousForces;

    float mass;

    PointMass(vec3 pos, float m) {
        Position = pos;
    	previousPosition = pos;

    	forces = vec3(0, 0, 0);
    	previousForces = vec3(0, 0, 0);
        mass = m;
    }
};

enum SpringType {
	EDGE,
	SHEAR,
	SHEAR_BODY,
	BEND,
	SURFACE
};

// enum springDirection
// {
//
// };

struct Spring {
    // the indices of the two point masses attached
    unsigned int v0;
    unsigned int v1;
	float restLength;

    float k;
	float kd;

	Spring(unsigned int v0, unsigned int v1, float k, float kd, float rl) {
		this->v0 = v0;
		this->v1 = v1;
		this->k = k;
		this->kd = kd;

		// Set to some constant, its a cube so all would the same distance, how far are positions from one another typically
		this->restLength = rl;
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

	void updatePhysics(GLFWwindow* window, float dt) {
		applyForces(vec3(0.0f, -9.81f, 0.0f));
		applyUserInput(window, dt);
		springCorrectionForces(dt);
	}

	void applyUserInput(GLFWwindow* window, float dt) {
			vec3 inputForce = vec3(0.0f);
			float forceStrength = 19.81f; // Adjust this value

			// Check inputs and accumulate forces
			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
				inputForce.z -= forceStrength;
			}
			if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
				inputForce.z += forceStrength;
			}
			if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
				inputForce.x -= forceStrength;
			}
			if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
				inputForce.x += forceStrength;
			}
			if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
				inputForce.y += 9.81f  * 3;
			}

			float friction = 15.0f;
			auto start = pts.begin();
			while (start != pts.end()  - (pts.size() / 2)) {
				PointMass &pointMass = *start;
				vec3 velocity = (pointMass.Position - pointMass.previousPosition)/dt;
				auto temp = pointMass.forces;
				pointMass.forces += inputForce + (-friction * velocity * pointMass.mass);
				pointMass.forces.y = temp.y + inputForce.y;
				++start;
			}

			// while (start != pts.end() - 5) {
			// 	PointMass &pointMass = *start;
			// 	vec3 velocity = (pointMass.Position - pointMass.previousPosition)/dt;
			// 	auto temp = pointMass.forces;
			// 	pointMass.forces += -inputForce;
			// 	pointMass.forces.y = temp.y + inputForce.y;
			// 	++start;
			// }

			// for (auto &pointMass : pts) {
			// 	vec3 velocity = (pointMass.Position - pointMass.previousPosition)/dt;
			// 	float friction = 0.5f;
			// 	pointMass.forces += inputForce + (-friction * velocity * pointMass.mass);
			// }
		}

		void appendForces(vec3 force) {
			for (auto &pointMass : pts) {
				pointMass.forces = force;
			}
		}


		void satisfyConstraints(float floorY) {
			for (auto &p : pts) {
				if (p.Position.y + pos.y < floorY) {
					p.Position.y = floorY - pos.y;
					p.forces = vec3(0, -9.8f, 0.0);

				}
			}
		}


		void applyForces(vec3 gravity) {
			for (auto &pointMass : pts) {
				if (pointMass.Position.y + pos.y > 0) {
					pointMass.forces = gravity * pointMass.mass;
				}
			}
		}

		void springCorrectionForces(float deltaTime) {
			for (auto &spring : springs) {
				PointMass *pm_a = &pts[spring.v0];
				PointMass *pm_b = &pts[spring.v1];

				// Vector pointing from one point to the other
				vec3 ab = pm_a->Position - pm_b->Position;

				// Magnitude of pm_a and pm_b
				float m_ab = length(ab);

				// Compute the force as the
				// Spring damping coefficient times the difference of the magnitude of pa-pb and spring rest length
				float force_elastic = spring.k * (m_ab - spring.restLength);
				vec3 force_dir = normalize(ab);
				vec3 f_a = -force_elastic * force_dir;

				pm_a->forces += f_a;
				pm_b->forces -= f_a;

				vec3 vA = (pm_a->Position - pm_a->previousPosition) / deltaTime;
				vec3 vB = (pm_b->Position - pm_b->previousPosition) / deltaTime;

				vec3 vDiff = vA - vB;

				vec3 force_damping = -spring.kd * dot(vDiff, ab) / length(ab) * normalize(ab);
				//cout << "spring force " << force << "N" << endl;

				// float criticalDamping = 2.0f * sqrt(spring.k * (pm_a->mass + pm_b->mass) / 2.0f);
				// vec3 force_damping = -criticalDamping * relativeVelAlongSpring * force_dir;\

				pm_a->forces += force_damping;
				pm_b->forces -= force_damping;
			}
		}

		void friction(float deltaTime, float dampening_coefff) {
			for (auto &spring : springs) {
				PointMass *pm_a = &pts[spring.v0];
				PointMass *pm_b = &pts[spring.v1];

				vec3 ab = pm_a->Position - pm_b->Position;
				float m_ab = length(ab);

				if (m_ab < 1e-6f) continue;
				vec3 force_dir = ab / m_ab; // normalize

				// Calculate relative velocity
				vec3 vA = (pm_a->Position - pm_a->previousPosition) / deltaTime;
				vec3 vB = (pm_b->Position - pm_b->previousPosition) / deltaTime;
				vec3 relativeVel = vA - vB;

				float velAlongSpring = dot(relativeVel, force_dir);
				vec3 dampingForce = -dampening_coefff * velAlongSpring * force_dir;

				vec3 velPerpToSpring = relativeVel - velAlongSpring * force_dir;

				vec3 shearDamping = -dampening_coefff * 0.5f * velPerpToSpring;

				vec3 totalDamping = dampingForce + shearDamping;

				pm_a->forces += totalDamping;
				pm_b->forces -= totalDamping;
			}
		}

		void verletStep(float deltaTime, float damping) {

			for (auto &point_mass : pts) {
				vec3 accel = point_mass.forces / point_mass.mass;

				vec3 v_dt = point_mass.Position - point_mass.previousPosition;

				vec3 nextPos = point_mass.Position
								+ (v_dt)
								+ accel * deltaTime * deltaTime;

				if (&point_mass == &pts[1]) {
					cout << "force: (" << point_mass.forces.x << ", "
						<< point_mass.forces.y << ", "
						<< point_mass.forces.z << ") | ";
					cout << "a = (" << accel.x << ", " << accel.y << ", " << accel.z << ") | ";
					cout << "t = " << glfwGetTime() << " | ";
					cout << "("
						<< point_mass.Position.x + pos.x << ", "
						<< point_mass.Position.y + pos.y << ", "
						<< point_mass.Position.z + pos.z << ")";
					cout << " -> (" << nextPos.x + pos.x <<
						", " << nextPos.y + pos.y <<
						", " << nextPos.z + pos.z << ") | ";
					cout << "v dt = (" << v_dt.x << ", " << v_dt.y << ", " << v_dt.z << ")" << endl;
				}

				point_mass.previousPosition = point_mass.Position;
				point_mass.Position = nextPos;
			}
		}

		void springConstrain() {
			const float minDist = 0.01;

			for (auto &spring : springs) {
				const float maxDist = 1.1f * spring.restLength;

				PointMass *pm_a = &pts[spring.v0];
				PointMass *pm_b = &pts[spring.v1];

				// Euclidean distance
				const float distance = glm::distance(pm_a->Position, pm_b->Position);
				vec3 ab_norm(0.0f, 1.0f, 0.0f);

				if (distance < minDist) {
					float diff = (minDist - distance);
					pm_a->Position -= 0.5f * diff * ab_norm;
					pm_b->Position += 0.5f * diff * ab_norm;
				}

				if (distance > maxDist) {
					float diff = (distance - maxDist);
					vec3 delta = normalize(pm_b->Position - pm_a->Position);
					// cout << "spring " << diff << "too long | rest length " << spring.restLength << " | actual length " << distance << endl;
					pm_a->Position += delta * 0.5f * diff;
					pm_b->Position -=  delta * 0.5f * diff;
				}
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
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PointMass), (void*)0);
			glEnableVertexAttribArray(0);
			// weight
			
			glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(PointMass), (void*)offsetof(PointMass, Position));
			glEnableVertexAttribArray(1);

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
						
						bool isTopZ = (k + 1 == nodesPerEdge);
						bool isTopX = (i + 1 == nodesPerEdge);
						bool isTopY = (j + 1 == nodesPerEdge);

						bool isBottomZ = (k == 0);
						bool isBottomX = (i == 0);
						bool isBottomY = (j == 0);

						bool bendX = (i + 2 < nodesPerEdge);
						bool bendY = (j + 2 < nodesPerEdge);
						bool bendZ = (k + 2 < nodesPerEdge);

						float k_val = 200;
						float kd = 6;
						float rl_edge = (float) length / (nodesPerEdge - 1);
						float rl_shear = sqrt(2 * rl_edge * rl_edge);
						float rl_body = sqrt(rl_shear * rl_shear + rl_edge * rl_edge);
						float rl_bend = rl_edge * 2;

						//cout << "edge, shear, body: " << rl_edge << " | " << rl_shear << " | " << rl_body << endl;

						int currIdx = i * (nodesPerEdge * nodesPerEdge) + j * nodesPerEdge + k;
						// connect z-axis edge
						if (!isTopZ) {
							int upIdx = currIdx + 1;
							springs.push_back(Spring(currIdx, upIdx, k_val, kd, rl_edge));
						}
						// connect x-axis edge
						if (!isTopX) {
							int rightIdx = currIdx + (nodesPerEdge * nodesPerEdge);
							springs.push_back(Spring(currIdx, rightIdx, k_val, kd, rl_edge));
						}
						// connect y-axis edge
						if (!isTopY) {
							int forIdx = currIdx + nodesPerEdge;
							springs.push_back(Spring(currIdx, forIdx, k_val, kd, rl_edge));
						}

						// connect to (x + 1, z + 1) | across x-z face
						if (!isTopZ && !isTopX) {
							int crossIdx = currIdx + 1 + nodesPerEdge * nodesPerEdge;
							springs.push_back(Spring(currIdx, crossIdx, k_val, kd, rl_shear));
						}
						// connect to (y + 1, z + 1) | across y-z face
						if (!isTopZ && !isTopY) {
							int crossIdx = currIdx + 1 + nodesPerEdge;
							springs.push_back(Spring(currIdx, crossIdx, k_val, kd, rl_shear));
						}
						// connect to (x + 1, y + 1) | across x-y face
						if (!isTopX && !isTopY) {
							int crossIdx = currIdx + nodesPerEdge + nodesPerEdge * nodesPerEdge;
							springs.push_back(Spring(currIdx, crossIdx, k_val, kd, rl_shear));
						}

						// connect to (y + 1, z - 1) | across y-z face down
						if (!isBottomZ && !isTopY) {
							int crossIdx = currIdx - 1 + nodesPerEdge;
							springs.push_back(Spring(currIdx, crossIdx, k_val, kd, rl_shear));
						}

						// connect to (x + 1, z - 1) | across x-z face down
						if (!isBottomZ && !isTopX) {
							int crossIdx = currIdx - 1 + nodesPerEdge * nodesPerEdge;
							springs.push_back(Spring(currIdx, crossIdx, k_val, kd, rl_shear));
						}
						
						// connect to (x + 1, y - 1) | across x-y face down
						if (!isTopX && !isBottomY) {
							int crossIdx = currIdx - nodesPerEdge + nodesPerEdge * nodesPerEdge;
							springs.push_back(Spring(currIdx, crossIdx, k_val, kd, rl_shear));
						}

						// connect to (x + 1, y + 1, z + 1) | across body
						if (!isTopZ && !isTopY && !isTopX) {
							int crossIdx = currIdx + 1 + nodesPerEdge + nodesPerEdge * nodesPerEdge;
							springs.push_back(Spring(currIdx, crossIdx, k_val, kd, rl_body));
						}

						// connect to (x + 1, y + 1, z - 1) | across body down
						if (!isBottomZ && !isTopY && !isTopX) {
							int crossIdx = currIdx - 1 + nodesPerEdge + nodesPerEdge * nodesPerEdge;
							springs.push_back(Spring(currIdx, crossIdx, k_val, kd, rl_body));
						}

						// connect to (x + 1, y - 1, z + 1) | across body down
						if (!isTopZ && !isBottomY && !isTopX) {
							int crossIdx = currIdx + 1 - nodesPerEdge + nodesPerEdge * nodesPerEdge;
							springs.push_back(Spring(currIdx, crossIdx, k_val, kd, rl_body));
						}

						// connect to (x - 1, y + 1, z + 1) | across body down
						if (!isTopZ && !isTopY && !isBottomX) {
							int crossIdx = currIdx + 1 + nodesPerEdge - nodesPerEdge * nodesPerEdge;
							springs.push_back(Spring(currIdx, crossIdx, k_val, kd, rl_body));
						}

						// bend
						if (bendX) {
							int bendIdx = currIdx + 2 * (nodesPerEdge * nodesPerEdge);
							springs.push_back(Spring(currIdx, bendIdx, k_val, kd, rl_bend));
						}
						if (bendY) {
							int bendIdx = currIdx + 2 * (nodesPerEdge);
							springs.push_back(Spring(currIdx, bendIdx, k_val, kd, rl_bend));
						}
						if (bendZ) {
							int bendIdx = currIdx + 2;
							springs.push_back(Spring(currIdx, bendIdx, k_val, kd, rl_bend));
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