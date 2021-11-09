#version 330 core
#define PI 3.1415926538

uniform sampler2DRect velData;
uniform sampler2DRect posData;
uniform sampler2DRect colorData;
uniform sampler2DRect trailData;
uniform sampler2DRect randomData;

uniform float timestep;
uniform vec2 screen;
uniform int dir_delta;

in vec2 vTexCoord;

out vec4 vFragColor;

vec2 look_dir(vec2 pos, float dir, float d) {
    return vec2(
              (pos.x + cos(dir) * d) * screen.x,
              (pos.y + sin(dir) * d) * screen.y
            );
}

float random (vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

void main(void){
    float look_amt = 0.1;
    float turn_amt = 0.1;
    vec2 pos = texture(posData, vTexCoord).xy;
    float dir = texture(velData, vTexCoord).x;
    float age = texture(velData, vTexCoord).y;
    vec3 goal = normalize(texture(colorData, vTexCoord).rgb);
    
    float d = 0.001; //length(1/screen) * 50;
    vec2 lp = look_dir(pos, dir - look_amt, d);
    vec2 rp = look_dir(pos, dir + look_amt, d);
    vec2 cp = look_dir(pos, dir, d);

    vec3 left = normalize(texture(trailData, lp).xyz);
    vec3 center = normalize(texture(trailData, rp).xyz);
    vec3 right = normalize(texture(trailData, cp).xyz);
    
    float ld = dot(goal, left);
    float rd = dot(goal, right);
    float cd = dot(goal, center);
    
    float mx = 0.99 + (length(random(goal.xy + pos * dir)) * 0.02);

    if (rd > cd && rd > ld) {
        dir = mix(dir, dir + turn_amt, mx);
    } else if (ld > cd && ld > rd) {
        dir = mix(dir, dir - turn_amt, mx);
    } else {
        dir += 0.2;// - (length(random(goal.xy + pos * turn_amt)) * 0.12);
    }
    
    age += 1.0;
    if (age > 1000.0) {
        age = 0.0;
    }

    vFragColor = vec4(dir, age, 1, 1);
}
