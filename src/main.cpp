#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>

#include "modelShader.h"
#include "shader.h"
#include "stb_image.h"
#include "camera.h"
#include "model.h"
#include "cage.h"

using namespace std;
using namespace glm;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int loadTexture(char const* path);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

float lastX = SCR_WIDTH / 2, lastY = SCR_HEIGHT / 2;
bool firstMouse = true;

// camera
Camera cam(vec3(0.0f, 3.0f, 20.0f));

// time
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// light
vec3 lightPos(1.2f, 1.0f, 2.0f);

// specular highlights
vec3 specularColor(1.0f, 1.0f, 1.0f);

// frustum
const float fclip = 100.0f;
const float nclip = 1.0f;

// physics
const float dt = 1.0f / 60;
float tAccum = 0.0f;

bool runPhysics = false;

// render settings
DrawMode mode = PHYSICS;

int main() {
	// glfw initialization & configuration
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif

	// create window
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);

	if (window == NULL) {
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	// call framebuffer_size_callback on resize
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// mouse movement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, keyCallback);

	// check that GLAD has loaded
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		cout << "Failed to initialize GLAD" << endl;
		return -1;
	}
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	glEnable(GL_DEPTH_TEST);

	// enable blending for translucency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// disabling face culling allows translucency from all angles
	glDisable(GL_CULL_FACE);

	//--------------------------------------------------------------

	unsigned int indices[] = {  // note that we start from 0!
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	float vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};



	// positions all containers
	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	glm::vec3 pointLightPositions[] = {
	glm::vec3(0.7f,  0.2f,  2.0f),
	glm::vec3(2.3f, -3.3f, -4.0f),
	glm::vec3(-4.0f,  2.0f, -12.0f),
	glm::vec3(0.0f,  0.0f, -3.0f)
	};



	// floor plane
	float plane[] = {
		 fclip, 0,  fclip, 
		 fclip, 0, -fclip,  
		-fclip, 0, fclip,  
		-fclip, 0, -fclip,  
	};

	float wall[] = {
		 50, 0,  50,
		 50, 0, -50,
		 -50, 0, 50,
		-50, 0, -50,
	};

	unsigned int planeIdx[] = {
		0, 2, 1,
		3, 1, 2
	};

	//--------------------------------------------------------------

	// generate buffers
	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	// vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO); // bind VBO to GL_ARRAY_BUFFER
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// vertex array
	glBindVertexArray(VAO);

	// define vertex data format
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	// normals
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// diffuse map
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// light vao
	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// floor vao
	unsigned int floorVAO, floorVBO, floorEBO;
	glGenBuffers(1, &floorVBO);
	glGenVertexArrays(1, &floorVAO);
	glGenBuffers(1, &floorEBO);

	glBindVertexArray(floorVAO);

	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(plane), plane, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIdx), planeIdx, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// wall vao
	unsigned int wallVAO, wallVBO, wallEBO;
	glGenBuffers(1, &wallVBO);
	glGenVertexArrays(1, &wallVAO);
	glGenBuffers(1, &wallEBO);

	glBindVertexArray(wallVAO);

	glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall), wall, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wallEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIdx), planeIdx, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//--------------------------------------------------------------

	stbi_set_flip_vertically_on_load(true);
	// load texture
	unsigned int diffuseMap = loadTexture("resources/objects/jello/jello_texture.jpg");
	unsigned int specularMap = loadTexture("resources/objects/jello/jello_texture.jpg");

	//--------------------------------------------------------------

	Shader ourShader("./shaders/model_shader.vertex", "./shaders/model_shader.frag");
	Shader translucentShader("./shaders/translucent.vert", "./shaders/translucent.frag");
	//Shader lightSourceShader("./shaders/shader.vs", "./shaders/lightSourceShader.fs");
	Shader ptShader("./shaders/pt_shader.vertex", "./shaders/pt_shader.frag");
	Shader lineShader("./shaders/line_shader.vertex", "./shaders/line_shader.frag");
	Shader planeShader("./shaders/plane_shader.vertex", "./shaders/plane_shader.frag");

	// jello shaders
	ModelShader jelloShader;
	jelloShader.ptMassShader = &ptShader;
	jelloShader.springShader = &lineShader;
	jelloShader.matShader = &translucentShader;

	// plate shaders
	ModelShader plateShader;
	plateShader.ptMassShader = &ptShader;
	plateShader.springShader = &lineShader;
	plateShader.matShader = &ourShader;

	//--------------------------------------------------------------

	cam.Pitch = -20.0f;

	// load models
	// -----------

	string modelPath = "resources/objects/jello/jello.obj";
	Model jello(modelPath, jelloShader, false);

	string platePath = "resources/objects/plate/plate.obj";
	Model plateModel(platePath, plateShader);

	// load some point masses
	vec3 start(0.0f, 5.0f, 0.0f);
	Cube c(jelloShader, 2, 2, start);
	/*vector<PointMass> pts;
	pts.push_back(PointMass(vec3(0.0f, -0.5f, 0.0f), 1));
	pts.push_back(PointMass(vec3(0.0f, 0.5f, 0.0f), 1));

	vector<Spring> springs;
	springs.push_back(Spring(0, 1, 20, 6, 1));
	
	vec3 pos(0.0f, 5.0f, 0.0f);

	Cage c(pts, springs, pos);*/

	// render loop
	lastFrame = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		// calculate frame time
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
		processInput(window); // handle inputs

		//render
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//// wireframe mode (commented out bc translucency wants solid rendering)
		// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// physics
		tAccum += deltaTime;
		//cout << "dt: " << deltaTime << " | accum: " << tAccum << endl;
		if (tAccum >= dt) {

			if (runPhysics) {
				jello.cage.updatePhysics(window, dt);
				jello.cage.verletStep(dt, 0.0f);
				jello.cage.satisfyConstraints(0.0f);
				jello.cage.springConstrain();
				jello.cage.refreshMesh();

				c.updatePhysics(window, dt);
				c.verletStep(dt, 0.7f);
				c.satisfyConstraints(0.0f);
				c.springConstrain();
				c.refreshMesh();
				//runPhysics = false;
			}
			tAccum = 0;
		}

		// camera
		mat4 view = cam.GetViewMatrix(); 
		mat4 projection = perspective(radians(cam.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, nclip, fclip);

		ourShader.use();
		ourShader.setMat4("view", view);
		ourShader.setMat4("projection", projection);

		// render plane
		vec3 planeColor(0.2f, 0.3f, 0.2f);

		planeShader.use();
		planeShader.setMat4("view", view);
		planeShader.setMat4("projection", projection);
		planeShader.setMat4("model", mat4(1.0f));

		planeShader.setVec3("objectColor", planeColor);

		glBindVertexArray(floorVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// render wall back
		vec3 wallColor(0.6f, 0.6f, 0.6f);
		planeShader.use();
		planeShader.setMat4("view", view);
		planeShader.setMat4("projection", projection);
		mat4 wallModel(1.0f);
		wallModel = rotate(wallModel, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
		wallModel = translate(wallModel, vec3(0.0f, -10.0f, -5.0f));
		planeShader.setMat4("model", wallModel);
		planeShader.setVec3("objectColor", wallColor);

		glBindVertexArray(wallVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// render wall right
		wallColor = vec3(0.4f, 0.4f, 0.7f);

		wallModel = mat4(1.0f);
		wallModel = rotate(wallModel, radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
		wallModel = translate(wallModel, vec3(5.0f, -10.0f, 0.0f));
		planeShader.setMat4("model", wallModel);

		planeShader.setVec3("objectColor", wallColor);

		glBindVertexArray(wallVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// render wall left
		wallColor = vec3(0.7f, 0.4f, 0.4f);

		wallModel = mat4(1.0f);
		wallModel = rotate(wallModel, radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
		wallModel = translate(wallModel, vec3(5.0f, 10.0f, 0.0f));
		planeShader.setMat4("model", wallModel);

		planeShader.setVec3("objectColor", wallColor);

		glBindVertexArray(wallVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// render cube
		ptShader.use();
		ptShader.setMat4("view", view);
		ptShader.setMat4("projection", projection);
		ptShader.setMat4("model", mat4(1.0f));

		lineShader.use();
		lineShader.setMat4("view", view);
		lineShader.setMat4("projection", projection);
		lineShader.setMat4("model", mat4(1.0f));
		
		translucentShader.use();
		translucentShader.setMat4("view", view);
		translucentShader.setMat4("projection", projection);
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 5.0f, 0.0f));
		translucentShader.setMat4("model", model);

		vec3 lightColor(1.0f, 0.0f, 0.0f);
		vec3 diffuseColor = lightColor * vec3(0.6f);
		vec3 ambientColor = diffuseColor * vec3(0.8f);
		vec3 specularColor = vec3(1.0f, 1.0f, 1.0f);

		translucentShader.setVec3("lightPos", lightPos);
		translucentShader.setVec3("eyePos", cam.Position);
		translucentShader.setVec3("DiffuseColor", diffuseColor);
		translucentShader.setVec3("AmbientColor", ambientColor);
		translucentShader.setVec3("SpecularColor", specularColor);

		vec3 jelloColor(0.9f, 0.3f, 0.3f);
		translucentShader.setVec3("objColor", jelloColor);

		/*ourShader.use();
		ourShader.setMat4("model", model);*/
		c.Draw(mode);

		//jello.Draw(mode);

		glfwSwapBuffers(window); // swap color buffer
		glfwPollEvents(); // checks if any events were triggered
	}

	// clean glfw resources
	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		cam.ProcessKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		cam.ProcessKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		cam.ProcessKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		cam.ProcessKeyboard(RIGHT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		cam.ProcessKeyboard(UP, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		cam.ProcessKeyboard(DOWN, deltaTime);
	}
	/*if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
		mode = OBJECT;
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		mode = PHYSICS;
	}*/
	/*if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		runPhysics = !runPhysics;
	}*/
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	cam.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	cam.ProcessMouseScroll(yoffset);
}

unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		if (mode == PHYSICS) mode = OBJECT;
		else if (mode == OBJECT) mode = PHYSICS;
	}
	//else if (key == GLFW_KEY_O && action == GLFW_PRESS) {
	//	mode = OBJECT;
	//}
	else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		runPhysics = !runPhysics;
	}
}