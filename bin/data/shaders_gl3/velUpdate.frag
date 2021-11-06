#version 330
#define PI 3.1415926538

uniform sampler2DRect velData;
uniform sampler2DRect posData;
uniform sampler2DRect colorData;
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
    float look_amt = PI / 6.0;
    float turn_amt = PI / 6.0;
    vec2 pos = texture(posData, vTexCoord).xy;
    float dir = texture(velData, vTexCoord).x;
    vec3 goal = texture(colorData, vTexCoord).rgb;
    
    float d = length(1/screen) * 4;
    vec2 lp = look_dir(pos, dir + look_amt, d);
    vec2 rp = look_dir(pos, dir - look_amt, d);
    vec2 cp = look_dir(pos, dir, d);

    vec3 left = texture(trailData, lp).xyz;
    vec3 center = texture(trailData, rp).xyz;
    vec3 right = texture(trailData, cp).xyz;
    
    float ld = distance(goal, left);
    float rd = distance(goal, right);
    float cd = distance(goal, center);

    if (rd > cd && rd > ld) {
        dir += turn_amt;
    } else if (ld > cd && ld > rd) {
        dir -= turn_amt;
    }

    vFragColor = vec4(dir,1.0,1.0,1.0);
}
