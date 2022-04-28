#version 410 core

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;
out vec2 tex_coord;

const vec2 coordinates [] = vec2 [] (vec2 (0.0f, 0.0f),
                                    vec2 (1.0f, 0.0f),
                                    vec2 (1.0f, 1.0f),
                                    vec2 (0.0f, 1.0f));

void main() {    
    
    gl_Position = gl_in[0].gl_Position + vec4(0, 0, 0, 0);
    tex_coord = vec2(0, 0);
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(0, 1, 0, 0);
    tex_coord = vec2(0, 1);
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(0.5625, 0, 0, 0);
    tex_coord = vec2(1, 0);
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(0.5625, 1, 0, 0);
    tex_coord = vec2(1, 1);
    EmitVertex();
    
    EndPrimitive();
}  