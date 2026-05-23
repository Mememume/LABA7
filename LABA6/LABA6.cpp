#define GLEW_DLL
#define GLFW_DLL

#include <iostream>
#include <cmath>
#include "GL\glew.h"
#include "GLFW/glfw3.h"
#include "Shader.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Model.h"

//Управление камерой
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 512.0f;
float lastY = 512.0f;
bool firstMouse = true;
float sensitivity = 0.1f;
float cameraSpeed = 2.5f;

glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 8.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;

//Начальные углы вращения деталей
float OX1_Angle = 0.0f;
float OX2_Angle = 0.0f;
float OX3_Angle = 0.0f;

// Точки вращения
float OX1_OffsetX = -1.0622f;
float OX1_OffsetY = 0.72316f;
float OX1_OffsetZ = 0.0f;

float OX2_OffsetX = -0.56763f;
float OX2_OffsetY = 0.73893f;
float OX2_OffsetZ = 0.0f;

float OX3_OffsetX = 0.60176f;
float OX3_OffsetY = 0.73924f;
float OX3_OffsetZ = 0.32593f;

//Обработка мыши
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;

    lastX = (float)xpos;
    lastY = (float)ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    cameraFront = glm::normalize(front);
}

//Обработка клавиатуры
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    float currentSpeed = cameraSpeed * deltaTime;

    // Движение камеры: WASD
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += currentSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= currentSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * currentSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * currentSpeed;

    // Управление OX1: Q/E
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        OX1_Angle += deltaTime * 50.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        OX1_Angle -= deltaTime * 50.0f;
    }

    // Управление OX2: R/F, ограничение -50 до +120
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        OX2_Angle += deltaTime * 50.0f;
        if (OX2_Angle > 120.0f) OX2_Angle = 120.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        OX2_Angle -= deltaTime * 50.0f;
        if (OX2_Angle < -50.0f) OX2_Angle = -50.0f;
    }

    // Управление OX3: T/G, ограничение -65 до +150
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        OX3_Angle += deltaTime * 50.0f;
        if (OX3_Angle > 150.0f) OX3_Angle = 150.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        OX3_Angle -= deltaTime * 50.0f;
        if (OX3_Angle < -65.0f) OX3_Angle = -65.0f;
    }
}

int main() {
    if (!glfwInit()) {
        fprintf(stderr, "error: не запускается GLFW3.\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Создание окна
    GLFWwindow* window = glfwCreateWindow(1024, 768, "3D Model Viewer with Lighting and Animation", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    GLenum ret = glewInit();
    if (GLEW_OK != ret) {
        fprintf(stderr, "error: %s\n", glewGetErrorString(ret));
        return 1;
    }

    glEnable(GL_DEPTH_TEST);

    const GLubyte* version_str = glGetString(GL_VERSION);
    const GLubyte* device_str = glGetString(GL_RENDERER);
    printf("OpenGL version: %s\n", version_str);
    printf("Renderer: %s\n", device_str);

    // Загрузка шейдеров и модели
    Shader ourShader("vert.glsl", "frag.glsl");
    Model ourModel("models/kawasaki.obj");

    // Настройка материала модели
    ourShader.use();
    ourShader.setVec3("material.ambient", 0.1f, 0.12f, 0.18f);
    ourShader.setVec3("material.diffuse", 0.75f, 0.8f, 0.95f);
    ourShader.setVec3("material.specular", 0.9f, 0.95f, 1.0f);
    ourShader.setFloat("material.shininess", 160.0f);

    // Настройка источника света
    ourShader.setVec3("light.ambient", 0.1f, 0.1f, 0.1f);
    ourShader.setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
    ourShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

    // Главный цикл рендеринга
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Матрицы камеры и проекции
        glm::mat4 modelMat = glm::mat4(1.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1024.0f / 768.0f, 0.1f, 100.0f);

        ourShader.use();
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        ourShader.setVec3("viewPos", cameraPos);
        ourShader.setVec3("light.position", 2.0f, 3.0f, 4.0f);

        // Отрисовка модели с иерархическими поворотами
        ourModel.Draw(ourShader, modelMat,
            OX1_Angle, OX2_Angle, OX3_Angle,
            OX1_OffsetX, OX1_OffsetY, OX1_OffsetZ,
            OX2_OffsetX, OX2_OffsetY, OX2_OffsetZ,
            OX3_OffsetX, OX3_OffsetY, OX3_OffsetZ);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}