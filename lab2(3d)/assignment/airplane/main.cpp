#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
using namespace std;

const unsigned int WIDTH = 1200;
const unsigned int HEIGHT = 900;
const float PI = 3.14159265359f;

// Camera & Global Variables
glm::vec3 camPos(0.0f, 2.0f, 15.0f);
glm::vec3 camFront(0.0f, 0.0f, -1.0f);
glm::vec3 camUp(0.0f, 1.0f, 0.0f);
float yaw = -90.0f, pitch = 0.0f, roll = 0.0f;

bool lightOn = true;
bool showInterior = false;
bool showCockpit = false;  // New: cockpit view
float doorAngle = 0.0f;
float wheelRotation = 0.0f;  // New: wheel rotation angle

unsigned int shader;
unsigned int cubeVAO, cubeVBO;
unsigned int cylinderVAO, cylinderVBO;
unsigned int coneVAO, coneVBO;
unsigned int diskVAO, diskVBO;
int cylinderVertexCount = 0;
int coneVertexCount = 0;
int diskVertexCount = 0; 

void printControls() {
	std:: cout << " Hello  world" << endl;

	std::cout << "================= AIRPLANE CONTROLS =================" << std::endl;
	std::cout << " [W, A, S, D] : Move Camera (Forward, Left, Back, Right)" << std::endl;
	std::cout << " [E, R]       : Move Camera Vertical (Up, Down)" << std::endl;
	std::cout << " [X, Y, Z]    : Rotate Camera (Pitch, Yaw, Roll)" << std::endl;
	std::cout << "---------------------------------------------------" << std::endl;
	std::cout << " [I]          : Toggle Interior/Cabin View" << std::endl;
	std::cout << " [P]          : Toggle Cockpit View" << std::endl;
	std::cout << " [O]          : Open Door" << std::endl;
	std::cout << " [C]          : Close Door" << std::endl;
	std::cout << " [L]          : Toggle Cabin Lights On/Off" << std::endl;
	std::cout << " [M]          : Rotate Wheels (simulate movement)" << std::endl;
	std::cout << "====================================================" << std::endl;
}

std::string loadShader(const char* path) {
	std::ifstream file(path, std::ios::binary);
	std::stringstream ss;
	if (file.is_open()) {
		ss << file.rdbuf();
		std::string content = ss.str();
		if (content.size() >= 3 && (unsigned char)content[0] == 0xEF) content.erase(0, 3);
		return content;
	}
	return "";
}

// Create cylinder mesh
void createCylinderVAO() {
	std::vector<float> vertices;
	int segments = 32;
	
	for (int i = 0; i < segments; i++) {
		float theta1 = (float)i / segments * 2.0f * PI;
		float theta2 = (float)(i + 1) / segments * 2.0f * PI;
		
		float x1 = cos(theta1), z1 = sin(theta1);
		float x2 = cos(theta2), z2 = sin(theta2);
		
		vertices.insert(vertices.end(), {x1, -0.5f, z1, x1, 0.5f, z1, x2, 0.5f, z2});
		vertices.insert(vertices.end(), {x1, -0.5f, z1, x2, 0.5f, z2, x2, -0.5f, z2});
		vertices.insert(vertices.end(), {0, 0.5f, 0, x1, 0.5f, z1, x2, 0.5f, z2});
		vertices.insert(vertices.end(), {0, -0.5f, 0, x2, -0.5f, z2, x1, -0.5f, z1});
	}
	
	cylinderVertexCount = (int)vertices.size() / 3;
	
	glGenVertexArrays(1, &cylinderVAO);
	glGenBuffers(1, &cylinderVBO);
	glBindVertexArray(cylinderVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

void createConeVAO() {
	std::vector<float> vertices;
	int segments = 32;
	
	for (int i = 0; i < segments; i++) {
		float theta1 = (float)i / segments * 2.0f * PI;
		float theta2 = (float)(i + 1) / segments * 2.0f * PI;
		
		float x1 = cos(theta1), z1 = sin(theta1);
		float x2 = cos(theta2), z2 = sin(theta2);
		
		vertices.insert(vertices.end(), {0, 0.5f, 0, x1, -0.5f, z1, x2, -0.5f, z2});
		vertices.insert(vertices.end(), {0, -0.5f, 0, x2, -0.5f, z2, x1, -0.5f, z1});
	}
	
	coneVertexCount = (int)vertices.size() / 3;
	
	glGenVertexArrays(1, &coneVAO);
	glGenBuffers(1, &coneVBO);
	glBindVertexArray(coneVAO);
	glBindBuffer(GL_ARRAY_BUFFER, coneVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

void createDiskVAO() {
	std::vector<float> vertices;
	int segments = 32;
	
	for (int i = 0; i < segments; i++) {
		float theta1 = (float)i / segments * 2.0f * PI;
		float theta2 = (float)(i + 1) / segments * 2.0f * PI;
		
		vertices.insert(vertices.end(), {0, 0, 0, cos(theta1), 0, sin(theta1), cos(theta2), 0, sin(theta2)});
	}
	
	diskVertexCount = (int)vertices.size() / 3;
	
	glGenVertexArrays(1, &diskVAO);
	glGenBuffers(1, &diskVBO);
	glBindVertexArray(diskVAO);
	glBindBuffer(GL_ARRAY_BUFFER, diskVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

void processInput(GLFWwindow* window, float dt) {
	float camSpeed = 8.0f * dt;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camPos += camSpeed * camFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camPos -= camSpeed * camFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camPos -= glm::normalize(glm::cross(camFront, camUp)) * camSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camPos += glm::normalize(glm::cross(camFront, camUp)) * camSpeed;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) camPos.y += camSpeed;
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) camPos.y -= camSpeed;

	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) pitch += 50.0f * dt;
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) yaw += 50.0f * dt;
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) roll += 50.0f * dt;

	static bool iP = false, lP = false, pP = false;
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS && !iP) { 
		showInterior = !showInterior; 
		showCockpit = false;
		if (showInterior) {
			// Move camera to interior view
			camPos = glm::vec3(2.0f, 0.0f, 0.0f);
			yaw = 180.0f;
			pitch = 0.0f;
		}
		iP = true; 
	}
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_RELEASE) iP = false;
	
	// Cockpit view toggle
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pP) { 
		showCockpit = !showCockpit; 
		showInterior = false;
		if (showCockpit) {
			// Move camera to cockpit position
			camPos = glm::vec3(6.5f, 0.1f, 0.0f);
			yaw = 0.0f;
			pitch = 0.0f;
		}
		pP = true; 
	}
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) pP = false;
	
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !lP) { lightOn = !lightOn; lP = true; }
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE) lP = false;

	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) doorAngle = glm::min(doorAngle + 60.0f * dt, 90.0f);
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) doorAngle = glm::max(doorAngle - 60.0f * dt, 0.0f);
	
	// Wheel rotation
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) wheelRotation += 200.0f * dt;
}

void drawCube(glm::mat4 model, glm::mat4 view, glm::mat4 proj, glm::vec3 color) {
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
	glUniform3fv(glGetUniformLocation(shader, "ourColor"), 1, glm::value_ptr(color));
	glUniform1i(glGetUniformLocation(shader, "lightOn"), 1);
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void drawCylinder(glm::mat4 model, glm::mat4 view, glm::mat4 proj, glm::vec3 color) {
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
	glUniform3fv(glGetUniformLocation(shader, "ourColor"), 1, glm::value_ptr(color));
	glUniform1i(glGetUniformLocation(shader, "lightOn"), 1);
	glBindVertexArray(cylinderVAO);
	glDrawArrays(GL_TRIANGLES, 0, cylinderVertexCount);
}

void drawCone(glm::mat4 model, glm::mat4 view, glm::mat4 proj, glm::vec3 color) {
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
	glUniform3fv(glGetUniformLocation(shader, "ourColor"), 1, glm::value_ptr(color));
	glUniform1i(glGetUniformLocation(shader, "lightOn"), 1);
	glBindVertexArray(coneVAO);
	glDrawArrays(GL_TRIANGLES, 0, coneVertexCount);
}

// CYLINDRICAL FUSELAGE - Updated colors
void drawFuselage(glm::mat4 view, glm::mat4 proj) {
	glm::vec3 whiteColor = {0.98f, 0.98f, 0.99f};  // Brighter white
	glm::vec3 darkBlueStripe = {0.08f, 0.35f, 0.75f};  // Darker blue like reference
	
	// Main fuselage body - cylindrical sections
	for (int i = 0; i < 24; i++) {
		float x = 5.5f - i * 0.55f;
		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0, 0));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
		model = glm::scale(model, glm::vec3(1.1f, 0.55f, 1.1f));
		drawCylinder(model, view, proj, whiteColor);
	}
	
	// Dark blue stripe along fuselage (larger, more prominent)
	for (int i = 2; i < 22; i++) {
		float x = 5.5f - i * 0.55f;
		// Main stripe
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.35f, 0)), glm::vec3(0.56f, 0.28f, 1.13f)),
			view, proj, darkBlueStripe);
		// Lower accent line
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.05f, 0)), glm::vec3(0.56f, 0.08f, 1.13f)),
			view, proj, darkBlueStripe);
	}
	
	// Nose cone (tapered)
	for (int i = 0; i < 5; i++) {
		float x = 6.0f + i * 0.4f;
		float scale = 1.0f - i * 0.18f;
		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0, 0));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
		model = glm::scale(model, glm::vec3(1.1f * scale, 0.4f, 1.1f * scale));
		drawCylinder(model, view, proj, whiteColor);
	}
	
	// Nose tip cone
	glm::mat4 noseCone = glm::translate(glm::mat4(1.0f), glm::vec3(8.2f, 0, 0));
	noseCone = glm::rotate(noseCone, glm::radians(-90.0f), glm::vec3(0, 0, 1));
	noseCone = glm::scale(noseCone, glm::vec3(0.25f, 0.6f, 0.25f));
	drawCone(noseCone, view, proj, whiteColor);
	
	// Tail taper
	for (int i = 0; i < 6; i++) {
		float x = -7.8f - i * 0.4f;
		float scale = 1.0f - i * 0.12f;
		float yOffset = i * 0.08f;
		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, yOffset, 0));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
		model = glm::scale(model, glm::vec3(1.1f * scale, 0.4f, 1.1f * scale));
		drawCylinder(model, view, proj, whiteColor);
	}
}

// Cockpit windows - More realistic
void drawCockpit(glm::mat4 view, glm::mat4 proj) {
	// Main windshield (larger, more realistic shape)
	glm::vec3 windowTint = {0.15f, 0.25f, 0.4f};  // Dark blue tint
	
	// Center windshield panels
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.1f, 0.5f, 0.25f)), glm::vec3(0.7f, 0.5f, 0.02f)),
		view, proj, windowTint);
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.1f, 0.5f, -0.25f)), glm::vec3(0.7f, 0.5f, 0.02f)),
		view, proj, windowTint);
	
	// Upper windshield
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.0f, 0.72f, 0)), glm::vec3(0.85f, 0.15f, 0.52f)),
		view, proj, windowTint);
	
	// Window frames (metallic grey)
	glm::vec3 frameColor = {0.6f, 0.6f, 0.65f};
	
	// Center frame
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.1f, 0.5f, 0)), glm::vec3(0.68f, 0.48f, 0.03f)),
		view, proj, frameColor);
	
	// Side frames
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.15f, 0.5f, 0.26f)), glm::vec3(0.02f, 0.45f, 0.02f)),
		view, proj, frameColor);
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.15f, 0.5f, -0.26f)), glm::vec3(0.02f, 0.45f, 0.02f)),
		view, proj, frameColor);
	
	// Side windows (pilot visibility)
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(6.7f, 0.42f, 0.55f)), glm::vec3(0.5f, 0.38f, 0.02f)),
		view, proj, windowTint);
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(6.7f, 0.42f, -0.55f)), glm::vec3(0.5f, 0.38f, 0.02f)),
		view, proj, windowTint);
}

// Passenger windows - FIXED: different colors for visibility
void drawWindows(glm::mat4 view, glm::mat4 proj) {
	for (int i = 0; i < 28; i++) {
		float x = 5.0f - i * 0.42f;
		
		for (float z : {0.56f, -0.56f}) {
			// Window frame (dark grey)
			drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.32f, z)), glm::vec3(0.32f, 0.26f, 0.025f)),
				view, proj, glm::vec3(0.2f, 0.2f, 0.22f));
			// Window glass (blue tint - clearly visible)
			float zGlass = z > 0 ? z + 0.013f : z - 0.013f;
			drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.32f, zGlass)), glm::vec3(0.24f, 0.18f, 0.008f)),
				view, proj, glm::vec3(0.4f, 0.55f, 0.75f));
		}
	}
}

// Wings - Better color matching
void drawWings(glm::mat4 view, glm::mat4 proj) {
	glm::vec3 wingColor = {0.96f, 0.96f, 0.97f};  // Light grey-white
	
	for (int side = 0; side < 2; side++) {
		float dir = side == 0 ? 1.0f : -1.0f;
		
		for (int i = 0; i < 8; i++) {
			float z = (1.1f + i * 0.55f) * dir;
			float scale = 1.0f - i * 0.1f;
			float xOffset = i * 0.1f;
			
			drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.3f - xOffset, -0.32f, z)),
				glm::vec3(4.0f * scale, 0.12f, 0.55f)), view, proj, wingColor);
			
			drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(2.2f - xOffset, -0.3f, z)),
				glm::vec3(0.25f, 0.14f, 0.56f)), view, proj, wingColor);
			
			if (i >= 1 && i < 5) {
				drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-1.6f - xOffset, -0.35f, z)),
					glm::vec3(0.7f, 0.08f, 0.5f)), view, proj, glm::vec3(0.9f, 0.9f, 0.91f));
			}
		}
		
		// Winglet with blue tip
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.3f, 5.6f * dir)), glm::vec3(1.2f, 0.9f, 0.12f)),
			view, proj, wingColor);
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-0.7f, 0.75f, 5.6f * dir)), glm::vec3(0.6f, 0.3f, 0.1f)),
			view, proj, glm::vec3(0.08f, 0.35f, 0.75f));  // Blue winglet tip
	}
}

void drawTailWings(glm::mat4 view, glm::mat4 proj) {
	for (int side = 0; side < 2; side++) {
		float dir = side == 0 ? 1.0f : -1.0f;
		for (int i = 0; i < 4; i++) {
			float z = (0.5f + i * 0.4f) * dir;
			float scale = 1.0f - i * 0.2f;
			drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-9.0f + i * 0.1f, 0.55f, z)),
				glm::vec3(1.8f * scale, 0.1f, 0.4f)), view, proj, glm::vec3(0.94f, 0.94f, 0.94f));
		}
	}
}

void drawVerticalStabilizer(glm::mat4 view, glm::mat4 proj) {
	glm::vec3 darkBlue = {0.08f, 0.35f, 0.75f};  // Match fuselage stripe
	
	for (int i = 0; i < 8; i++) {
		float y = 0.7f + i * 0.38f;
		float scale = 1.0f - i * 0.12f;
		float xOffset = i * 0.15f;
		
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-9.2f - xOffset, y, 0)),
			glm::vec3(1.8f * scale, 0.35f, 0.3f)), view, proj, darkBlue);
	}
	
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-10.5f, 3.5f, 0)), glm::vec3(0.7f, 0.35f, 0.28f)),
		view, proj, glm::vec3(0.98f, 0.98f, 0.99f));  // White tip
}

// Engines - Better colors
void drawEngines(glm::mat4 view, glm::mat4 proj) {
	glm::vec3 engineGrey = {0.75f, 0.75f, 0.78f};
	glm::vec3 darkGrey = {0.35f, 0.35f, 0.38f};
	
	for (float z : {3.0f, -3.0f}) {
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, -0.55f, z)), glm::vec3(0.8f, 0.7f, 0.35f)),
			view, proj, engineGrey);
		
		for (int i = 0; i < 5; i++) {
			float x = 1.8f - i * 0.45f;
			glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, -1.0f, z));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
			model = glm::scale(model, glm::vec3(0.6f, 0.45f, 0.6f));
			drawCylinder(model, view, proj, engineGrey);
		}
		
		glm::mat4 intake = glm::translate(glm::mat4(1.0f), glm::vec3(2.4f, -1.0f, z));
		intake = glm::rotate(intake, glm::radians(90.0f), glm::vec3(0, 0, 1));
		intake = glm::scale(intake, glm::vec3(0.5f, 0.12f, 0.5f));
		drawCylinder(intake, view, proj, darkGrey);
		
		glm::mat4 exhaust = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -1.0f, z));
		exhaust = glm::rotate(exhaust, glm::radians(90.0f), glm::vec3(0, 0, 1));
		exhaust = glm::scale(exhaust, glm::vec3(0.45f, 0.15f, 0.45f));
		drawCylinder(exhaust, view, proj, darkGrey);
		
		for (int b = 0; b < 8; b++) {
			float angle = b * 45.0f;
			glm::mat4 blade = glm::translate(glm::mat4(1.0f), glm::vec3(2.45f, -1.0f, z));
			blade = glm::rotate(blade, glm::radians(angle), glm::vec3(1, 0, 0));
			drawCube(glm::scale(glm::translate(blade, glm::vec3(0, 0.15f, 0)), glm::vec3(0.04f, 0.3f, 0.06f)),
				view, proj, glm::vec3(0.55f, 0.55f, 0.6f));
		}
	}
}

// Landing gear with ROTATING wheels
void drawLandingGear(glm::mat4 view, glm::mat4 proj) {
	// NOSE GEAR
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(5.5f, -0.7f, 0)), glm::vec3(0.15f, 1.3f, 0.15f)),
		view, proj, glm::vec3(0.3f, 0.3f, 0.3f));
	
	for (float z : {0.22f, -0.22f}) {
		// Tire with rotation
		glm::mat4 wheel = glm::translate(glm::mat4(1.0f), glm::vec3(5.5f, -1.4f, z));
		wheel = glm::rotate(wheel, glm::radians(90.0f), glm::vec3(1, 0, 0));
		wheel = glm::rotate(wheel, glm::radians(wheelRotation), glm::vec3(0, 1, 0));  // Rotation!
		wheel = glm::scale(wheel, glm::vec3(0.35f, 0.25f, 0.35f));
		drawCylinder(wheel, view, proj, glm::vec3(0.1f, 0.1f, 0.1f));
		
		// Rim with rotation
		wheel = glm::translate(glm::mat4(1.0f), glm::vec3(5.5f, -1.4f, z));
		wheel = glm::rotate(wheel, glm::radians(90.0f), glm::vec3(1, 0, 0));
		wheel = glm::rotate(wheel, glm::radians(wheelRotation), glm::vec3(0, 1, 0));
		wheel = glm::scale(wheel, glm::vec3(0.25f, 0.26f, 0.25f));
		drawCylinder(wheel, view, proj, glm::vec3(0.7f, 0.7f, 0.7f));
		
		// Hub
		wheel = glm::translate(glm::mat4(1.0f), glm::vec3(5.5f, -1.4f, z));
		wheel = glm::rotate(wheel, glm::radians(90.0f), glm::vec3(1, 0, 0));
		wheel = glm::rotate(wheel, glm::radians(wheelRotation), glm::vec3(0, 1, 0));
		wheel = glm::scale(wheel, glm::vec3(0.12f, 0.27f, 0.12f));
		drawCylinder(wheel, view, proj, glm::vec3(0.85f, 0.85f, 0.85f));
	}
	
	for (float z : {1.8f, -1.8f}) {
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -0.85f, z)), glm::vec3(0.25f, 1.1f, 0.25f)),
			view, proj, glm::vec3(0.3f, 0.3f, 0.3f));
		
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -1.45f, z)), glm::vec3(0.4f, 0.18f, 0.85f)),
			view, proj, glm::vec3(0.35f, 0.35f, 0.35f));
		
		for (int w = 0; w < 2; w++) {
			float zOffset = z + (w == 0 ? -0.3f : 0.3f);
			
			// Tire with rotation
			glm::mat4 wheel = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -1.5f, zOffset));
			wheel = glm::rotate(wheel, glm::radians(90.0f), glm::vec3(1, 0, 0));
			wheel = glm::rotate(wheel, glm::radians(wheelRotation), glm::vec3(0, 1, 0));
			wheel = glm::scale(wheel, glm::vec3(0.45f, 0.28f, 0.45f));
			drawCylinder(wheel, view, proj, glm::vec3(0.08f, 0.08f, 0.08f));
			
			// Rim with rotation
			wheel = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -1.5f, zOffset));
			wheel = glm::rotate(wheel, glm::radians(90.0f), glm::vec3(1, 0, 0));
			wheel = glm::rotate(wheel, glm::radians(wheelRotation), glm::vec3(0, 1, 0));
			wheel = glm::scale(wheel, glm::vec3(0.32f, 0.29f, 0.32f));
			drawCylinder(wheel, view, proj, glm::vec3(0.7f, 0.7f, 0.7f));
			
			// Hub with rotation
			wheel = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -1.5f, zOffset));
			wheel = glm::rotate(wheel, glm::radians(90.0f), glm::vec3(1, 0, 0));
			wheel = glm::rotate(wheel, glm::radians(wheelRotation), glm::vec3(0, 1, 0));
			wheel = glm::scale(wheel, glm::vec3(0.15f, 0.3f, 0.15f));
			drawCylinder(wheel, view, proj, glm::vec3(0.85f, 0.85f, 0.85f));
		}
	}
}

// Door - LARGER and more visible
void drawDoor(glm::mat4 view, glm::mat4 proj) {
	glm::mat4 doorBase = glm::translate(glm::mat4(1.0f), glm::vec3(3.5f, -0.15f, 0.57f));
	doorBase = glm::rotate(doorBase, glm::radians(-doorAngle), glm::vec3(0, 1, 0));
	
	// Door body - LARGER SIZE - distinct red-brown color for visibility
	drawCube(glm::scale(doorBase, glm::vec3(1.0f, 1.5f, 0.12f)), view, proj, glm::vec3(0.65f, 0.25f, 0.25f));
	
	// Door window - LARGER blue tint
	drawCube(glm::scale(glm::translate(doorBase, glm::vec3(0, 0.35f, 0.065f)), glm::vec3(0.7f, 0.75f, 0.015f)),
		view, proj, glm::vec3(0.3f, 0.4f, 0.6f));
	
	// Door handle - metallic grey - LARGER
	drawCube(glm::scale(glm::translate(doorBase, glm::vec3(-0.38f, 0, 0.065f)), glm::vec3(0.12f, 0.22f, 0.03f)),
		view, proj, glm::vec3(0.5f, 0.5f, 0.55f));
	
	// Door frame border (makes it stand out more)
	drawCube(glm::scale(doorBase, glm::vec3(1.05f, 1.55f, 0.08f)), view, proj, glm::vec3(0.3f, 0.3f, 0.3f));
}

void drawEmergencyExits(glm::mat4 view, glm::mat4 proj) {
	for (float x : {1.5f, -2.0f, -5.5f}) {
		for (float z : {0.565f, -0.565f}) {
			drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(x, -0.2f, z)), glm::vec3(0.55f, 0.95f, 0.025f)),
				view, proj, glm::vec3(0.85f, 0.25f, 0.25f));
		}
	}
}

// ==================== INTERIOR ====================

// Realistic cabin interior matching reference image (dark grey/black seats)
void drawCabinSeats(glm::mat4 view, glm::mat4 proj) {
	glm::vec3 seatColor = {0.18f, 0.18f, 0.2f};      // Dark grey
	glm::vec3 headrestColor = {0.15f, 0.15f, 0.17f}; // Darker grey
	glm::vec3 frameColor = {0.12f, 0.12f, 0.12f};    // Black frame
	
	for (int row = 0; row < 24; row++) {
		float x = 4.5f - row * 0.45f;
		
		// Left side seats (3 seats: A, B, C)
		for (int seat = 0; seat < 3; seat++) {
			float z = 0.55f + seat * 0.28f;
			
			// Seat cushion
			drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(x, -0.42f, z)), glm::vec3(0.28f, 0.1f, 0.25f)),
				view, proj, seatColor);
			
			// Seat back
			glm::mat4 back = glm::translate(glm::mat4(1.0f), glm::vec3(x - 0.12f, -0.18f, z));
			back = glm::rotate(back, glm::radians(-8.0f), glm::vec3(0, 0, 1));
			drawCube(glm::scale(back, glm::vec3(0.08f, 0.52f, 0.25f)), view, proj, seatColor);
			
			// Headrest
			drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(x - 0.14f, 0.18f, z)), glm::vec3(0.07f, 0.18f, 0.2f)),
				view, proj, headrestColor);
			
			// Armrests
			if (seat < 2) {
				drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(x, -0.36f, z + 0.14f)), glm::vec3(0.25f, 0.045f, 0.06f)),
					view, proj, frameColor);
			}
		}
		
		// Right side seats (3 seats: D, E, F)
		for (int seat = 0; seat < 3; seat++) {
			float z = -0.55f - seat * 0.28f;
			
			drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(x, -0.42f, z)), glm::vec3(0.28f, 0.1f, 0.25f)),
				view, proj, seatColor);
			
			glm::mat4 back = glm::translate(glm::mat4(1.0f), glm::vec3(x - 0.12f, -0.18f, z));
			back = glm::rotate(back, glm::radians(-8.0f), glm::vec3(0, 0, 1));
			drawCube(glm::scale(back, glm::vec3(0.08f, 0.52f, 0.25f)), view, proj, seatColor);
			
			drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(x - 0.14f, 0.18f, z)), glm::vec3(0.07f, 0.18f, 0.2f)),
				view, proj, headrestColor);
			
			if (seat < 2) {
				drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(x, -0.36f, z - 0.14f)), glm::vec3(0.25f, 0.045f, 0.06f)),
					view, proj, frameColor);
			}
		}
	}
}

// Aisle carpet
void drawAisle(glm::mat4 view, glm::mat4 proj) {
	// Main aisle
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, -0.54f, 0)), glm::vec3(15.0f, 0.01f, 0.45f)),
		view, proj, glm::vec3(0.35f, 0.3f, 0.38f));
	
	// Floor strips
	for (int i = 0; i < 30; i++) {
		float x = 5.0f - i * 0.42f;
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(x, -0.535f, 0)), glm::vec3(0.08f, 0.005f, 0.4f)),
			view, proj, glm::vec3(0.4f, 0.35f, 0.42f));
	}
}

// Overhead compartments
void drawOverheadCompartments(glm::mat4 view, glm::mat4 proj) {
	for (int i = 0; i < 24; i++) {
		float x = 4.5f - i * 0.45f;
		
		for (float z : {0.62f, -0.62f}) {
			drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.85f, z)), glm::vec3(0.4f, 0.3f, 0.38f)),
				view, proj, glm::vec3(0.9f, 0.9f, 0.92f));
			drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.72f, z)), glm::vec3(0.4f, 0.03f, 0.39f)),
				view, proj, glm::vec3(0.8f, 0.8f, 0.82f));
		}
	}
}

// Curved cabin ceiling
void drawCabinCeiling(glm::mat4 view, glm::mat4 proj) {
	// Main ceiling panel
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 1.05f, 0)), glm::vec3(15.0f, 0.04f, 1.3f)),
		view, proj, glm::vec3(0.94f, 0.94f, 0.95f));
	
	glm::vec3 lightCol = lightOn ? glm::vec3(1.0f, 0.98f, 0.92f) : glm::vec3(0.3f, 0.3f, 0.32f);
	
	for (int i = 0; i < 28; i++) {
		float x = 5.0f - i * 0.45f;
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(x, 1.03f, 0)), glm::vec3(0.32f, 0.015f, 0.25f)),
			view, proj, lightCol);
		
		if (lightOn) {
			for (float z : {0.58f, -0.58f}) {
				drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.99f, z)), glm::vec3(0.25f, 0.015f, 0.08f)),
					view, proj, glm::vec3(0.5f, 0.7f, 1.0f));
			}
		}
	}
}

// Cabin floor
void drawCabinFloor(glm::mat4 view, glm::mat4 proj) {
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, -0.55f, 0)), glm::vec3(15.0f, 0.02f, 1.1f)),
		view, proj, glm::vec3(0.45f, 0.4f, 0.38f));
}

// Galley (front service area)
void drawGalley(glm::mat4 view, glm::mat4 proj) {
	for (float z : {0.62f, -0.62f}) {
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(5.5f, 0, z)), glm::vec3(1.4f, 1.15f, 0.42f)),
			view, proj, glm::vec3(0.78f, 0.78f, 0.8f));
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(5.5f, 0.55f, z)), glm::vec3(1.35f, 0.05f, 0.4f)),
			view, proj, glm::vec3(0.88f, 0.88f, 0.9f));
	}
}

// Lavatory (rear)
void drawLavatory(glm::mat4 view, glm::mat4 proj) {
	for (float z : {0.55f, -0.55f}) {
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-9.5f, 0, z)), glm::vec3(1.4f, 1.35f, 0.48f)),
			view, proj, glm::vec3(0.9f, 0.9f, 0.92f));
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-8.85f, -0.1f, z)), glm::vec3(0.05f, 1.05f, 0.42f)),
			view, proj, glm::vec3(0.7f, 0.7f, 0.72f));
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-8.83f, 0.38f, z)), glm::vec3(0.02f, 0.1f, 0.12f)),
			view, proj, glm::vec3(0.2f, 0.85f, 0.2f));
	}
}

// ==================== COCKPIT INTERIOR - REALISTIC ====================
void drawCockpitInterior(glm::mat4 view, glm::mat4 proj) {
	// Color scheme
	glm::vec3 blackPanel = {0.05f, 0.05f, 0.05f};
	glm::vec3 darkGrey = {0.12f, 0.12f, 0.12f};
	glm::vec3 screenBlue = {0.15f, 0.3f, 0.5f};
	glm::vec3 screenGreen = {0.1f, 0.25f, 0.15f};
	glm::vec3 skyBlue = {0.4f, 0.7f, 1.0f};
	glm::vec3 groundBrown = {0.6f, 0.45f, 0.3f};
	glm::vec3 buttonWhite = {0.85f, 0.85f, 0.85f};
	glm::vec3 seatBlack = {0.08f, 0.08f, 0.1f};
	
	// Pilot seats - realistic aircraft seats
	for (float z : {0.32f, -0.32f}) {
		// Main seat body
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(6.8f, -0.28f, z)), glm::vec3(0.35f, 0.58f, 0.35f)),
			view, proj, seatBlack);
		// Seat cushion
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(6.85f, -0.42f, z)), glm::vec3(0.3f, 0.12f, 0.3f)),
			view, proj, glm::vec3(0.1f, 0.1f, 0.12f));
		// Headrest
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(6.72f, 0.1f, z)), glm::vec3(0.08f, 0.18f, 0.2f)),
			view, proj, seatBlack);
		// Armrests
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(6.85f, -0.3f, z + 0.18f)), glm::vec3(0.28f, 0.05f, 0.04f)),
			view, proj, seatBlack);
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(6.85f, -0.3f, z - 0.18f)), glm::vec3(0.28f, 0.05f, 0.04f)),
			view, proj, seatBlack);
	}
	
	// Main instrument panel (glareshield)
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.35f, -0.02f, 0)), glm::vec3(0.55f, 0.65f, 0.95f)),
		view, proj, darkGrey);
	
	// Top of glareshield
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.35f, 0.3f, 0)), glm::vec3(0.5f, 0.08f, 0.9f)),
		view, proj, blackPanel);
	
	// PRIMARY FLIGHT DISPLAYS (PFD) - Captain and First Officer
	for (float z : {0.28f, -0.28f}) {
		// Screen bezel
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.32f, 0.08f, z)), glm::vec3(0.03f, 0.28f, 0.28f)),
			view, proj, blackPanel);
		
		// Screen background
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.30f, 0.08f, z)), glm::vec3(0.01f, 0.24f, 0.24f)),
			view, proj, screenBlue);
		
		// Artificial Horizon - Sky (top half)
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.295f, 0.14f, z)), glm::vec3(0.008f, 0.12f, 0.2f)),
			view, proj, skyBlue);
		
		// Artificial Horizon - Ground (bottom half)
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.295f, 0.02f, z)), glm::vec3(0.008f, 0.12f, 0.2f)),
			view, proj, groundBrown);
		
		// Horizon line
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.29f, 0.08f, z)), glm::vec3(0.005f, 0.01f, 0.18f)),
			view, proj, glm::vec3(1.0f, 1.0f, 1.0f));
		
		// Pitch ladder indicators
		for (int p = -1; p <= 1; p++) {
			if (p != 0) {
				drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.29f, 0.08f + p * 0.04f, z)), 
					glm::vec3(0.005f, 0.005f, 0.1f)),
					view, proj, glm::vec3(1.0f, 1.0f, 1.0f));
			}
		}
		
		// Airspeed indicator (left)
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.295f, 0.08f, z - 0.1f)), glm::vec3(0.008f, 0.15f, 0.04f)),
			view, proj, glm::vec3(0.2f, 0.9f, 0.2f));
		
		// Altitude indicator (right)
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.295f, 0.08f, z + 0.1f)), glm::vec3(0.008f, 0.15f, 0.04f)),
			view, proj, glm::vec3(0.2f, 0.9f, 0.2f));
		
		// Heading indicator (bottom)
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.295f, -0.08f, z)), glm::vec3(0.008f, 0.03f, 0.18f)),
			view, proj, glm::vec3(1.0f, 1.0f, 1.0f));
	}
	
	// NAVIGATION DISPLAY (ND) - Center
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.32f, 0.08f, 0)), glm::vec3(0.03f, 0.22f, 0.22f)),
		view, proj, blackPanel);
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.30f, 0.08f, 0)), glm::vec3(0.01f, 0.18f, 0.18f)),
		view, proj, screenGreen);
	
	// Compass rose on ND
	for (int i = 0; i < 12; i++) {
		float angle = i * 30.0f;
		float rad = glm::radians(angle);
		float cx = 0.07f * cos(rad);
		float cy = 0.07f * sin(rad);
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.295f, 0.08f + cy, cx)), 
			glm::vec3(0.005f, 0.008f, 0.008f)),
			view, proj, glm::vec3(1.0f, 1.0f, 1.0f));
	}
	
	// EICAS (Engine Indicating) Display - Lower center
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.32f, -0.2f, 0)), glm::vec3(0.03f, 0.18f, 0.35f)),
		view, proj, blackPanel);
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.30f, -0.2f, 0)), glm::vec3(0.01f, 0.14f, 0.3f)),
		view, proj, screenGreen);
	
	// Engine gauges
	for (float ez : {0.08f, -0.08f}) {
		// N1 gauge (circular)
		for (int seg = 0; seg < 12; seg++) {
			float ang = seg * 30.0f;
			float r = glm::radians(ang);
			drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.295f, -0.2f + 0.04f * cos(r), ez + 0.04f * sin(r))), 
				glm::vec3(0.005f, 0.006f, 0.006f)),
				view, proj, glm::vec3(0.3f, 1.0f, 0.3f));
		}
	}
	
	// CENTER PEDESTAL - Throttle Quadrant
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(6.95f, -0.35f, 0)), glm::vec3(0.5f, 0.25f, 0.35f)),
		view, proj, darkGrey);
	
	// Throttle levers (dual engine)
	for (float z : {0.08f, -0.08f}) {
		// Lever shaft
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.0f, -0.18f, z)), glm::vec3(0.08f, 0.22f, 0.05f)),
			view, proj, blackPanel);
		// Lever handle
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.0f, -0.05f, z)), glm::vec3(0.06f, 0.06f, 0.04f)),
			view, proj, glm::vec3(0.3f, 0.3f, 0.3f));
	}
	
	// OVERHEAD PANEL
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.15f, 0.62f, 0)), glm::vec3(0.8f, 0.08f, 0.9f)),
		view, proj, darkGrey);
	
	// Overhead switches (realistic layout)
	for (int row = 0; row < 5; row++) {
		for (int col = 0; col < 10; col++) {
			float x = 7.35f - row * 0.12f;
			float z = -0.4f + col * 0.09f;
			
			// Switch base
			drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.59f, z)), glm::vec3(0.035f, 0.02f, 0.03f)),
				view, proj, buttonWhite);
			
			// Some switches have lights
			if ((row + col) % 3 == 0) {
				drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.605f, z)), glm::vec3(0.015f, 0.005f, 0.015f)),
					view, proj, glm::vec3(0.2f, 1.0f, 0.2f));
			}
		}
	}
	
	// CONTROL YOKES (realistic aircraft yoke)
	for (float z : {0.32f, -0.32f}) {
		// Yoke column
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.15f, -0.25f, z)), glm::vec3(0.08f, 0.35f, 0.08f)),
			view, proj, glm::vec3(0.15f, 0.15f, 0.15f));
		
		// Yoke wheel (U-shape)
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.08f, -0.08f, z)), glm::vec3(0.06f, 0.16f, 0.24f)),
			view, proj, glm::vec3(0.18f, 0.18f, 0.18f));
		
		// Yoke grips
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.06f, -0.02f, z + 0.11f)), glm::vec3(0.05f, 0.1f, 0.06f)),
			view, proj, blackPanel);
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.06f, -0.02f, z - 0.11f)), glm::vec3(0.05f, 0.1f, 0.06f)),
			view, proj, blackPanel);
		
		// Trim wheel on yoke
		for (int spoke = 0; spoke < 4; spoke++) {
			float ang = spoke * 90.0f;
			glm::mat4 trimWheel = glm::translate(glm::mat4(1.0f), glm::vec3(7.06f, -0.02f, z));
			trimWheel = glm::rotate(trimWheel, glm::radians(ang), glm::vec3(0, 0, 1));
			drawCube(glm::scale(glm::translate(trimWheel, glm::vec3(0, 0.035f, 0)), glm::vec3(0.03f, 0.04f, 0.03f)),
				view, proj, glm::vec3(0.25f, 0.25f, 0.25f));
		}
	}
	
	// RUDDER PEDALS
	for (float z : {0.28f, -0.28f}) {
		// Pedal base
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.45f, -0.52f, z)), glm::vec3(0.15f, 0.08f, 0.11f)),
			view, proj, glm::vec3(0.25f, 0.25f, 0.25f));
		// Pedal face
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.48f, -0.48f, z)), glm::vec3(0.02f, 0.12f, 0.1f)),
			view, proj, glm::vec3(0.3f, 0.3f, 0.3f));
	}
	
	// SIDE PANELS (realistic texture)
	for (float z : {0.52f, -0.52f}) {
		drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(6.95f, 0.08f, z)), glm::vec3(0.58f, 0.5f, 0.12f)),
			view, proj, darkGrey);
		
		// Side panel buttons/controls
		for (int i = 0; i < 8; i++) {
			drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.05f - i * 0.08f, 0.08f, z)), 
				glm::vec3(0.03f, 0.03f, 0.02f)),
				view, proj, buttonWhite);
		}
	}
	
	// COCKPIT FLOOR
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(6.95f, -0.55f, 0)), glm::vec3(1.2f, 0.02f, 0.95f)),
		view, proj, glm::vec3(0.2f, 0.2f, 0.22f));
	
	// COCKPIT DIVIDER WALL
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(6.28f, 0, 0)), glm::vec3(0.08f, 1.2f, 1.0f)),
		view, proj, glm::vec3(0.7f, 0.7f, 0.73f));
	
	// COCKPIT DOOR (reinforced)
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(6.30f, -0.1f, 0)), glm::vec3(0.04f, 1.0f, 0.5f)),
		view, proj, glm::vec3(0.5f, 0.5f, 0.52f));
	
	// Door lock indicator
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(6.28f, 0.3f, 0)), glm::vec3(0.02f, 0.08f, 0.12f)),
		view, proj, glm::vec3(1.0f, 0.2f, 0.2f));
	
	// WINDSHIELD FRAME (from inside)
	drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(7.55f, 0.4f, 0)), glm::vec3(0.03f, 0.5f, 0.9f)),
		view, proj, glm::vec3(0.15f, 0.2f, 0.25f));
}

int main() {
	printControls();

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Realistic Airplane - Cylindrical Design", NULL, NULL);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glEnable(GL_DEPTH_TEST);

	float vertices[] = {
		-0.5f,-0.5f,-0.5f, 0.5f,-0.5f,-0.5f, 0.5f,0.5f,-0.5f, 0.5f,0.5f,-0.5f, -0.5f,0.5f,-0.5f, -0.5f,-0.5f,-0.5f,
		-0.5f,-0.5f, 0.5f, 0.5f,-0.5f, 0.5f, 0.5f,0.5f, 0.5f, 0.5f,0.5f, 0.5f, -0.5f,0.5f, 0.5f, -0.5f,-0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f, -0.5f,-0.5f,-0.5f, -0.5f,-0.5f, 0.5f, -0.5f,0.5f, 0.5f,
		 0.5f, 0.5f, 0.5f, 0.5f, 0.5f,-0.5f, 0.5f,-0.5f,-0.5f, 0.5f,-0.5f,-0.5f, 0.5f,-0.5f, 0.5f, 0.5f,0.5f, 0.5f,
		-0.5f,-0.5f,-0.5f, 0.5f,-0.5f,-0.5f, 0.5f,-0.5f, 0.5f, 0.5f,-0.5f, 0.5f, -0.5f,-0.5f, 0.5f, -0.5f,-0.5f,-0.5f,
		-0.5f, 0.5f,-0.5f, 0.5f, 0.5f,-0.5f, 0.5f, 0.5f, 0.5f, 0.5f,0.5f, 0.5f, -0.5f,0.5f, 0.5f, -0.5f,0.5f,-0.5f
	};

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	createCylinderVAO();
	createConeVAO();
	createDiskVAO();

	shader = glCreateProgram();
	std::string vCode = loadShader("vertex.vs"), fCode = loadShader("fragment.fs");
	const char* vSrc = vCode.c_str(), * fSrc = fCode.c_str();

	unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vSrc, NULL);
	glCompileShader(vs);

	int success;
	char infoLog[512];
	glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vs, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fSrc, NULL);
	glCompileShader(fs);

	glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fs, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	glAttachShader(shader, vs);
	glAttachShader(shader, fs);
	glLinkProgram(shader);

	glGetProgramiv(shader, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vs);
	glDeleteShader(fs);

	float lastT = 0.0f;

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	while (!glfwWindowShouldClose(window)) {
		float now = (float)glfwGetTime();
		float dt = now - lastT;
		lastT = now;
		processInput(window, dt);

		glClearColor(0.55f, 0.82f, 0.95f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shader);

		glm::vec3 f;
		f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		f.y = sin(glm::radians(pitch));
		f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		camFront = glm::normalize(f);

		glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
		view = glm::rotate(view, glm::radians(roll), camFront);
		glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 150.0f);

		// Ground & Runway (only when not in cockpit/interior view)
		if (!showCockpit && !showInterior) {
			drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0, -2.0f, 0)), glm::vec3(120, 0.1f, 120)),
				view, proj, glm::vec3(0.35f, 0.38f, 0.35f));

			drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0, -1.95f, 0)), glm::vec3(80, 0.02f, 8)),
				view, proj, glm::vec3(0.25f, 0.25f, 0.28f));

			for (int i = -12; i < 12; i++) {
				drawCube(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(i * 2.8f, -1.93f, 0)), glm::vec3(1.8f, 0.01f, 0.3f)),
					view, proj, glm::vec3(0.95f, 0.95f, 0.1f));
			}
		}

		// Draw exterior (not in interior/cockpit mode)
		if (!showInterior && !showCockpit) {
			drawFuselage(view, proj);
			drawWindows(view, proj);
			drawCockpit(view, proj);
			drawDoor(view, proj);
			drawEmergencyExits(view, proj);
			drawWings(view, proj);
			drawTailWings(view, proj);
			drawVerticalStabilizer(view, proj);
			drawEngines(view, proj);
			drawLandingGear(view, proj);
		}

		// Draw cabin interior
		if (showInterior) {
			drawCabinFloor(view, proj);
			drawAisle(view, proj);
			drawCabinSeats(view, proj);
			drawOverheadCompartments(view, proj);
			drawCabinCeiling(view, proj);
			drawGalley(view, proj);
			drawLavatory(view, proj);
		}
		
		// Draw cockpit interior
		if (showCockpit) {
			drawCockpitInterior(view, proj);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteVertexArrays(1, &cylinderVAO);
	glDeleteBuffers(1, &cylinderVBO);
	glDeleteVertexArrays(1, &coneVAO);
	glDeleteBuffers(1, &coneVBO);
	glDeleteVertexArrays(1, &diskVAO);
	glDeleteBuffers(1, &diskVBO);
	glDeleteProgram(shader);

	glfwTerminate();
	return 0;
}

