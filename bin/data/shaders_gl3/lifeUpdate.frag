#version 330 core

uniform sampler2DRect prevLifeData;
uniform sampler2DRect prevPosData;
uniform sampler2DRect randomData;
uniform sampler2DRect colorData;

in vec2 vTexCoord;

out vec4 vFragColor;

void main(void){
    vec3 lifeData = texture(prevLifeData, vTexCoord).xyz;
    vec2 pos = texture(prevPosData, vTexCoord).xy;
    vec3 color = texture(colorData, vTexCoord).xyz;

    float lifespan = lifeData.x;
    float age = lifeData.y;
    float is_active = lifeData.z;
    
    bool should_die = !(pos.x >=0 && pos.y >= 0 && pos.y < 1 && pos.x < 1);
    
    if (age > 10 && (color.x < 0.01 && color.y < 0.01 && color.z < 0.01)) {
        //should_die = true;
    }
    
    if (is_active == 1.0 && (age == 0 || !should_die)) {
        age += 1.0;
        if (age >= lifespan) {
            is_active = 0;
            age = 0.0;
        } else if (age == 3) {
            //lifespan *= length(color) * length(color);
        }
    } else {
        vec3 random = texture(randomData, vTexCoord).xyz;
        is_active = 1;
        age = 0;
        lifespan = 512; // magic number
    }

    vFragColor = vec4(lifespan, age, is_active, 1.0);
}
