#version 410 core

//--- INPUT AND OUTPUT GEOMETRY
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

//--- INPUT DELTAS FROM APP
uniform float deltaX;
uniform float deltaY;
uniform float deltaZ;

//--- INPUT FROM MAIN
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform int billboard;

//--- OUTPUT UV
out vec2 tex_coord;

void main() {
    gl_Position = gl_in[0].gl_Position + vec4(0, 0, 0, 0);
    if(billboard == 0) {
        gl_Position = gl_Position *  projectionMatrix;
    }
    tex_coord = vec2(0, 0);
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(0, deltaY, deltaZ, 0);
    if(billboard == 0) {
        gl_Position = gl_Position *  projectionMatrix;
    }
    tex_coord = vec2(0, 1);
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(deltaX, 0, 0, 0);
    if(billboard == 0) {
        gl_Position = gl_Position *  projectionMatrix;
    }
    tex_coord = vec2(1, 0);
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(deltaX, deltaY, deltaZ, 0);
    if(billboard == 0) {
        gl_Position = gl_Position *  projectionMatrix;
    }
    tex_coord = vec2(1, 1);
    EmitVertex();
    
    EndPrimitive();
}  