#version 330 core

in vec3 vertex;

uniform mat4 MVP;

out vec3 UV;
out vec4 fragPos;

void main() {
    gl_Position = MVP*vec4(vertex, 1.0);
    fragPos = gl_Position;

    UV = vertex + vec3(0.5);
}