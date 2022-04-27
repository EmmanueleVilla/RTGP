#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 UV;

//*** "BASIC" INPUT FROM APP ***//
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

//--- UBO INPUT FROM APP
layout (std140) uniform Matrices {
    //--- HANDLING A MAXIMUM OF 1024 ELEMENTS
    mat4 modelMatrices[1024];
};

//--- OUTPUT TO FRAGMENT SHADER
out vec2 interp_UV;

//--- SUBROUTINES
subroutine vec4 vertshader();
subroutine uniform vertshader vertshaderImpl;

subroutine(vertshader)
vec4 standard() {
    return projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}

subroutine(vertshader)
vec4 instanced() {
    return projectionMatrix * viewMatrix * modelMatrices[gl_InstanceID] * vec4(position, 1.0);
}

void main() {
    interp_UV = UV;
    gl_Position = vertshaderImpl();
}
