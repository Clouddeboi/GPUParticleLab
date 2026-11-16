#version 430 core

in vec3 vColor;

out vec4 FragColor;

uniform float uTime;

void main()
{
    float pulse = 0.6 + 0.4 * sin(uTime * 2.0);
    FragColor = vec4(vColor * pulse, 1.0);
}