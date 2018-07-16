#version 330 core

layout(location = 0) out vec4 fragColor;

in vec3 UV;

uniform sampler2D exitPointTex;

void main() {
    vec3 startPoint = UV;
    vec3 endPoint   = texture(exitPointTex, startPoint.xy).xyz;
    vec3 dir = normalize(endPoint - startPoint);
    fragColor = vec4(endPoint, 1.0);
}