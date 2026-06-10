#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

// Tiny helper: read a whole file into a string.
static std::string ReadFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "ERROR: Cannot open shader file: " << path << "\n";
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Compiles either a vertex shader or fragment shader from source code.
static GLuint CompileShader(GLenum type, const std::string& src) {
    GLuint id = glCreateShader(type);
    const char* cstr = src.c_str();

    glShaderSource(id, 1, &cstr, nullptr);
    glCompileShader(id);

    // Check compile status and print log if it fails
    GLint ok = GL_FALSE;
    glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
    if (ok != GL_TRUE) {
        GLint logLen = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLen);

        std::vector<char> log(logLen + 1);
        glGetShaderInfoLog(id, logLen, nullptr, log.data());

        std::cerr << "Shader compile error:\n" << log.data() << "\n";
    }
    return id;
}

GLuint LoadShaders(const std::string& vertex_file_path, const std::string& fragment_file_path) {
    // Read shader files
    std::string vsrc = ReadFile(vertex_file_path);
    std::string fsrc = ReadFile(fragment_file_path);
    if (vsrc.empty() || fsrc.empty()) return 0;

    // Compile both
    GLuint vs = CompileShader(GL_VERTEX_SHADER, vsrc);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsrc);

    // Link into one program
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    // Check link status
    GLint ok = GL_FALSE;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (ok != GL_TRUE) {
        GLint logLen = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLen);

        std::vector<char> log(logLen + 1);
        glGetProgramInfoLog(prog, logLen, nullptr, log.data());

        std::cerr << "Program link error:\n" << log.data() << "\n";
    }

    // Shaders can be deleted after linking
    glDetachShader(prog, vs);
    glDetachShader(prog, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    return prog;
}