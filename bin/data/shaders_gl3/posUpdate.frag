#version 150

uniform sampler2DRect prevPosData;  // previous position texture
uniform sampler2DRect velData;      // velocity texture

uniform float timestep;

in vec2 vTexCoord;

out vec4 vFragColor;

void main(void){
    // Get the position and velocity from the pixel color.
    vec2 pos = texture( prevPosData, vTexCoord ).xy;
    float dir = texture( velData, vTexCoord ).x;
    
    // Update the position.
    pos.x += cos(dir) * timestep * 0.1;
    pos.y += sin(dir) * timestep * 0.1;
    
    // And finally store it on the position FBO.
    vFragColor = vec4(pos.x,pos.y,1.0,1.0);  
}
