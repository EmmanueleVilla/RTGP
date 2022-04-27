#version 410 core

//--- OUTPUT TO FRAME BUFFER
layout(location = 0) out vec4 color;

//--- INPUT FROM APP
uniform sampler2D tex;

void main() {
    vec4 baseColor = texture(tex, gl_PointCoord);
    color = baseColor * vec4(1.0f, 0.0f, 0.0f, 0.75f);
}