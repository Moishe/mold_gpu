#version 330 core

uniform sampler2DRect prevLifeData;
uniform sampler2DRect prevPosData;
uniform sampler2DRect randomData;

in vec2 vTexCoord;

out vec4 vFragColor;

void main(void){
    vec3 lifeData = texture(prevLifeData, vTexCoord).xyz;
    vec2 pos = texture(prevPosData, vTexCoord).xy;
    float lifespan = lifeData.x;
    float age = lifeData.y;
    float is_active = lifeData.z;

    if (is_active == 1.0) {
        age += 1.0;
        if (age >= lifespan || pos.x < 0 || pos.y < 0 || pos.x > 1 || pos.y > 1) {
            is_active = 0;
            age = 0.0;
        }
    } else {
        vec3 random = texture(randomData, vTexCoord).xyz;
        if (random.x < 0.01) { // magic number
            is_active = 1;
            age = 0;
            lifespan = random.y * 500; // magic number
        }
    }

    vFragColor = vec4(lifespan, age, is_active, 1.0);
}
