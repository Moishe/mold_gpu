#version 330
#define KERNEL_SIZE 9
#define PI 3.1415926538

uniform sampler2DRect velData;
uniform sampler2DRect posData;
uniform sampler2DRect trailData;

uniform float timestep;
uniform vec2 screen;

in vec2 vTexCoord;

out vec4 vFragColor;

vec2 look_dir(vec2 pos, float dir, float d);

vec2 look_dir(vec2 pos, float dir, float d) {
    return vec2(
              (pos.x + cos(dir) * d) * screen.x,
              (pos.y + sin(dir) * d) * screen.y
            );
}

void main(void){
    vec2 pos = texture(posData, vTexCoord).xy;
    float dir = texture(velData, vTexCoord).x;
    vec3 goal = vec3(1.0, 1.0, 1.0);
    
    float d = 5.0 * timestep;
    vec2 lp = look_dir(pos, dir + PI / 6.0, d);
    vec2 rp = look_dir(pos, dir - PI / 6.0, d);
    vec2 cp = look_dir(pos, dir, d);

    vec3 left = texture(trailData, lp).xyz;
    vec3 center = texture(trailData, rp).xyz;
    vec3 right = texture(trailData, cp).xyz;
    
    float ld = distance(goal, left);
    float rd = distance(goal, right);
    float cd = distance(goal, center);

    if (rd > cd && rd > ld) {
        dir += PI / 6.0;
    } else if (ld > cd && ld > rd) {
        dir -= PI / 6.0;
    }

    vFragColor = vec4(dir,1.0,1.0,1.0);
}
