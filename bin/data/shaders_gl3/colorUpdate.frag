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

    if (age == 0.0) {
        vFragColor = texture(origImageData, pos);
    } else {
        vFragColor = texture(prevColorData, vTexCoord);
    }
}
