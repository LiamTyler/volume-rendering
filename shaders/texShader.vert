#version 330 core

in vec3 vertex;

out vec2 UV;

void main() {
    gl_Position = vec4(vertex, 1);
    
    UV = .5 * (vertex.xy + vec2(1));
}