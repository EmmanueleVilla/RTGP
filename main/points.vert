#version 410 core

in vec2 aPos;

out vec2 interp_UV;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
}