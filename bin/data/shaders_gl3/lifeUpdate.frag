#version 330 core

uniform sampler2DRect prevLifeData;
uniform sampler2DRect prevPosData;
uniform sampler2DRect randomData;
uniform sampler2DRect colorData;
uniform sampler2DRect foodData;

in vec2 vTexCoord;

out vec4 vFragColor;

void main(void){
    vec3 lifeData = texture(prevLifeData, vTexCoord).xyz;
    vec2 pos = texture(prevPosData, vTexCoord).xy;
    vec3 color = texture(colorData, vTexCoord).xyz;
    vec3 random = texture(randomData, vTexCoord).xyz;
    vec3 food = texture(foodData, vTexCoord).xyz;

    float lifespan = lifeData.x;
    float age = lifeData.y;
    float is_active = lifeData.z;
    
    bool should_die = !(pos.x >=0 && pos.y >= 0 && pos.y < 1 && pos.x < 1);
    /*
    if (age > 10 && length(color) < 0.1) {
        should_die = true;
    }
    */

    if (age == 2) {
        lifespan *= length(food);
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
        if (random.x < 1.0) {
            is_active = 1;
            age = 0;
            lifespan = 128 + int(256 * random.y); // magic number
        }
    }

    vFragColor = vec4(lifespan, age, is_active, 1.0);
}
