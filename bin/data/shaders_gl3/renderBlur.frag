#version 330 core

// This fill the billboard made on the Geometry Shader with a texture

in vec2 vTexCoord;

uniform sampler2DRect image;
uniform bool horizontal;
uniform vec2 screen;

uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

out vec4 vFragColor;

void main() {
    vec2 tex_offset = 0.12 * vec2(1, 1);
    vec3 result = texture(image, vTexCoord).rgb * weight[0]; // current fragment’s contribution
    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, vTexCoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(image, vTexCoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, vTexCoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(image, vTexCoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    result = max(result - 0.000002, vec3(0,0,0));
    vFragColor = vec4(result, 1.0);
}
