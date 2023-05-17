#version 330

in vec3 vertex_position;
in vec2 vertex_texcoord;

out vec2 texcoord;

void main() {
	texcoord = vertex_texcoord;
	gl_Position = vec4(vertex_position, 1.0);
}

