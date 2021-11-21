#version 330 core
#define PI 3.1415926538

uniform sampler2DRect velData;
uniform sampler2DRect posData;
uniform sampler2DRect colorData;
uniform sampler2DRect trailData;
uniform sampler2DRect randomData;

uniform vec2 screen;
uniform float timestep;
uniform float maxage;

in vec2 vTexCoord;

out vec4 vFragColor;

const float look_segments = 10;

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
    vec2 pos = texture(posData, vTexCoord).xy;
    float dir = texture(velData, vTexCoord).x;
    float age = texture(velData, vTexCoord).y;
    float rand = texture(velData, vTexCoord).z;
    vec3 goal = texture(colorData, vTexCoord).rgb;

    float look_amt = 0.03 * (1 + (1 - length(goal)));
    float d = length(1/screen) * 2 * (1 + length(goal));
    float maxdp = 0;
    float idxmax = -1;
    float dirmax = dir + random(pos) - 0.5;
    vec3 colormax = goal;
    for (int i = 0; i < look_segments; i++) {
        for (int j = 0; j < 2; j++) {
            int mul;
            if (j == 0) {
                mul = 1;
            } else {
                mul = -1;
            }
            float dirlook = dir + float(i) * look_amt * mul;
            vec2 dir_p = look_dir(pos, dirlook, d);
            vec3 dest = texture(trailData, dir_p).xyz;
            float dot_p = abs(dot(goal, dest));
            if (dot_p > maxdp) {
                maxdp = dot_p;
                idxmax = i;
                dirmax = dirlook;
                colormax = dest;
            }
        }
    }
    
    float mx = 0.5 + random(pos * dir) * 0.5;
    dir = dirmax;// mix(dir, dirmax, mx);

    pos.x += cos(dir) * timestep;
    pos.y += sin(dir) * timestep;

    vec3 color_at_next_loc = texture(trailData, pos).xyz;
    if (idxmax != -1 && distance(color_at_next_loc, goal) < 0.1) {
        dir += PI + rand - 0.5;
        age = 1.0;
    }
    
    if (length(goal) == 0) {
        age = 1.0;
    }
    
    age -= 1.0;
    if (age <= 0.0) {
        age = 500.0 + round(random(rand * dir * pos) * 50);
        //dir = texture(velData, vec2(0,0)).x;
    }

    if (pos.x < 0 || pos.y < 0 || pos.x > 1 || pos.y > 1) {
        age = 0.0;
        //dir = texture(velData, vec2(int(vTexCoord.x / 2), int(vTexCoord.y / 2))).x;
    }

    vFragColor = vec4(dir, age, random(pos * dir * rand), /*max(random(pos * rand), 1.0),*/ 1.0);
}
