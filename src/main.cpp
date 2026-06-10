// CS3388B Assignment 4 - Textured Mesh House + First Person Camera
// Name: Muniver Kharod
// Student #: 251425117

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "TexturedMesh.h"

// Simple key state array so holding a key keeps moving every frame
static bool gKeys[1024];

// GLFW key callback: I just track pressed/released states.
// This makes the movement smooth (not one-step-per-press).
static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode; (void)mods;

    // ESC quits
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)   gKeys[key] = true;
        if (action == GLFW_RELEASE) gKeys[key] = false;
    }
}

int main() {
    // --- 1) Init GLFW (window + OpenGL context) ---
    if (!glfwInit()) {
        std::cerr << "ERROR: glfwInit failed\n";
        return 1;
    }

    // Request OpenGL 3.3 core (pretty standard for these assignments)
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1200, 800, "CS3388 A4 - LinksHouse", nullptr, nullptr);
    if (!window) {
        std::cerr << "ERROR: glfwCreateWindow failed\n";
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, KeyCallback);

    // --- 2) Init GLEW (loads OpenGL function pointers) ---
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        std::cerr << "ERROR: glewInit failed\n";
        glfwTerminate();
        return 1;
    }

    // --- 3) Basic OpenGL states ---
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Blending is required for transparent textures (alpha channel)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // --- 4) Load shaders ---
    GLuint program = LoadShaders("shaders/textured.vert", "shaders/textured.frag");
    if (program == 0) {
        std::cerr << "ERROR: Shader program failed\n";
        glfwTerminate();
        return 1;
    }

    // --- 5) Camera setup (assignment required values) ---
    // Starting position and facing direction:
    // pos = (0.5, 0.4, 0.5), forward = (0, 0, -1)
    glm::vec3 camPos(0.5f, 0.4f, 0.5f);

    // I store yaw only and keep movement on the XZ plane (no flying).
    // yaw=0 => forward = (0,0,-1)
    float yaw = 0.0f;

    // Projection: 45-degree vertical FOV
    glm::mat4 Projection = glm::perspective(
        glm::radians(45.0f),
        1200.0f / 800.0f,
        0.01f,
        100.0f
    );

    // --- 6) Load all meshes + textures ---
    // I assume the unzip contents are in assets/ next to the executable.
    std::string A = "assets/";

    TexturedMesh bottles (A+"Bottles.ply",     A+"bottles.bmp",      program);
    TexturedMesh curtains(A+"Curtains.ply",    A+"curtains.bmp",     program);
    TexturedMesh doorbg  (A+"DoorBG.ply",      A+"doorbg.bmp",       program);
    TexturedMesh floorM  (A+"Floor.ply",       A+"floor.bmp",        program);
    TexturedMesh metal   (A+"MetalObjects.ply",A+"metalobjects.bmp", program);
    TexturedMesh patio   (A+"Patio.ply",       A+"patio.bmp",        program);
    TexturedMesh table   (A+"Table.ply",       A+"table.bmp",        program);
    TexturedMesh walls   (A+"Walls.ply",       A+"walls.bmp",        program);
    TexturedMesh windowbg(A+"WindowBG.ply",    A+"windowbg.bmp",     program);
    TexturedMesh wood    (A+"WoodObjects.ply", A+"woodobjects.bmp",  program);

    // Assignment says these are transparent and should be drawn last
    curtains.isTransparent = true;
    metal.isTransparent    = true;
    doorbg.isTransparent   = true;

    // I split into two lists so ordering is easy and obvious
    std::vector<TexturedMesh*> opaque;
    std::vector<TexturedMesh*> transparent;

    // Opaque first (these generally don't use alpha)
    opaque.push_back(&floorM);
    opaque.push_back(&walls);
    opaque.push_back(&table);
    opaque.push_back(&wood);
    opaque.push_back(&bottles);
    opaque.push_back(&patio);
    opaque.push_back(&windowbg);

    // Transparent last (door bg + metal + curtains)
    transparent.push_back(&doorbg);
    transparent.push_back(&metal);
    transparent.push_back(&curtains);

    // --- 7) Main loop ---
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Camera movement step sizes (assignment suggested values are fine)
        const float moveStep = 0.05f;
        const float rotStep  = glm::radians(3.0f);

        // Compute forward direction from yaw (stays flat on XZ plane)
        glm::vec3 forward(glm::sin(yaw), 0.0f, -glm::cos(yaw));
        forward = glm::normalize(forward);

        // Arrow keys:
        // Up/Down move forward/backward
        // Left/Right rotate in place
        if (gKeys[GLFW_KEY_UP])    camPos += forward * moveStep;
        if (gKeys[GLFW_KEY_DOWN])  camPos -= forward * moveStep;
        if (gKeys[GLFW_KEY_LEFT])  yaw += rotStep;
        if (gKeys[GLFW_KEY_RIGHT]) yaw -= rotStep;

        // Look-at matrix: eye at camPos, looking in forward direction
        glm::mat4 View = glm::lookAt(camPos, camPos + forward, glm::vec3(0, 1, 0));

        // Clear both color and depth every frame
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw opaque meshes first
        for (auto* m : opaque) {
            glm::mat4 MVP = Projection * View * glm::mat4(1.0f);
            m->draw(MVP);
        }

        // Draw transparent meshes last
        // Trick: stop writing to the depth buffer during transparent draws,
        // otherwise you can get weird “cut out” artifacts.
        glDepthMask(GL_FALSE);
        for (auto* m : transparent) {
            glm::mat4 MVP = Projection * View * glm::mat4(1.0f);
            m->draw(MVP);
        }
        glDepthMask(GL_TRUE);

        glfwSwapBuffers(window);
    }

    glDeleteProgram(program);
    glfwTerminate();
    return 0;
}