#version 120
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect prevPosData;
uniform sampler2DRect velData;

uniform float timestep;

void main(void){
    vec2 st = gl_TexCoord[0].st;
    
    vec2 pos = texture2DRect( prevPosData, st ).xy;
    float dir = texture2DRect( velData, st ).x;
    
    pos.x += cos(dir) * timestep;
    pos.y += sin(dir) * timestep;
    
    if (pos.x < 0 || pos.y < 0 || pos.x >= 1 || pos.y >= 1) {
        pos.x = 0.5;
        pos.y = 0.5;        
    }
    
    gl_FragColor.rgba = vec4(pos.x,pos.y,1.0,1.0);
}
