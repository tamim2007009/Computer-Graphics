#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// Animation state
struct CraneState {
    float positionX = 0.0f;
    float wheelRotation = 0.0f;
    float boomAngle = 45.0f;
    float hookHeight = 0.35f;
    float wholeObjectRotation = 0.0f;  // Rotation in radians
    bool hookMovingDown = true;
    bool boomRotating = false;
    bool autoMoving = false;
} craneState;

// Rotation limits (in radians)
const float MAX_ROTATION = 0.785f;  // 45 degrees (π/4)
const float MIN_ROTATION = -0.785f; // -45 degrees

// Colors
const float YELLOW[3] = {1.0f, 0.9f, 0.0f};
const float LIGHT_BLUE[3] = {0.6f, 0.8f, 1.0f};
const float DARK_GRAY[3] = {0.2f, 0.2f, 0.2f};
const float GRAY[3] = {0.5f, 0.5f, 0.5f};
const float BLACK[3] = {0.1f, 0.1f, 0.1f};
const float SILVER[3] = {0.7f, 0.7f, 0.7f};

std::string readShaderSource(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cout << "Failed to open shader file: " << filePath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

inline void addVertex(std::vector<float>& vertices, float x, float y, const float* color) {
    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(color[0]);
    vertices.push_back(color[1]);
    vertices.push_back(color[2]);
}

inline void addTriangle(std::vector<float>& vertices, float x1, float y1, float x2, float y2, float x3, float y3, const float* color) {
    addVertex(vertices, x1, y1, color);
    addVertex(vertices, x2, y2, color);
    addVertex(vertices, x3, y3, color);
}

inline void addQuad(std::vector<float>& vertices, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, const float* color) {
    addTriangle(vertices, x1, y1, x2, y2, x3, y3, color);
    addTriangle(vertices, x1, y1, x3, y3, x4, y4, color);
}

void createTransformMatrix(float* matrix, float translateX, float translateY, float rotateAngle = 0.0f) {
    float cosA = cos(rotateAngle);
    float sinA = sin(rotateAngle);
    
    matrix[0] = cosA;
    matrix[1] = sinA;
    matrix[4] = -sinA;
    matrix[5] = cosA;
    matrix[12] = translateX;
    matrix[13] = translateY;
    
    matrix[2] = matrix[3] = matrix[6] = matrix[7] = 0.0f;
    matrix[8] = matrix[9] = 0.0f;
    matrix[10] = 1.0f;
    matrix[11] = 0.0f;
    matrix[14] = 0.0f;
    matrix[15] = 1.0f;
}

void createTransformMatrixWithPivot(float* matrix, float translateX, float translateY, float rotateAngle, float pivotX, float pivotY) {
    float cosA = cos(rotateAngle);
    float sinA = sin(rotateAngle);
    
    float tx = translateX + pivotX - (cosA * pivotX - sinA * pivotY);
    float ty = translateY + pivotY - (sinA * pivotX + cosA * pivotY);
    
    matrix[0] = cosA;
    matrix[1] = sinA;
    matrix[4] = -sinA;
    matrix[5] = cosA;
    matrix[12] = tx;
    matrix[13] = ty;
    
    matrix[2] = matrix[3] = matrix[6] = matrix[7] = 0.0f;
    matrix[8] = matrix[9] = 0.0f;
    matrix[10] = 1.0f;
    matrix[11] = 0.0f;
    matrix[14] = 0.0f;
    matrix[15] = 1.0f;
}

std::vector<float> getCraneBodyVertices() {
    std::vector<float> vertices;
    vertices.reserve(400);
    
    float darkYellow[3] = {0.85f, 0.75f, 0.0f};
    float windowFrame[3] = {0.2f, 0.2f, 0.2f};
    float metalGray[3] = {0.4f, 0.4f, 0.4f};
    float white[3] = {1.0f, 1.0f, 1.0f};
    
    // Main chassis and reinforcement
    addQuad(vertices, -0.7f, -0.32f, 0.7f, -0.32f, 0.7f, -0.18f, -0.7f, -0.18f, YELLOW);
    addQuad(vertices, -0.7f, -0.32f, -0.68f, -0.32f, -0.68f, -0.18f, -0.7f, -0.18f, darkYellow);
    addQuad(vertices, 0.68f, -0.32f, 0.7f, -0.32f, 0.7f, -0.18f, 0.68f, -0.18f, darkYellow);
    
    // Cabin
    addQuad(vertices, -0.7f, -0.18f, -0.2f, -0.18f, -0.2f, 0.25f, -0.7f, 0.25f, YELLOW);
    addQuad(vertices, -0.7f, 0.25f, -0.55f, 0.25f, -0.55f, 0.35f, -0.7f, 0.35f, YELLOW);
    addQuad(vertices, -0.55f, 0.25f, -0.2f, 0.25f, -0.2f, 0.3f, -0.55f, 0.35f, YELLOW);
    addQuad(vertices, -0.7f, -0.18f, -0.7f, 0.35f, -0.72f, 0.33f, -0.72f, -0.18f, darkYellow);
    
    // Bumpers
    addQuad(vertices, -0.72f, -0.32f, -0.7f, -0.32f, -0.7f, -0.18f, -0.72f, -0.18f, darkYellow);
    addQuad(vertices, 0.7f, -0.32f, 0.72f, -0.32f, 0.72f, -0.18f, 0.7f, -0.18f, darkYellow);
    
    // Windows
    addQuad(vertices, -0.69f, 0.03f, -0.63f, 0.03f, -0.63f, 0.23f, -0.69f, 0.23f, LIGHT_BLUE);
    addQuad(vertices, -0.695f, 0.025f, -0.685f, 0.025f, -0.685f, 0.235f, -0.695f, 0.235f, windowFrame);
    addQuad(vertices, -0.6f, 0.03f, -0.45f, 0.03f, -0.45f, 0.23f, -0.6f, 0.23f, LIGHT_BLUE);
    addQuad(vertices, -0.42f, 0.03f, -0.23f, 0.03f, -0.23f, 0.23f, -0.42f, 0.23f, LIGHT_BLUE);
    
    // Door
    addQuad(vertices, -0.5f, -0.18f, -0.35f, -0.18f, -0.35f, 0.15f, -0.5f, 0.15f, darkYellow);
    addQuad(vertices, -0.48f, -0.15f, -0.37f, -0.15f, -0.37f, -0.13f, -0.48f, -0.13f, windowFrame);
    
    // Outriggers
    addQuad(vertices, -0.68f, -0.18f, -0.62f, -0.18f, -0.62f, -0.35f, -0.68f, -0.35f, YELLOW);
    addQuad(vertices, 0.62f, -0.18f, 0.68f, -0.18f, 0.68f, -0.35f, 0.62f, -0.35f, YELLOW);
    addQuad(vertices, -0.72f, -0.35f, -0.58f, -0.35f, -0.58f, -0.33f, -0.72f, -0.33f, metalGray);
    addQuad(vertices, 0.58f, -0.35f, 0.72f, -0.35f, 0.72f, -0.33f, 0.58f, -0.33f, metalGray);
    
    // Details
    addQuad(vertices, -0.4f, -0.3f, -0.35f, -0.3f, -0.35f, -0.28f, -0.4f, -0.28f, metalGray);
    addQuad(vertices, -0.3f, 0.05f, -0.24f, 0.05f, -0.24f, 0.12f, -0.3f, 0.12f, white);
    addQuad(vertices, -0.7f, -0.05f, -0.68f, -0.05f, -0.68f, 0.02f, -0.7f, 0.02f, windowFrame);
    
    return vertices;
}

std::vector<float> getTurretVertices() {
    std::vector<float> vertices;
    vertices.reserve(200);
    
    float darkYellow[3] = {0.85f, 0.75f, 0.0f};
    float orange[3] = {0.9f, 0.5f, 0.0f};
    float metalGray[3] = {0.4f, 0.4f, 0.4f};
    
    // Platform
    addQuad(vertices, -0.2f, -0.18f, 0.5f, -0.18f, 0.5f, 0.18f, -0.2f, 0.18f, YELLOW);
    addQuad(vertices, -0.22f, -0.18f, -0.2f, -0.18f, -0.2f, 0.18f, -0.22f, 0.18f, darkYellow);
    addQuad(vertices, 0.5f, -0.18f, 0.52f, -0.18f, 0.52f, 0.18f, 0.5f, 0.18f, darkYellow);
    
    // Counterweight
    addQuad(vertices, 0.5f, -0.18f, 0.8f, -0.18f, 0.8f, 0.2f, 0.5f, 0.2f, YELLOW);
    addQuad(vertices, 0.55f, 0.2f, 0.75f, 0.2f, 0.75f, 0.35f, 0.55f, 0.35f, YELLOW);
    addQuad(vertices, 0.56f, 0.22f, 0.74f, 0.22f, 0.74f, 0.25f, 0.56f, 0.25f, orange);
    addQuad(vertices, 0.56f, 0.28f, 0.74f, 0.28f, 0.74f, 0.31f, 0.56f, 0.31f, orange);
    
    // Exhaust
    addQuad(vertices, 0.48f, 0.1f, 0.5f, 0.1f, 0.5f, 0.25f, 0.48f, 0.25f, metalGray);
    
    return vertices;
}

std::vector<float> getBoomVertices() {
    std::vector<float> vertices;
    vertices.reserve(150);
    
    float darkYellow[3] = {0.85f, 0.75f, 0.0f};
    float metalGray[3] = {0.5f, 0.5f, 0.5f};
    float hydraulicBlue[3] = {0.2f, 0.3f, 0.5f};
    float red[3] = {0.8f, 0.1f, 0.1f};
    
    // Pivot housing
    addQuad(vertices, -0.08f, -0.05f, 0.08f, -0.05f, 0.08f, 0.11f, -0.08f, 0.11f, YELLOW);
    addQuad(vertices, -0.09f, -0.06f, -0.07f, -0.06f, -0.07f, 0.12f, -0.09f, 0.12f, darkYellow);
    addQuad(vertices, 0.07f, -0.06f, 0.09f, -0.06f, 0.09f, 0.12f, 0.07f, 0.12f, darkYellow);
    
    // Main boom
    addQuad(vertices, -0.04f, 0.11f, 0.04f, 0.11f, 0.54f, 0.63f, 0.48f, 0.65f, YELLOW);
    addQuad(vertices, -0.045f, 0.10f, -0.03f, 0.10f, 0.47f, 0.64f, 0.45f, 0.66f, darkYellow);
    addQuad(vertices, -0.02f, 0.11f, 0.02f, 0.11f, 0.52f, 0.63f, 0.5f, 0.63f, metalGray);
    
    // Hydraulic & pulley
    addQuad(vertices, -0.01f, -0.02f, 0.01f, -0.02f, 0.25f, 0.33f, 0.23f, 0.33f, hydraulicBlue);
    addQuad(vertices, 0.48f, 0.62f, 0.54f, 0.62f, 0.54f, 0.66f, 0.48f, 0.66f, red);
    
    return vertices;
}

std::vector<float> getCableAndHookVertices(float hookY) {
    std::vector<float> vertices;
    vertices.reserve(50);
    
    addQuad(vertices, 0.485f, 0.59f, 0.505f, 0.59f, 0.505f, hookY, 0.485f, hookY, BLACK);
    addQuad(vertices, 0.47f, hookY, 0.53f, hookY, 0.53f, hookY - 0.07f, 0.47f, hookY - 0.07f, SILVER);
    addQuad(vertices, 0.47f, hookY - 0.07f, 0.53f, hookY - 0.07f, 0.55f, hookY - 0.09f, 0.49f, hookY - 0.09f, SILVER);
    
    return vertices;
}

std::vector<float> getWheelVertices(float rotation) {
    std::vector<float> vertices;
    vertices.reserve(2000);
    
    const float wheelPositions[4] = {-0.55f, -0.15f, 0.15f, 0.55f};
    const float wheelY = -0.35f, wheelRadius = 0.09f, rimRadius = 0.045f;
    const int tireSegments = 24, rimSegments = 20;
    
    float brightSilver[3] = {0.75f, 0.75f, 0.75f};
    float orangeHub[3] = {0.9f, 0.5f, 0.1f};
    float brightWhite[3] = {0.95f, 0.95f, 0.95f};
    float redBolt[3] = {0.9f, 0.1f, 0.1f};
    
    for (int w = 0; w < 4; w++) {
        float centerX = wheelPositions[w];
        
        // Tire
        for (int i = 0; i < tireSegments; i++) {
            float angle1 = 2.0f * 3.14159f * i / tireSegments + rotation;
            float angle2 = 2.0f * 3.14159f * (i + 1) / tireSegments + rotation;
            float tireColor[3] = {i % 2 == 0 ? 0.15f : 0.25f, i % 2 == 0 ? 0.15f : 0.25f, i % 2 == 0 ? 0.15f : 0.25f};
            
            addTriangle(vertices, centerX, wheelY,
                centerX + wheelRadius * cos(angle1), wheelY + wheelRadius * sin(angle1),
                centerX + wheelRadius * cos(angle2), wheelY + wheelRadius * sin(angle2), tireColor);
        }
        
        // Tread pattern
        for (int i = 0; i < tireSegments; i++) {
            float angle1 = 2.0f * 3.14159f * i / tireSegments + rotation;
            float angle2 = 2.0f * 3.14159f * (i + 1) / tireSegments + rotation;
            float innerRadius = wheelRadius * 0.85f, outerRadius = wheelRadius * 0.95f;
            float treadColor[3] = {i % 4 == 0 ? 0.9f : 0.08f, i % 4 == 0 ? 0.8f : 0.08f, i % 4 == 0 ? 0.1f : 0.08f};
            
            addQuad(vertices,
                centerX + innerRadius * cos(angle1), wheelY + innerRadius * sin(angle1),
                centerX + outerRadius * cos(angle1), wheelY + outerRadius * sin(angle1),
                centerX + outerRadius * cos(angle2), wheelY + outerRadius * sin(angle2),
                centerX + innerRadius * cos(angle2), wheelY + innerRadius * sin(angle2), treadColor);
        }
        
        // Rim
        for (int i = 0; i < rimSegments; i++) {
            float angle1 = 2.0f * 3.14159f * i / rimSegments + rotation;
            float angle2 = 2.0f * 3.14159f * (i + 1) / rimSegments + rotation;
            addTriangle(vertices, centerX, wheelY,
                centerX + rimRadius * cos(angle1), wheelY + rimRadius * sin(angle1),
                centerX + rimRadius * cos(angle2), wheelY + rimRadius * sin(angle2), brightSilver);
        }
        
        // Hub
        for (int i = 0; i < 12; i++) {
            float angle1 = 2.0f * 3.14159f * i / 12.0f + rotation;
            float angle2 = 2.0f * 3.14159f * (i + 1) / 12.0f + rotation;
            addTriangle(vertices, centerX, wheelY,
                centerX + 0.02f * cos(angle1), wheelY + 0.02f * sin(angle1),
                centerX + 0.02f * cos(angle2), wheelY + 0.02f * sin(angle2), orangeHub);
        }
        
        // Spokes
        for (int spoke = 0; spoke < 5; spoke++) {
            float spokeAngle = (2.0f * 3.14159f / 5.0f) * spoke + rotation;
            float spokeWidth = 0.01f, perpAngle = spokeAngle + 3.14159f / 2.0f;
            float x1 = centerX + 0.02f * cos(spokeAngle), y1 = wheelY + 0.02f * sin(spokeAngle);
            float x2 = centerX + rimRadius * 0.85f * cos(spokeAngle), y2 = wheelY + rimRadius * 0.85f * sin(spokeAngle);
            
            addQuad(vertices,
                x1 + spokeWidth * cos(perpAngle), y1 + spokeWidth * sin(perpAngle),
                x2 + spokeWidth * cos(perpAngle), y2 + spokeWidth * sin(perpAngle),
                x2 - spokeWidth * cos(perpAngle), y2 - spokeWidth * sin(perpAngle),
                x1 - spokeWidth * cos(perpAngle), y1 - spokeWidth * sin(perpAngle), brightWhite);
        }
        
        // Bolts
        for (int bolt = 0; bolt < 5; bolt++) {
            float boltAngle = (2.0f * 3.14159f / 5.0f) * bolt + rotation;
            float boltX = centerX + rimRadius * 0.6f * cos(boltAngle);
            float boltY = wheelY + rimRadius * 0.6f * sin(boltAngle);
            
            for (int i = 0; i < 8; i++) {
                float angle1 = 2.0f * 3.14159f * i / 8.0f;
                float angle2 = 2.0f * 3.14159f * (i + 1) / 8.0f;
                addTriangle(vertices, boltX, boltY,
                    boltX + 0.01f * cos(angle1), boltY + 0.01f * sin(angle1),
                    boltX + 0.01f * cos(angle2), boltY + 0.01f * sin(angle2), redBolt);
            }
        }
    }
    
    return vertices;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

unsigned int compileShader(unsigned int type, const std::string& source) {
    unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return shader;
}

unsigned int createShaderProgram(const std::string& vertexSource, const std::string& fragmentSource) {
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cout << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    
    // Boom control
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) craneState.boomAngle = std::min(70.0f, craneState.boomAngle + 0.5f);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) craneState.boomAngle = std::max(20.0f, craneState.boomAngle - 0.5f);
    
    // Movement
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        craneState.positionX = std::min(0.5f, std::max(-0.5f, craneState.positionX + 0.003f));
        craneState.wheelRotation += 0.03f;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        craneState.positionX = std::min(0.5f, std::max(-0.5f, craneState.positionX - 0.003f));
        craneState.wheelRotation -= 0.03f;
    }
    
    // Rotation
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) craneState.wholeObjectRotation = std::max(MIN_ROTATION, craneState.wholeObjectRotation - 0.01f);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) craneState.wholeObjectRotation = std::min(MAX_ROTATION, craneState.wholeObjectRotation + 0.01f);
    
    // Toggle boom auto-rotation
    static bool rKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !rKeyPressed) {
        craneState.boomRotating = !craneState.boomRotating;
        rKeyPressed = true;
        std::cout << "Boom auto-rotation: " << (craneState.boomRotating ? "ON" : "OFF") << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) rKeyPressed = false;
    
    // Toggle auto-movement
    static bool aKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && !aKeyPressed) {
        craneState.autoMoving = !craneState.autoMoving;
        aKeyPressed = true;
        std::cout << "Auto-movement: " << (craneState.autoMoving ? "ON" : "OFF") << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE) aKeyPressed = false;
}

void updateAnimation(float deltaTime) {
    // Hook animation
    if (craneState.hookMovingDown) {
        craneState.hookHeight -= 0.3f * deltaTime;
        if (craneState.hookHeight < -0.1f) {
            craneState.hookHeight = -0.1f;
            craneState.hookMovingDown = false;
        }
    } else {
        craneState.hookHeight += 0.3f * deltaTime;
        if (craneState.hookHeight > 0.5f) {
            craneState.hookHeight = 0.5f;
            craneState.hookMovingDown = true;
        }
    }
    
    // Boom auto-rotation
    if (craneState.boomRotating) {
        craneState.boomAngle += 15.0f * deltaTime;
        if (craneState.boomAngle > 70.0f) craneState.boomAngle = 20.0f;
    }
    
    // Auto-movement
    if (craneState.autoMoving) {
        static float autoDirection = 1.0f;
        craneState.positionX += autoDirection * 0.1f * deltaTime;
        craneState.wheelRotation += autoDirection * 1.0f * deltaTime;
        
        if (craneState.positionX > 0.4f) autoDirection = -1.0f;
        else if (craneState.positionX < -0.4f) autoDirection = 1.0f;
    }
}

void setupVertexAttributes() {
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void renderComponent(unsigned int VAO, unsigned int VBO, const std::vector<float>& vertices, 
                     float* transformMatrix, unsigned int modelLoc) {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    setupVertexAttributes();
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, transformMatrix);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 5);
}

int main() {
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(1000, 750, "Realistic Animated Crane", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD!" << std::endl;
        return -1;
    }

    glEnable(GL_MULTISAMPLE);

    std::string vertexShaderSource = readShaderSource("shader.vs");
    std::string fragmentShaderSource = readShaderSource("shader.fs");
    unsigned int shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    // Create VAOs and VBOs (now 5 components)
    unsigned int VAOs[5], VBOs[5];
    glGenVertexArrays(5, VAOs);
    glGenBuffers(5, VBOs);

    glClearColor(0.85f, 0.9f, 0.95f, 1.0f);

    std::cout << "\n╔══════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║     REALISTIC ANIMATED CRANE CONTROLS           ║" << std::endl;
    std::cout << "╠══════════════════════════════════════════════════╣" << std::endl;
    std::cout << "║                                                  ║" << std::endl;
    std::cout << "║  MANUAL CONTROLS:                                ║" << std::endl;
    std::cout << "║  ┌────────────────────────────────────────────┐  ║" << std::endl;
    std::cout << "║  │ LEFT/RIGHT Arrows  → Move crane left/right│  ║" << std::endl;
    std::cout << "║  │                      (wheels rotate)       │  ║" << std::endl;
    std::cout << "║  │ UP/DOWN Arrows     → Raise/lower boom arm │  ║" << std::endl;
    std::cout << "║  │                      (20° - 70° range)     │  ║" << std::endl;
    std::cout << "║  │ Q / E Keys         → Rotate crane ±45°    │  ║" << std::endl;
    std::cout << "║  │                      (limited rotation)    │  ║" << std::endl;
    std::cout << "║  └────────────────────────────────────────────┘  ║" << std::endl;
    std::cout << "║                                                  ║" << std::endl;
    std::cout << "║  AUTOMATIC CONTROLS:                             ║" << std::endl;
    std::cout << "║  ┌────────────────────────────────────────────┐  ║" << std::endl;
    std::cout << "║  │ R Key             → Toggle boom auto-rotate│  ║" << std::endl;
    std::cout << "║  │                      (sweeps 20°-70°)      │  ║" << std::endl;
    std::cout << "║  │ A Key             → Toggle auto-movement   │  ║" << std::endl;
    std::cout << "║  │                      (moves left/right)    │  ║" << std::endl;
    std::cout << "║  └────────────────────────────────────────────┘  ║" << std::endl;
    std::cout << "║                                                  ║" << std::endl;
    std::cout << "║  AUTOMATIC ANIMATIONS (Always Active):          ║" << std::endl;
    std::cout << "║  ┌────────────────────────────────────────────┐  ║" << std::endl;
    std::cout << "║  │ • Hook moves up and down continuously      │  ║" << std::endl;
    std::cout << "║  │ • Cable extends and retracts               │  ║" << std::endl;
    std::cout << "║  │ • Wheels rotate when crane moves           │  ║" << std::endl;
    std::cout << "║  └────────────────────────────────────────────┘  ║" << std::endl;
    std::cout << "║                                                  ║" << std::endl;
    std::cout << "║  ESC                  → Exit application         ║" << std::endl;
    std::cout << "║                                                  ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════╝" << std::endl;
    std::cout << "\n  Rotation Range: -45° to +45° (limited swing)\n" << std::endl;
    std::cout << "Press any control key to begin...\n" << std::endl;

    float lastFrame = 0.0f;
    float transformMatrix[16];

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        processInput(window);
        updateAnimation(deltaTime);

        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        
        // Render crane chassis with whole object rotation around its center
        createTransformMatrixWithPivot(transformMatrix, craneState.positionX, 0.0f, 
                                       craneState.wholeObjectRotation, craneState.positionX, 0.0f);
        renderComponent(VAOs[0], VBOs[0], getCraneBodyVertices(), transformMatrix, modelLoc);
        
        // Render wheels with whole object rotation
        renderComponent(VAOs[1], VBOs[1], getWheelVertices(craneState.wheelRotation), transformMatrix, modelLoc);
        
        // Render turret platform with whole object rotation
        renderComponent(VAOs[2], VBOs[2], getTurretVertices(), transformMatrix, modelLoc);
        
        // CORRECT FIX: Boom stays attached by using proper pivot transformation
        // The boom pivot point in the crane's local coordinate system (adjusted closer to body)
        float boomPivotLocalX = 0.0f;
        float boomPivotLocalY = 0.03f;  // Moved closer to body (was 0.2f)
        
        // Transform the boom pivot by the crane's rotation
        float cosWhole = cos(craneState.wholeObjectRotation);
        float sinWhole = sin(craneState.wholeObjectRotation);
        float boomPivotRotatedX = boomPivotLocalX * cosWhole - boomPivotLocalY * sinWhole;
        float boomPivotRotatedY = boomPivotLocalX * sinWhole + boomPivotLocalY * cosWhole;
        
        // Add crane's world position
        float boomPivotWorldX = craneState.positionX + boomPivotRotatedX;
        float boomPivotWorldY = boomPivotRotatedY;
        
        // Calculate total boom rotation (crane rotation + boom angle)
        float totalBoomRotation = craneState.wholeObjectRotation + craneState.boomAngle * 3.14159f / 180.0f;
        
        // The key: Use 0,0 as translation and the world pivot for rotation
        // Then manually add the translation
        float cosTotal = cos(totalBoomRotation);
        float sinTotal = sin(totalBoomRotation);
        
        transformMatrix[0] = cosTotal;
        transformMatrix[1] = sinTotal;
        transformMatrix[4] = -sinTotal;
        transformMatrix[5] = cosTotal;
        transformMatrix[12] = boomPivotWorldX;
        transformMatrix[13] = boomPivotWorldY;
        
        transformMatrix[2] = transformMatrix[3] = transformMatrix[6] = transformMatrix[7] = 0.0f;
        transformMatrix[8] = transformMatrix[9] = 0.0f;
        transformMatrix[10] = 1.0f;
        transformMatrix[11] = 0.0f;
        transformMatrix[14] = 0.0f;
        transformMatrix[15] = 1.0f;
        
        renderComponent(VAOs[3], VBOs[3], getBoomVertices(), transformMatrix, modelLoc);
        
        // Render hook and cable with same transformation as boom
        renderComponent(VAOs[4], VBOs[4], getCableAndHookVertices(craneState.hookHeight), 
                       transformMatrix, modelLoc);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(5, VAOs);
    glDeleteBuffers(5, VBOs);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}
