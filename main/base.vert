#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

uniform float weight;
uniform float timer;

out vec3 N;

void main() {

    float displacement = weight * sin(timer) + weight;
    vec3 newPos = position + displacement * normal;
    //flatten the model
    /*
    vec3 flattened = position;
    flattened.z = 0.0f;
    gl_Position = projectionMatrix * viewMatrix * modelMatrix* vec4(flattened, 1.0);
    */

    //save normals as out
    N = normalize(normalMatrix * normal);

    gl_Position = projectionMatrix * viewMatrix * modelMatrix* vec4(newPos, 1.0);
}