#version 330 core

uniform sampler2DRect prevPosData;
uniform sampler2DRect velData;

uniform float timestep;
uniform float locx;
uniform float locy;

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

    if (age >= 1000) {
        pos.x = random(pos);
        pos.y = random(pos + dir);
    }

    vFragColor = vec4(pos.x, pos.y, age, 1.0);
}
