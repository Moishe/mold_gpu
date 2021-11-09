#version 330 core

// Vertex shader. Emit the x/y coordinates from our position texture,
// and the color from our color texture.

uniform sampler2DRect posTex;
uniform sampler2DRect colorTex;
uniform vec2 screen;

in vec2 texcoord;
in vec4 color;

uniform mat4 modelViewProjectionMatrix;

out vec4 vPosition;

out VS_OUT {
    vec4 color;
} vs_out;

void main() {
    vec4 pixPos = texture(posTex, texcoord);
    
    pixPos.x *= screen.x;
    pixPos.y *= screen.y;
    pixPos.z = 0;
    
    vPosition = modelViewProjectionMatrix * pixPos;
    vs_out.color = vec4(texture(colorTex, texcoord).rgb, 1);
}
