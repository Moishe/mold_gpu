#version 120
#extension GL_EXT_geometry_shader4 : enable
#extension GL_EXT_gpu_shader4 : enable

uniform float size;

void main(void){
    for(int i = 0; i < gl_VerticesIn; i++){
        //gl_Position = gl_ModelViewProjectionMatrix * gl_PositionIn[i];
        //EmitVertex();

        gl_Position = gl_ModelViewProjectionMatrix * ( gl_PositionIn[i] + vec4(0,0,0.0,0.0));
        gl_TexCoord[0].x = 0.0;
        gl_TexCoord[0].y = 0.0;
        EmitVertex();
        /*
        
        gl_Position = gl_ModelViewProjectionMatrix * (gl_PositionIn[i] + vec4(size,-size,0.0,0.0));
        gl_TexCoord[0].x = 0.0;
        gl_TexCoord[0].y = 0.0;
        EmitVertex();
        
        gl_Position = gl_ModelViewProjectionMatrix * (gl_PositionIn[i] + vec4(size,size,0.0,0.0));
        gl_TexCoord[0].x = 0.0;
        gl_TexCoord[0].y = 0.0;
        EmitVertex();
        EndPrimitive();

        gl_Position = gl_ModelViewProjectionMatrix * (gl_PositionIn[i] + vec4(-size,-size,0.0,0.0));
        gl_TexCoord[0].x = 0.0;
        gl_TexCoord[0].y = 0.0;
        EmitVertex();
        
        gl_Position = gl_ModelViewProjectionMatrix * (gl_PositionIn[i] + vec4(-size,size,0.0,0.0));
        gl_TexCoord[0].x = 0.0;
        gl_TexCoord[0].y = 0.0;
        EmitVertex();
        
        gl_Position = gl_ModelViewProjectionMatrix * (gl_PositionIn[i] + vec4(size,size,0.0,0.0));
        gl_TexCoord[0].x = 0.0;
        gl_TexCoord[0].y = 0.0;
        EmitVertex();
        EndPrimitive();
        */
    }
}
