#include "TexturedMesh.h"
#include "PLYLoader.h"
#include "LoadBitmap.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <iostream>

TexturedMesh::TexturedMesh(const std::string& plyPath,
                           const std::string& bmpPath,
                           GLuint shaderProgram)
{
    // Just store the program handle so draw() can use it
    program = shaderProgram;

    // 1) Load mesh data from PLY
    std::vector<VertexData> verts;
    std::vector<TriData> tris;
    readPLYFile(plyPath, verts, tris);

    // 2) Convert to arrays GL wants
    // I’m using separate arrays for pos and uv because the assignment wants
    // separate VBOs (and it’s also easy to see what’s going on).
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uvs;
    positions.reserve(verts.size());
    uvs.reserve(verts.size());

    for (const auto& v : verts) {
        positions.push_back(glm::vec3(v.x, v.y, v.z));
        uvs.push_back(glm::vec2(v.u, v.v));
    }

    // Indices come from triangle list
    std::vector<unsigned int> indices;
    indices.reserve(tris.size() * 3);
    for (const auto& t : tris) {
        indices.push_back(t.i0);
        indices.push_back(t.i1);
        indices.push_back(t.i2);
    }
    indexCount = (GLsizei)indices.size();

    // 3) Load texture from BMP and upload to GPU
    loadTextureBMP(bmpPath);

    // 4) Create VAO/VBO/EBO
    // IMPORTANT: The assignment wants the VAO set up once (in constructor),
    // not rebuilt every frame in draw().
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Positions VBO -> attribute 0
    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER,
                 positions.size() * sizeof(glm::vec3),
                 positions.data(),
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,              // location 0
        3,              // vec3
        GL_FLOAT,
        GL_FALSE,
        0,
        (void*)0
    );

    // UV VBO -> attribute 1
    glGenBuffers(1, &vboUV);
    glBindBuffer(GL_ARRAY_BUFFER, vboUV);
    glBufferData(GL_ARRAY_BUFFER,
                 uvs.size() * sizeof(glm::vec2),
                 uvs.data(),
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,              // location 1
        2,              // vec2
        GL_FLOAT,
        GL_FALSE,
        0,
        (void*)0
    );

    // EBO stores indices. Important: EBO binding is part of the VAO state.
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(unsigned int),
                 indices.data(),
                 GL_STATIC_DRAW);

    // Unbind VAO so I don’t accidentally mess it up later
    glBindVertexArray(0);
}

void TexturedMesh::loadTextureBMP(const std::string& bmpPath) {
    // Assignment gave us loadARGB_BMP() which reads the BMP into memory.
    unsigned char* imageData = nullptr;
    unsigned int w = 0, h = 0;

    loadARGB_BMP(bmpPath.c_str(), &imageData, &w, &h);

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    // Many BMP loaders end up with BGRA layout. If textures look “color swapped”,
    // switching GL_BGRA -> GL_RGBA is usually the fix.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, imageData);

    // Basic filtering + mipmaps (house looks better with mipmaps)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    delete[] imageData;
}

void TexturedMesh::draw(const glm::mat4& MVP) {
    // Use shared shader program
    glUseProgram(program);

    // Send MVP uniform (one matrix per mesh draw)
    GLint mvpLoc = glGetUniformLocation(program, "MVP");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &MVP[0][0]);

    // Bind texture at texture unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    // Tell the sampler to read from texture unit 0
    GLint texLoc = glGetUniformLocation(program, "myTextureSampler");
    glUniform1i(texLoc, 0);

    // Bind VAO and draw with indices
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);
}