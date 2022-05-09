#version 410 core

layout(location = 0) in vec3 position;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform int billboard;

void main()
{
    if(billboard == 1) {
        gl_Position = projectionMatrix * viewMatrix * vec4(position.x, position.y, position.z, 1.0);
    } else {
        gl_Position = vec4(position.x, position.y, position.z, 1.0);
    }
}