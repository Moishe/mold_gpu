#version 330 core

uniform sampler2DRect prevColorData;
uniform sampler2DRect posData;
uniform sampler2DRect velData;
uniform sampler2DRect origImageData;
uniform vec2 screen;

in vec2 vTexCoord;

out vec4 vFragColor;

void main(void) {
    vec2 pos = texture(posData, vTexCoord).xy * screen;
    float age = texture(velData, vTexCoord).y;

    vec3 goalColor = texture(prevColorData, vTexCoord).rgb; // vec4(0.6, 0.2, 0.2, 1.0);
    vec3 origColor = texture(origImageData, pos).rgb;

    if (age <= 1.0) {
        vFragColor = vec4(origColor, 1.0); //mix(origColor, vec4(1.0, 1.0, 1.0, 1.0), 0.2);
    } else {
        vFragColor = vec4(goalColor, 1.0);
    }
}
