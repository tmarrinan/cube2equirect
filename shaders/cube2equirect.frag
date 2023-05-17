#version 330

#define M_PI 3.1415926535897932384626433832795

in vec2 texcoord;

uniform sampler2D cube_left;
uniform sampler2D cube_right;
uniform sampler2D cube_bottom;
uniform sampler2D cube_top;
uniform sampler2D cube_back;
uniform sampler2D cube_front;

out vec4 FragColor;

void main() {
	float theta = texcoord.x * M_PI;
	float phi = (texcoord.y * M_PI) / 2.0;

	float x = cos(phi) * sin(theta);
	float y = sin(phi);
	float z = cos(phi) * cos(theta);

	float scale;
	vec2 px;
	vec4 src;

	if (abs(x) >= abs(y) && abs(x) >= abs(z)) {
		if (x < 0.0) {
			scale = -1.0 / x;
			px.x = ( z*scale + 1.0) / 2.0;
			px.y = ( y*scale + 1.0) / 2.0;
			src = texture(cube_left, px);
		}
		else {
			scale = 1.0 / x;
			px.x = (-z*scale + 1.0) / 2.0;
			px.y = ( y*scale + 1.0) / 2.0;
			src = texture(cube_right, px);
		}
	}
	else if (abs(y) >= abs(z)) {
		if (y < 0.0) {
			scale = -1.0 / y;
			px.x = ( x*scale + 1.0) / 2.0;
			px.y = ( z*scale + 1.0) / 2.0;
			src = texture(cube_top, px);
		}
		else {
			scale = 1.0 / y;
			px.x = ( x*scale + 1.0) / 2.0;
			px.y = (-z*scale + 1.0) / 2.0;
			src = texture(cube_bottom, px);
		}
	}
	else {
		if (z < 0.0) {
			scale = -1.0 / z;
			px.x = (-x*scale + 1.0) / 2.0;
			px.y = ( y*scale + 1.0) / 2.0;
			src = texture(cube_back, px);
		}
		else {
			scale = 1.0 / z;
			px.x = ( x*scale + 1.0) / 2.0;
			px.y = ( y*scale + 1.0) / 2.0;
			src = texture(cube_front, px);
		}
	}

	FragColor = src;
}
