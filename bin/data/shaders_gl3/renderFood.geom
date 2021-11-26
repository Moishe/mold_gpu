#version 330 core

// Geometry shader. Transform the point with our projection matrix and emit it, and its color.

layout (points) in;

layout (max_vertices = 1) out;
layout (points) out;

in vec4 vPosition[];

in VS_OUT {
    vec4 color;
} gs_in[];

uniform mat4 modelViewProjectionMatrix;

out vec2 vTexCoord;
out vec4 vColor;

void main(){
    gl_Position = vPosition[0];
    vTexCoord.x = 0.0;
    vTexCoord.y = 0.0;
    vColor = gs_in[0].color;
    EmitVertex();
}
