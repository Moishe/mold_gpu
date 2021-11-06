#version 330

// This fill the billboard made on the Geometry Shader with a texture

in vec2 vTexCoord;

out vec4 vFragColor;

void main() {
    vFragColor = vec4(1.0, 1.0, 1.0, 0.01);
}
