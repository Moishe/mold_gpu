#version 330 core

uniform sampler2DRect prevColorData;
uniform sampler2DRect posData;
uniform sampler2DRect origImageData;
uniform sampler2DRect lifeData;

uniform vec2 screen;

in vec2 vTexCoord;

out vec4 vFragColor;

void main(void) {
    vec2 pos = texture(posData, vTexCoord).xy * screen;
    vec3 life = texture(lifeData, vTexCoord).xyz;
    float lifespan = life.x;
    float age = life.y;
    float is_active = life.z;
    
    vec3 goalColor = texture(prevColorData, vTexCoord).rgb;
    vec3 origColor = texture(origImageData, pos).rgb;
    vec3 sharedActorGoal = texture(prevColorData, vTexCoord).xyz;

    if (age == 0.0 && is_active == 1.0) {
        vFragColor = vec4(sharedActorGoal, 1);
    } else if (age == 1.0 && is_active == 1.0) {
        vFragColor = vec4(origColor * 1.2, 1.0);
//    } else if (age == 2.0 && is_active == 1.0) {
//        vFragColor = goalColor;
    } else {
        vFragColor = vec4(goalColor, 1.0);
    }
}
