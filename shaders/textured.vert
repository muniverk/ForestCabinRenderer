#version 330 core
layout(location=0) in vec3 inPos;
layout(location=1) in vec2 inUV;

out vec2 UV;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(inPos, 1.0);
    UV = inUV;
}