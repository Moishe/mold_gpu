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

    vec4 goalColor = texture(prevColorData, vTexCoord); // vec4(0.6, 0.2, 0.2, 1.0);
    vec4 origColor = texture(origImageData, pos);

    if (age <= 1.0) {
        vFragColor = mix(origColor, vec4(1.0, 1.0, 1.0, 1.0), 0.5);
    } else {
        vFragColor = mix(origColor, goalColor, 0.9);
    }
}
