#version 330 core

uniform sampler2DRect prevPosData;
uniform sampler2DRect posData;

in vec2 vTexCoord;

out vec4 vFragColor;

void main(void) {
    vec2 pos = texture(prevPosData, vTexCoord).xy;
    
    if (pos.x == -1 && pos.y == -1) {
        vFragColor = texture(posData, vec2(0, 0));
    } else {
        vFragColor = pos;
    }
}

