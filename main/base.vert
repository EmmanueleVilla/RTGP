#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 UV;

//*** "BASIC" INPUT FROM APP ***//
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform int instanced;

//--- UBO INPUT FROM APP
layout (std140) uniform Matrices {
    //--- HANDLING A MAXIMUM OF 1024 ELEMENTS
    mat4 modelMatrices[1024];
};

//--- OUTPUT TO GEOMETRY SHADER
out vec2 interp_UV;

void main() {

    interp_UV = UV;

    if(instanced == 1) {
        gl_Position = projectionMatrix * viewMatrix * modelMatrices[gl_InstanceID] * vec4(position, 1.0);
    } else {
        gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
    }
}