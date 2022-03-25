#version 410 core

/* INPUT FROM SHADERS */
in vec2 interp_UV;

/* OUTPUT */
out vec4 color;

/* INPUT FROM APP */
uniform sampler2D tex;

void main(){
    vec2 newUV = 2 * interp_UV.xy;  
    newUV /= vec2(1280, 720);
    newUV -= vec2(1.0, 1.0);  

    color = vec4(vec3(texture(tex, newUV).xyz), 1.0f);
}