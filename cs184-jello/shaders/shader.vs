# version 460 core
layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec3 aColor;
layout (location = 1) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord;

uniform mat4 transform;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	mat4 o2w = projection * view * model;
	gl_Position = o2w * vec4(aPos, 1.0);
	TexCoord = aTexCoord;
}