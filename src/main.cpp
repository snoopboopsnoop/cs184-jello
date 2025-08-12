#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>

#include "shader.h"
#include "stb_image.h"
#include "camera.h"
#include "model.h"

using namespace std;
using namespace glm;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int loadTexture(char const* path);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float lastX = SCR_WIDTH / 2, lastY = SCR_HEIGHT / 2;
bool firstMouse = true;

// camera
Camera cam(vec3(0.0f, 0.0f, 3.0f));

// time
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// light
vec3 lightPos(1.2f, 1.0f, 2.0f);

// specular highlights
vec3 specularColor(1.0f, 1.0f, 1.0f);


int main() {
	// glfw initialization & configuration
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);


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
	
	//--------------------------------------------------------------

	stbi_set_flip_vertically_on_load(true);
	// load texture
	unsigned int diffuseMap = loadTexture("resources/objects/jello/jello_texture.jpg");
	unsigned int specularMap = loadTexture("resources/objects/jello/jello_texture.jpg");

	//--------------------------------------------------------------
	Shader ourShader("./shaders/translucent.vert", "./shaders/translucent.frag");
	Shader lightSourceShader("./shaders/shader.vs", "./shaders/lightSourceShader.fs");

	//--------------------------------------------------------------

	// load models
	// -----------
	// these 2 lines for crystal to comment out when basic translucency not working
	string modelPath = "resources/objects/jello/jello.obj";
	Model ourModel(modelPath);

	// render loop
	while (!glfwWindowShouldClose(window)) {
		// calculate frame time
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
		processInput(window); // handle inputs

		//render
		glClearColor(0.89f, 1.0f, 0.96f, 0.5f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//// wireframe mode (commented out bc translucency wants solid rendering)
		// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		vec3 lightColor;
		lightColor.x = sin(glfwGetTime() * 2.0f);
		lightColor.y = sin(glfwGetTime() * 0.7f);
		lightColor.z = sin(glfwGetTime() * 1.3f);

		vec3 diffuseColor = lightColor * vec3(0.5f);
		vec3 ambientColor = diffuseColor * vec3(0.2f);

		ourShader.use();
		ourShader.setVec3("DiffuseColor", diffuseColor);
		ourShader.setVec3("SpecularColor", specularColor);
		ourShader.setVec3("lightPos", lightPos);
		ourShader.setVec3("eyePos", cam.Position);

		/* no LUT texture code needed, we do basic translucency
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, skinLUTTextureID);
		ourShader.setInt("skinLUT", 0);
		*/

/*
		ourShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
		ourShader.setVec3("viewPos", cam.Position);

		ourShader.setInt("material.diffuse", 0);
		ourShader.setInt("material.specular", 1);
		ourShader.setFloat("material.shininess", 32.0f);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularMap);
*/
		//// directional light
		//ourShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
		//ourShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
		//ourShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
		//ourShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
		//// point light 1
		//ourShader.setVec3("pointLights[0].position", pointLightPositions[0]);
		//ourShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
		//ourShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
		//ourShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
		//ourShader.setFloat("pointLights[0].constant", 1.0f);
		//ourShader.setFloat("pointLights[0].linear", 0.09f);
		//ourShader.setFloat("pointLights[0].quadratic", 0.032f);
		//// point light 2
		//ourShader.setVec3("pointLights[1].position", pointLightPositions[1]);
		//ourShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
		//ourShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
		//ourShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
		//ourShader.setFloat("pointLights[1].constant", 1.0f);
		//ourShader.setFloat("pointLights[1].linear", 0.09f);
		//ourShader.setFloat("pointLights[1].quadratic", 0.032f);
		//// point light 3
		//ourShader.setVec3("pointLights[2].position", pointLightPositions[2]);
		//ourShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
		//ourShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
		//ourShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
		//ourShader.setFloat("pointLights[2].constant", 1.0f);
		//ourShader.setFloat("pointLights[2].linear", 0.09f);
		//ourShader.setFloat("pointLights[2].quadratic", 0.032f);
		//// point light 4
		//ourShader.setVec3("pointLights[3].position", pointLightPositions[3]);
		//ourShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
		//ourShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
		//ourShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
		//ourShader.setFloat("pointLights[3].constant", 1.0f);
		//ourShader.setFloat("pointLights[3].linear", 0.09f);
		//ourShader.setFloat("pointLights[3].quadratic", 0.032f);
		//// spotLight
		//ourShader.setVec3("spotLight.position", cam.Position);
		//ourShader.setVec3("spotLight.direction", cam.Front);
		//ourShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
		//ourShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
		//ourShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
		//ourShader.setFloat("spotLight.constant", 1.0f);
		//ourShader.setFloat("spotLight.linear", 0.09f);
		//ourShader.setFloat("spotLight.quadratic", 0.032f);
		//ourShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
		//ourShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

		// camera
		mat4 view = cam.GetViewMatrix();
		ourShader.setMat4("view", view);

		mat4 projection;
		projection = perspective(radians(cam.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		ourShader.setMat4("projection", projection);

		//ourShader.use();

		//glBindVertexArray(VAO);
		//glDrawArrays(GL_TRIANGLES, 0, 36);

		for (unsigned int i = 0; i < 3; i++)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = translate(model, cubePositions[i]);
			float angle = 20.0f * i;
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			ourShader.setMat4("model", model);

			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		/*lightSourceShader.use();
		lightSourceShader.setMat4("view", view);
		lightSourceShader.setMat4("projection", projection);
		for (unsigned int i = 0; i < 4; i++) {
			mat4 model = mat4(1.0f);
			model = translate(model, pointLightPositions[i]);
			model = scale(model, vec3(0.2f));
			lightSourceShader.setMat4("model", model);

			glBindVertexArray(lightVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}*/

		//ourShader.use();
		//// render the loaded model
		//glm::mat4 model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		//model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		//ourShader.setMat4("model", model);
		//ourModel.Draw(ourShader);

		glfwSwapBuffers(window); // swap color buffer
		glfwPollEvents(); // checks if any events were triggered
	}

	// de-allocate resources
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

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