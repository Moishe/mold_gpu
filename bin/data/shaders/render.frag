#version 120
#extension GL_ARB_texture_rectangle : enable

// This fill the billboard made on the Geometry Shader with a texture

uniform sampler2DRect prevTex;

void main() {
    gl_FragColor = vec4(1.0, 1.0, 1.0, 0.05);
}
