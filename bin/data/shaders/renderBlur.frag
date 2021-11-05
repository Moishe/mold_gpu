#version 120
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect image;

uniform bool horizontal;
uniform vec2 screen;

uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
    vec2 st = gl_TexCoord[0].st;
    vec3 result = texture2DRect(image, st).rgb;
    result = max(result - 0.04, vec3(0,0,0));
    gl_FragColor = vec4(result, 1.0);
/*
    vec2 tex_offset = 1.0 / screen; // gets size of single texel
    vec3 result = texture2DRect(image, st).rgb * weight[0]; // current fragment's contribution

    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture2DRect(image, st + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture2DRect(image, st - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture2DRect(image, st + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture2DRect(image, st - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }

    result = max(result - 0.01, vec3(0,0,0));
    gl_FragColor = vec4(result, 1.0);
*/
}
