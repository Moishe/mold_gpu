#version 330 core

// Fragment shader. Emit the color at the given location with very low alpha

in vec2 vTexCoord;
in vec4 vColor;

out vec4 fColor;

void main() {
    fColor = vec4(vColor.rgb, 0.05); // * length(vColor.rgb)); //max(0.0, min(1.0, length(vColor.rgb))) / 20.0);
}
