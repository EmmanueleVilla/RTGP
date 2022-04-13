#version 410 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec2 interp_UV[];
out vec2 TexCoords;

void main() {    
    gl_Position = gl_in[0].gl_Position;
    TexCoords = interp_UV[0];
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    TexCoords = interp_UV[1];
    EmitVertex();
    
    gl_Position = gl_in[2].gl_Position;
    TexCoords = interp_UV[2];
    EmitVertex();

    EndPrimitive();
}  