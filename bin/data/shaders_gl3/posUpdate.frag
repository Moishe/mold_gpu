#version 330 core

uniform sampler2DRect prevPosData;
uniform sampler2DRect velData;

uniform float timestep;
uniform float locx;
uniform float locy;
uniform float maxage;

in vec2 vTexCoord;

out vec4 vFragColor;

float random (vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

void main(void){
    vec2 pos = texture(prevPosData, vTexCoord).xy;
    float dir = texture(velData, vTexCoord).x;
    float age = texture(velData, vTexCoord).y;

    pos.x += cos(dir) * timestep;
    pos.y += sin(dir) * timestep;

    if (age <= 0 || pos.x >= 1.0 || pos.y >= 1.0 || pos.x <= 0 || pos.y <= 0) {
        if (vTexCoord.x == 0 && vTexCoord.y == 0) {
            pos.x = 0.5;
            pos.y = 0.5;
        } else {
            pos = texture(prevPosData, vec2(vTexCoord.x - 1, vTexCoord.y - 1)).xy;
            if (pos.x > 1.0 || pos.y > 1.0 || pos.x < 0 || pos.y < 0) {
                pos.x = 0.5;
                pos.y = 0.5;
            }
        }
    }

    vFragColor = vec4(pos.x, pos.y, 1.0, 1.0);
}
