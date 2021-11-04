#version 120
#extension GL_ARB_texture_rectangle : enable

// This fill the billboard made on the Geometry Shader with a texture

uniform sampler2DRect prevTex;

void main() {
    vec2 st = gl_TexCoord[0].st;
    vec3 p = texture2DRect(prevTex, st).rgb;
    vec3 colorA = vec3(0.912,0.0,0.212);
    gl_FragColor = vec4(mix(p, colorA, 0.002), 1);
}
