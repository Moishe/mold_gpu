#version 330 core

uniform sampler2DRect prevRandData;

in vec2 vTexCoord;

out vec4 vFragColor;

float random (vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

void main(void){
    vec3 r = texture(prevRandData, vTexCoord).xyz;

    r.x = random(r.xy);
    r.y = random(r.yz);
    r.z = random(r.xz);
    if (r.x == 0) {
        r.x = 0.3;
    }
    
    if (r.y == 0) {
        r.y = 0.1;
    }
    
    if (r.z == 0) {
        r.z = 0.4;
    }

    vFragColor = vec4(r, 1.0);
}

