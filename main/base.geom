#version 410 core

//--- POINT IN INPUT
layout (points) in;

//--- POINT IN OUTPUT
layout (points, max_vertices = 1) out;

void main() {
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    EndPrimitive();
}  