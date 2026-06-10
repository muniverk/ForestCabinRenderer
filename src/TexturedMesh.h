#pragma once
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

// This class owns:
// - GPU buffers for positions + UVs + indices
// - a texture object
// - a VAO that "remembers" attribute bindings
//
// So in draw(), I just bind the VAO + texture and call glDrawElements.
class TexturedMesh {
public:
    TexturedMesh(const std::string& plyPath,
                 const std::string& bmpPath,
                 GLuint shaderProgram);

    // Draw with the provided MVP matrix (model is identity in this assignment)
    void draw(const glm::mat4& MVP);

    // Used by main() to put meshes in the right draw order
    bool isTransparent = false;

private:
    GLuint vboPos = 0;   // vertex positions
    GLuint vboUV  = 0;   // vertex UVs
    GLuint ebo    = 0;   // element buffer (indices)
    GLuint vao    = 0;   // vertex array object (captures setup)
    GLuint tex    = 0;   // texture object

    GLuint program = 0;  // shader program shared by all meshes
    GLsizei indexCount = 0;

    void loadTextureBMP(const std::string& bmpPath);
};