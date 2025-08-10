# version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	mat4 o2w = projection * view * model;
	gl_Position = o2w * vec4(aPos, 1.0);
}