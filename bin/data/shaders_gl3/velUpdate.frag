#version 330 core
#define PI 3.1415926538

uniform sampler2DRect velData;
uniform sampler2DRect posData;
uniform sampler2DRect colorData;
uniform sampler2DRect foodData;
uniform sampler2DRect lifeData;
uniform sampler2DRect trailData;
uniform sampler2DRect randomData;

uniform vec2 screen;
uniform float timestep;
uniform float numParticlesSqrt;

in vec2 vTexCoord;

out vec4 vFragColor;

const float look_segments = 10;

vec2 look_dir(vec2 pos, float dir, float d) {
    return vec2(
              (pos.x + cos(dir) * d) * screen.x,
              (pos.y + sin(dir) * d) * screen.y
            );
}

float sigmoid(float f) {
    return f / (1 + pow(2.718281828459, f));
}

void main(void){
    vec2 pos = texture(posData, vTexCoord).xy;
    float dir = texture(velData, vTexCoord).x;
    float orig_dir = dir;
    vec3 goal = texture(colorData, vTexCoord).rgb;
    vec3 rand = texture(randomData, vTexCoord).rgb;
    vec3 life = texture(lifeData, vTexCoord).xyz;
    float lifespan = life.x;
    float age = life.y;
    float is_active = life.z;
    
    if (is_active == 1.0 && age == 0.0) {
        dir = texture(velData, pos).x + (rand.x - 0.5) * 0.314;
    } else {
        float look_amt = 0.0314;
        float d = length(1/screen) * 12;
        float maxdp = 0;
        float idxmax = -1;
        float dirmax = dir;
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
                vec3 trail_dest = texture(trailData, dir_p).xyz;
                vec3 food_dest = texture(foodData, dir_p).xyz;
                //vec3 dest = mix(normalize(trail_dest), trail_dest, 0.9);
                //goal = mix(normalize(goal), goal, 0.9);
                float dot_p = dot(goal, trail_dest) * length(food_dest);
                if (dot_p > maxdp) {
                    maxdp = dot_p;
                    idxmax = i;
                    dirmax = dirlook;
                }
            }
        }
        
        dirmax += (rand.x - 0.5) * 0.01;
        dir = mix(dir, dirmax, 0.9);
    }

    
    vFragColor = vec4(dir, 1.0, 1.0, /*max(random(pos * rand), 1.0),*/ 1.0);
}
