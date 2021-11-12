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
    float look_amt = 0.6;
    vec2 pos = texture(posData, vTexCoord).xy;
    float dir = texture(velData, vTexCoord).x;
    float age = texture(velData, vTexCoord).y;
    vec3 goal = normalize(texture(colorData, vTexCoord).rgb);
    
    float d = length(1/screen) * 8;
    float maxdp = 0;
    float idxmax = 0;
    float dirmax = dir + random(pos + dir + age) * 0.1 - 0.05;
    for (int i = 0; i < 11; i++) {
        float dirlook = mix(dir - look_amt, dir + look_amt, float(i) * 0.1);
        vec2 dir_p = look_dir(pos, dirlook, d);
        float dot_p = abs(dot(goal, normalize(texture(trailData, dir_p).xyz)));
        if (dot_p > maxdp) {
            maxdp = dot_p;
            idxmax = i;
            dirmax = dirlook;
        }
    }
    float mx = 0.9;
    dir = mix(dir, dirmax, mx);
    
    pos.x += cos(dir) * timestep;
    pos.y += sin(dir) * timestep;
    if (pos.x < 0 || pos.y < 0 || pos.x > 1 || pos.y > 1) {
        age = 0.0;
    }

    age += 1.0;
    if (age >= 1000.0 + random(dir + pos * age) * 100.0) {
        age = 0.0;
        dir += PI;
    }

    vFragColor = vec4(dir, age, 1.0, 1.0);
}
