#version 330 core

layout(location = 0) out vec4 fragColor;

in vec3 UV;
in vec4 fragPos;

uniform sampler2D exitPointTex;
uniform sampler3D volTex;

const float step_size = 0.005;
const int MAX_SAMPLES = 300;

void main() {
    vec3 startPoint = UV;
    vec2 texC       = .5*(fragPos.xy / fragPos.w + vec2(1,1));
    vec3 endPoint   = texture(exitPointTex, texC).xyz;
    vec3 diff = endPoint - startPoint;
    vec3 dir = normalize(diff);
    float len2 = dot(diff, diff);
    
    vec4 dst = vec4(0);
    fragColor = vec4(0);
    vec3 step = step_size * dir;
    vec3 pos = startPoint;
    
    for (int i = 0; i < MAX_SAMPLES && dot(pos - startPoint, pos - startPoint) < len2; i++) {
        float texVal = texture(volTex, vec3(pos.x, 1 - pos.y, pos.z)).r;
        vec4 src = vec4(texVal);
        src.a *= .5;
        
        // front to back blending
        fragColor.rgb = fragColor.rgb + (1 - fragColor.a) * src.a * src.rgb;
        fragColor.a   = fragColor.a + (1 - fragColor.a) * src.a;
        
        if (fragColor.a >= .95)
            break;
        
        pos += step;
        
        //if (pos.x > 1.0 || pos.y > 1.0 || pos.z > 1.0)
        //    break;
    }
    
    
}