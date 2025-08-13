#version 330 core
uniform vec3 stretchColor;

out vec4 FragColor;

void main()
{    
    FragColor = vec4(stretchColor, 1.0f);
}