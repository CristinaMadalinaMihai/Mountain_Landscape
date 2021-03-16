//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"

#include <iostream>

int glWindowWidth = 1000;
int glWindowHeight = 800;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 8192; // to be modified if needed
const unsigned int SHADOW_HEIGHT = 8192; // to be modified if needed

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

////////////////// directional light
glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;
///////////////////// point light
glm::vec3 pointLight;
GLuint pointLightLoc;
bool pointLightEffect;
////////////////////// spot light
bool spotEffect = false;
glm::vec3 spotLight;
GLuint spotLightLoc;
glm::vec3 spotLightDir;
GLuint spotLightDirLoc;
float newX = 0.071923f;
float newZ = 0.102006f;

gps::Camera myCamera(
	glm::vec3(0.0f, 1.0f, 2.5f),
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.1f; // to be modified if needed

//////// camera animation
bool cameraAnimation = false;
bool camStarted = false;
float temp1, temp2, temp3, temp4, temp5, temp6, temp7 = 0;

bool pressedKeys[1024];
float angleY = 0.0f;
GLfloat lightAngle;

gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D scenaModel; ////// scena mea
gps::Model3D backleftModel; ////// car tyre
gps::Model3D carbodyModel; ///// car body
gps::Model3D dropModel; ///// water dropssssssss
float xVector[1000];
float zVector[1000];
float yVector[1000];
float yVectorCopy[1000];
float xVectorCopy[1000];
float zVectorCopy[1000];
bool rainEffect = false;

/////// carrrrr
bool carAnimation = false;
float moveCarX = 0.085862f;
float moveCarZ = 0.305371f;
float t1X = 0.081623f;
float t1Z = 0.330741f;
float t2X = 0.104744f;
float t2Z = 0.319248f;
float t3X = 0.087584f;
float t3Z = 0.28186f;
float t4X = 0.064654f;
float t4Z = 0.291657f;


bool fogEffect = false; //// fog
bool windEffect = false; //// wind

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;
gps::Shader spotLightShader;


GLuint shadowMapFBO;
GLuint depthMapTexture;
bool showDepthMap;

/////////// mouse movement
bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = retina_width / 2.0;
float lastY = retina_height / 2.0;

GLenum glCheckError_(const char* file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO	
		//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xPosition, double yPosition) {

	if (firstMouse)
	{
		lastX = xPosition;
		lastY = yPosition;
		firstMouse = false;
	}

	float xoffset = xPosition - lastX;
	float yoffset = lastY - yPosition;  // reversed since y-coordinates go from bottom to top
	lastX = xPosition;
	lastY = yPosition;

	float sensitivity = 0.10f; // change this value to your liking
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	myCamera.rotate(pitch, yaw);
}

float zoom = 45.0f;
gps::MOVE_DIRECTION mouseScrollDirection;
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	zoom -= (float)yoffset; // amount scrolled vertically

	if (zoom < 1.0f)
		zoom = 1.0f;
	if (zoom > 45.0f)
		zoom = 45.0f;

	if (xoffset < yoffset)
		mouseScrollDirection = gps::MOVE_BACKWARD;
	else
		mouseScrollDirection = gps::MOVE_FORWARD;

	myCamera.move(mouseScrollDirection, 0.1f);
}

void processMovement()
{
	if (pressedKeys[GLFW_KEY_O]) { ///////// start camera animation
		cameraAnimation = true;
		if (!camStarted) {
			myCamera.setCamera(glm::vec3(0.0f, 0.1f, 1.0f),
				glm::vec3(0.0f, 0.0f, 10.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
			myCamera.rotate(pitch, yaw);
			temp1 = temp2 = temp3 = temp4 = temp5 = temp6 = temp7 = 0;
		}
		camStarted = true;
	}

	if (pressedKeys[GLFW_KEY_P]) { ///// end camera animation
		cameraAnimation = false;
	}

	if (pressedKeys[GLFW_KEY_G]) { ///// start object (car) animation
		carAnimation = true;
	}

	if (pressedKeys[GLFW_KEY_H]) { ///// end object animation -> reposition to initial position
		carAnimation = false;
		moveCarX = 0.085862f;
		moveCarZ = 0.305371f;
		t1X = 0.081623f;
		t1Z = 0.330741f;
		t2X = 0.104744f;
		t2Z = 0.319248f;
		t3X = 0.087584f;
		t3Z = 0.28186f;
		t4X = 0.064654f;
		t4Z = 0.291657f;
	}
	
	if (pressedKeys[GLFW_KEY_N]) { ///// wireframe objects
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (pressedKeys[GLFW_KEY_M]) { ///// solid objects
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (pressedKeys[GLFW_KEY_Q]) { //// rotate camera
		yaw -= 1.0f;
		myCamera.rotate(pitch, yaw);
	}

	if (pressedKeys[GLFW_KEY_E]) { //// rotate camera
		yaw += 1.0f;
		myCamera.rotate(pitch, yaw);
	}

	if (pressedKeys[GLFW_KEY_Z]) { //// rotate scene
		angleY -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_C]) { //// rotate scene
		angleY += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_J]) { //// rotate light cube
		lightAngle -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_L]) { //// rotate light cube
		lightAngle += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_1]) { //// foggy day
		fogEffect = true;
	}

	if (pressedKeys[GLFW_KEY_2]) { //// not a foggy day
		fogEffect = false;
	}

	if (pressedKeys[GLFW_KEY_3]) { //// spot light
		spotEffect = true;
	}

	if (pressedKeys[GLFW_KEY_4]) { //// no spot light
		spotEffect = false;
	}

	if (pressedKeys[GLFW_KEY_5]) { //// start rain
		rainEffect = true;
	}

	if (pressedKeys[GLFW_KEY_6]) { //// stop rain
		rainEffect = false;
	}

	if (pressedKeys[GLFW_KEY_7]) { //// start wind
		windEffect = true;
		rainEffect = true;
	}

	if (pressedKeys[GLFW_KEY_8]) { //// stop wind
		windEffect = false;
		//rainEffect = false; 
	}

	if (pressedKeys[GLFW_KEY_9]) { //// polygonal surfaces
		glDisable(GL_MULTISAMPLE);
	}

	if (pressedKeys[GLFW_KEY_0]) { //// smooth surfaces
		glEnable(GL_MULTISAMPLE);
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_X]) {
		exit(0);
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4); ///// smooth surface - multisampling/antialiasing

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetScrollCallback(glWindow, scrollCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	// for multisampling/antialising - is enabled by default anyway
	

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	lightCube.LoadModel("objects/cube/cube.obj");
	screenQuad.LoadModel("objects/quad/quad.obj");

	scenaModel.LoadModel("objects/scenamea/scene2.obj", "objects/scenamea/");
	backleftModel.LoadModel("objects/scenamea/t1.obj", "objects/scenamea/");
	carbodyModel.LoadModel("objects/scenamea/cartrial4.obj", "objects/scenamea/");
	dropModel.LoadModel("objects/scenamea/drop.obj", "objects/scenamea/");
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("shaders/depth.vert", "shaders/depth.frag");
	depthMapShader.useShaderProgram();

	spotLightShader.loadShader("shaders/spotLight.vert", "shaders/spotLight.frag");
	spotLightShader.useShaderProgram();
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	GLuint pointLightLoc = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightVariable");
	glUniform1i(pointLightLoc, 0);

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

	//bind nothing to attachment points
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	//unbind until ready to use
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

glm::mat4 computeLightSpaceTrMatrix() {
	//TODO - Return the light-space transformation matrix
	const GLfloat near_plane = 0.1f, far_plane = 5.0f; // to be modified if needed

	glm::mat4 lightView = glm::lookAt(glm::mat3(lightRotation) * lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightProjection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, near_plane, far_plane);

	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

	return lightSpaceTrMatrix;
}

void drawObjects(gps::Shader shader, bool depthPass) {

	shader.useShaderProgram();

	//model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	//glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	//// do not send the normal matrix if we are rendering in the depth map
	//if (!depthPass) {
	//	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	//	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	//}

	//nanosuit.Draw(shader);

	//////////////// drawing my objectsssss
	// draw my scene
	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	scenaModel.Draw(shader);

	// draw car animation
	if (carAnimation && moveCarX >= -0.09f) {
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(moveCarX, 0.014125f, moveCarZ));
		model = glm::rotate(model, 0.9f, glm::vec3(0, 1, 0));
		moveCarZ -= 0.01;
		moveCarX -= 0.0045;

		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}
		
		carbodyModel.Draw(shader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(t1X, 0.00794f, t1Z));
		model = glm::rotate(model, 0.9f, glm::vec3(0, 1, 0));
		t1Z -= 0.01;
		t1X -= 0.0045;
		
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		backleftModel.Draw(shader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(t2X, 0.00794f, t2Z));
		model = glm::rotate(model, 0.9f, glm::vec3(0, 1, 0));
		t2Z -= 0.01;
		t2X -= 0.0045;
		
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		backleftModel.Draw(shader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(t3X, 0.00794f, t3Z));
		model = glm::rotate(model, 0.9f, glm::vec3(0, 1, 0));
		t3Z -= 0.01;
		t3X -= 0.0045;
		
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		backleftModel.Draw(shader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(t4X, 0.00794f, t4Z));
		model = glm::rotate(model, 0.9f, glm::vec3(0, 1, 0));
		t4Z -= 0.01;
		t4X -= 0.0045;
		
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		backleftModel.Draw(shader);
	}
	else {
		model = glm::translate(model, glm::vec3(moveCarX, 0.014125f, moveCarZ));
		model = glm::rotate(model, 0.9f, glm::vec3(0, 1, 0));
		
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		carbodyModel.Draw(shader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(t1X, 0.00794f, t1Z));
		model = glm::rotate(model, 0.9f, glm::vec3(0, 1, 0));

		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		backleftModel.Draw(shader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(t2X, 0.00794f, t2Z));
		model = glm::rotate(model, 0.9f, glm::vec3(0, 1, 0));
		
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		backleftModel.Draw(shader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(t3X, 0.00794f, t3Z));
		model = glm::rotate(model, 0.9f, glm::vec3(0, 1, 0));
		
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		backleftModel.Draw(shader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(t4X, 0.00794f, t4Z));
		model = glm::rotate(model, 0.9f, glm::vec3(0, 1, 0));
		
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		backleftModel.Draw(shader);
	}

	// draw my water drops
	for (int i = 0; i < 1000; i++) {
		model = glm::mat4(1.0f);
		if (windEffect) {
			model = glm::translate(model, glm::vec3(xVector[i], yVector[i], zVector[i]));
			model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		}
		else {
			model = glm::translate(model, glm::vec3(xVector[i], yVector[i], zVector[i]));
			model = glm::rotate(model, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		dropModel.Draw(shader);
	}
	for (int i = 0; i < 1000; i++) {
		model = glm::mat4(1.0f);
		if (windEffect) {
			model = glm::translate(model, glm::vec3(xVector[i], yVector[i], zVector[i]));
			model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		}
		else {
			model = glm::translate(model, glm::vec3(xVector[i], yVector[i], zVector[i]));
			model = glm::rotate(model, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		dropModel.Draw(shader);
	}
	for (int i = 0; i < 1000; i++) {
		model = glm::mat4(1.0f);
		if (windEffect) {
			model = glm::translate(model, glm::vec3(xVector[i], yVector[i], zVector[i]));
			model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		}
		else {
			model = glm::translate(model, glm::vec3(xVector[i], yVector[i], zVector[i]));
			model = glm::rotate(model, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		dropModel.Draw(shader);
	}
	if (rainEffect) {
		for (int i = 0; i < 1000; i++) {
			yVector[i] -= 0.04f;
			if (yVector[i] < -1.0f) {
				yVector[i] = yVectorCopy[i];
			}
		}
	} else {
		for (int i = 0; i < 1000; i++) {
			if (yVector[i] != yVectorCopy[i]) {
				yVector[i]-= 0.04f;
			}
			if (yVector[i] < -4.0f) {
				yVector[i] = yVectorCopy[i];
			}
		}
	}

	if (windEffect) {
		for (int i = 0; i < 1000; i++) {
			xVector[i] -= 0.01f;
			if (xVector[i] < -4.0f) {
				xVector[i] = xVectorCopy[i];
			}
		}
	}
	
}

void renderScene() {

	// depth maps creation pass
	//TODO - Send the light-space transformation matrix to the depth map creation shader and
	//		 render the scene in the depth map
	depthMapShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	//render scene = draw objects
	drawObjects(depthMapShader, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render depth map on screen - toggled with the M key
	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {

		// final scene rendering pass (with shadows)

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader, false);

		//draw a white cube around the light

		lightShader.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		model = lightRotation;
		model = glm::translate(model, 1.0f * lightDir);
		model = glm::scale(model, glm::vec3(0.01f, 0.05f, 0.05f));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		//lightCube.Draw(lightShader);

		//// point light
		glUseProgram(myCustomShader.shaderProgram);	
		if (carAnimation) {
			GLint uniformLocation = glGetUniformLocation(myCustomShader.shaderProgram, "lightPosEye");
			glUniform3f(uniformLocation, moveCarX, 0.003332f, moveCarZ);

			uniformLocation = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightVariable");
			glUniform1i(uniformLocation, 1);

			
		}
		else {
			GLint uniformLocation = glGetUniformLocation(myCustomShader.shaderProgram, "lightPosEye");
			glUniform3f(uniformLocation, moveCarX, -1.0f, moveCarZ);

			uniformLocation = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightVariable");
			glUniform1i(uniformLocation, 0);

			
		}

		////// spot light
		glUseProgram(myCustomShader.shaderProgram);
		if (spotEffect) {
			GLint uniformLocation = glGetUniformLocation(myCustomShader.shaderProgram, "lightPosEyeSL");
			glUniform3f(uniformLocation, newX, -0.1f, newZ); 
			
			uniformLocation = glGetUniformLocation(myCustomShader.shaderProgram, "spotLightVariable");
			glUniform1i(uniformLocation, 1);		
		}
		else {
			GLint uniformLocation = glGetUniformLocation(myCustomShader.shaderProgram, "lightPosEyeSL");
			glUniform3f(uniformLocation, newX, -0.1f, newZ); 

			uniformLocation = glGetUniformLocation(myCustomShader.shaderProgram, "spotLightVariable");
			glUniform1i(uniformLocation, 0);
		}
		
	}

	//////// foggg
	glUseProgram(myCustomShader.shaderProgram);
	GLint uniformLocation = glGetUniformLocation(myCustomShader.shaderProgram, "fogVariable");
	if (fogEffect) {
		glUniform1i(uniformLocation, 1);
	}
	else {
		glUniform1i(uniformLocation, 0);
	}

	//////// camera animation
	if (cameraAnimation) {
		if (cameraAnimation && temp1 < 20) {
			myCamera.rotate(pitch, yaw++);
			temp1++;
		}

		if (cameraAnimation && temp1 >= 20 && temp2 < 60) {
			myCamera.move(gps::MOVE_FORWARD, 0.01);
			temp2++;
		}

		if (cameraAnimation && temp2 >= 60 && temp3 < 40) {
			myCamera.rotate(pitch, yaw++);
			temp3++;
		}

		if (cameraAnimation && temp3 >= 40 && temp4 < 40) {
			myCamera.move(gps::MOVE_FORWARD, 0.01);
			temp4++;
		}

		if (cameraAnimation && temp4 >= 40 && temp5 < 15) {
			myCamera.rotate(pitch, yaw--);
			temp5++;
		}

		if (cameraAnimation && temp5 >= 15 && temp6 < 100) {
			myCamera.move(gps::MOVE_FORWARD, 0.01);
			temp6++;
		}

		if (cameraAnimation && temp6 >= 100 && temp7 < 150) {
			myCamera.rotate(pitch, yaw--);
			temp7++;
		}

		if (temp7 >= 150) {
			cameraAnimation = false;
			camStarted = false;
			myCamera.setCamera(glm::vec3(0.0f, 0.1f, 1.0f),
				glm::vec3(0.0f, 0.0f, 10.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
			yaw = yaw + 100;
			myCamera.rotate(pitch, yaw);
			temp1 = temp2 = temp3 = temp4 = temp5 = temp6 = temp7 = 0;
		}

	}
}

void cleanup() {
	glDeleteTextures(1, &depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char* argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();

	/////////// rainnnnnn
	for (int i = 0; i < 1000; i+=4) {
		float x = rand() % 500;
		float y = rand() % 500;
		xVector[i] = x * 0.006f;
		zVector[i] = y * 0.006f;
		xVector[i + 1] = -xVector[i];
		zVector[i + 1] = -zVector[i];
		xVector[i + 2] = xVector[i];
		zVector[i + 2] = -zVector[i];
		xVector[i + 3] = -xVector[i];
		zVector[i + 3] = zVector[i];

		float z = rand() % 50;
		yVector[i] = z * 0.02f + 3.0f;
		yVector[i + 1] = z * 0.02f + 3.0f;
		yVector[i + 2] = -z * 0.02f + 3.0f;
		yVector[i + 3] = -z * 0.02f + 3.0f;
	}
	for (int i = 0; i < 1000; i++) {
		yVectorCopy[i] = yVector[i];
		xVectorCopy[i] = xVector[i];
		zVectorCopy[i] = zVector[i];
	}


	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}
