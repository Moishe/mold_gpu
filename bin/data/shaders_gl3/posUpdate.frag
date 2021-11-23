#version 330 core

uniform sampler2DRect prevPosData;
uniform sampler2DRect velData;
uniform sampler2DRect lifeData;
uniform sampler2DRect randomData;

uniform float timestep;
uniform float numParticlesSqrt;

in vec2 vTexCoord;

out vec4 vFragColor;

void main(void){
    vec2 pos = texture(prevPosData, vTexCoord).xy;
    float dir = texture(velData, vTexCoord).x;
    vec3 lifeData = texture(lifeData, vTexCoord).xyz;
    vec2 newpos_age_1 = texture(prevPosData, pos).xy;
    vec2 newpos = texture(randomData, vTexCoord).xy;
    float lifespan = lifeData.x;
    float age = lifeData.y;
    float is_active = lifeData.z;

    if (is_active == 1 && age > 1.0) {
        pos.x += cos(dir) * timestep;
        pos.y += sin(dir) * timestep;
    } else if (is_active == 1 && age == 0) {
        if (pos.x >= 0 && pos.y >= 0 && pos.x < 1 && pos.y < 1) {
            pos.x = int(pos.x * newpos.x * numParticlesSqrt);
            pos.y = int(pos.y * newpos.y * numParticlesSqrt);
        } else {
            pos = newpos * numParticlesSqrt;
            pos.x = int(pos.x);
            pos.y = int(pos.y) % 2;
        }
    } else if (is_active == 1.0 && age == 1.0) {
        if (newpos_age_1.x >= 0 && newpos_age_1.y >= 0 && newpos_age_1.x < 1 && newpos_age_1.y < 1) {
            pos = newpos_age_1;
        } else {
            pos.x = -1;
            pos.y = -1;
        }
    } else {
        pos.x = -1;
        pos.y = -1;
    }

    vFragColor = vec4(pos.x, pos.y, 1.0, 1.0);
}
