#version 330

layout (points) in;
layout (max_vertices = 1) out;
layout (points) out;

in vec4 vPosition[];

uniform mat4 modelViewProjectionMatrix;

out vec2 vTexCoord;

void main(void){
    for(int i = 0; i < gl_in.length(); i++){
        gl_Position = modelViewProjectionMatrix * vPosition[i];
        vTexCoord.x = 0.0;
        vTexCoord.y = 0.0;
        EmitVertex();
    }
}
