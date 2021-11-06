#version 330

uniform sampler2DRect posTex;
uniform vec2 screen;

in vec2 texcoord;
in vec4 color;

out vec4 vPosition;
out vec2 vTexCoord;
out vec4 vColor;

void main() {
    vec4 pixPos = texture(posTex, texcoord);
    
    // Maps the position from the texture (from 0.0 to 1.0) to
    // the screen position (0 - screenWidth/screenHeight)
    //
    pixPos.x *= screen.x;
    pixPos.y *= screen.y;
    
    vPosition = pixPos;
    vTexCoord = texcoord;
    vColor = color;
}
