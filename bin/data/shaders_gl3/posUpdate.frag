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
    float lifespan = lifeData.x;
    float age = lifeData.y;
    float is_active = lifeData.z;

    if (is_active == 1 && age > 0) {
        pos.x += cos(dir) * timestep;
        pos.y += sin(dir) * timestep;
    } else if (is_active == 1 && age == 0) {
        vec2 randpos = texture(randomData, vTexCoord).xy * numParticlesSqrt;
        randpos.x = int(randpos.x);
        randpos.y = int(randpos.y);
        vec2 newpos = texture(prevPosData, randpos).xy;
        if (newpos.x >= 0 && newpos.y >= 0) {
            pos = newpos;
        } else {
            pos.x = randpos.x;
            pos.y = randpos.y;
        }
    } else {
        pos.x = -1;
        pos.y = -1;
    }

    vFragColor = vec4(pos.x, pos.y, 1.0, 1.0);
}
