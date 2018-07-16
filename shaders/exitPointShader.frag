#version 330 core

layout(location = 0) out vec4 fragColor;

in vec3 UV;

void main() {
    fragColor = vec4(UV, 1.0);
}