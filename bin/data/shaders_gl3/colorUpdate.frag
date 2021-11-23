#version 330 core

uniform sampler2DRect origImageData;
uniform sampler2DRect prevColorData;
uniform sampler2DRect posData;
uniform sampler2DRect lifeData;

uniform vec2 screen;
uniform float numParticlesSqrt;

in vec2 vTexCoord;

out vec4 vFragColor;

void main(void) {
    vec2 pos = texture(posData, vTexCoord).xy * screen;
    vec3 life = texture(lifeData, vTexCoord).xyz;
    float lifespan = life.x;
    float age = life.y;
    float is_active = life.z;
    
    vec3 goalColor = texture(prevColorData, vTexCoord).rgb; // vec4(0.6, 0.2, 0.2, 1.0);
    vec3 origColor = texture(origImageData, pos).rgb;

    if (age == 0.0 && is_active == 1.0) {
        pos = texture(posData, vTexCoord).xy;
        vFragColor = vec4(pos, 1.0, 1.0); //vec4(mix(origColor * 1.2, spawnColor, 0.001), 1.0);
        //vFragColor = vec4(origColor, 1.0);
    } else if (age == 1.0 && is_active == 1.0) {
        vFragColor = vec4(origColor, 1.0);
    } else {
        vFragColor = vec4(goalColor, 1.0);
    }
}
