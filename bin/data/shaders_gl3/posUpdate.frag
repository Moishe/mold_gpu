#version 330

uniform sampler2DRect prevPosData;
uniform sampler2DRect velData;

uniform float timestep;

in vec2 vTexCoord;

out vec4 vFragColor;

void main(void){
    vec2 pos = texture( prevPosData, vTexCoord ).xy;
    float dir = texture( velData, vTexCoord ).x;

    pos.x += cos(dir) * timestep;
    pos.y += sin(dir) * timestep;
    
    if (pos.x < 0 || pos.y < 0 || pos.x >= 1 || pos.y >= 1) {
        pos.x = 0.5;
        pos.y = 0.5;
    }
    
    vFragColor = vec4(pos.x,pos.y,1.0,1.0);
}
