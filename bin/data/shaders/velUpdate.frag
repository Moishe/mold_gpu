#version 120
#extension GL_ARB_texture_rectangle : enable
#define KERNEL_SIZE 9

uniform sampler2DRect backbuffer;
uniform sampler2DRect posData;
uniform sampler2DRect trailData;

uniform float timestep;
uniform vec2 screen;

vec2 look_dir(vec2 pos, float dir, float d);

vec2 look_dir(vec2 pos, float dir, float d) {
    return vec2(
              (pos.x + cos(dir) * d) * screen.x,
              (pos.y + sin(dir) * d) * screen.y
            );
}

void main(void){
    vec2 st = gl_TexCoord[0].st;
    
    vec2 pos = texture2DRect( posData, st).xy;
    float dir = texture2DRect( backbuffer, st ).x;
    vec3 goal = vec3(texture2DRect( backbuffer, st ).yzw);

    float d = 4.0 * timestep;
    vec2 lp = look_dir(pos, dir + 3.14 / 3.0, d);
    vec2 rp = look_dir(pos, dir - 3.14 / 3.0, d);
    vec2 cp = look_dir(pos, dir, d);

    vec3 left = texture2DRect(trailData, lp).xyz;
    vec3 center = texture2DRect(trailData, rp).xyz;
    vec3 right = texture2DRect(trailData, cp).xyz;
    
    float ld = distance(goal, left);
    float rd = distance(goal, right);
    float cd = distance(goal, center);

    if (ld > cd && ld > rd) {
        dir -= 3.14 / 9.0;
    } else if (rd > cd && rd > ld) {
        dir += 3.14 / 9.0;
    }

    vec2 nextPos = pos;
    nextPos.x += cos(dir) * timestep; // Updates the position
    nextPos.y += sin(dir) * timestep; // Updates the position
        
/*    if ( nextPos.x <= 0.0 || nextPos.y <= 0 || nextPos.x >= 1 || nextPos.y >= 1)
        dir = 0;

    if ( nextPos.x >= 1.0)
        dir = dir - 3.14;

    if (nextPos.y <= 0.0)
        dir = dir - 3.14;

    if ( nextPos.y >= 1.0)
        dir = dir - 3.14;
*/
    gl_FragColor = vec4(dir,1.0,1.0,1.0);
}
