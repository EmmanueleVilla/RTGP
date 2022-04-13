#version 410 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

//--- INPUT FROM VERTEX SHADER
in vec2 interp_UV[];

//--- OUTPUT TO FRAGMENT SHADER
out vec2 texCoords;

void main() {    
    gl_Position = gl_in[0].gl_Position;
    texCoords = interp_UV[0];
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    texCoords = interp_UV[1];
    EmitVertex();
    
    gl_Position = gl_in[2].gl_Position;
    texCoords = interp_UV[2];
    EmitVertex();

    EndPrimitive();
}  