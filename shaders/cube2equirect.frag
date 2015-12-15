#version 150

#define M_PI 3.1415926535897932384626433832795

uniform sampler2D cubeLeftImage;
uniform sampler2D cubeRightImage;
uniform sampler2D cubeBottomImage;
uniform sampler2D cubeTopImage;
uniform sampler2D cubeBackImage;
uniform sampler2D cubeFrontImage;

in vec2 vTexCoord;

out vec4 FragColor;

void main() {
	float theta = vTexCoord.x * M_PI;
	float phi = (vTexCoord.y * M_PI) / 2.0;

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
			src = texture(cubeLeftImage, px);
		}
		else {
			scale = 1.0 / x;
			px.x = (-z*scale + 1.0) / 2.0;
			px.y = ( y*scale + 1.0) / 2.0;
			src = texture(cubeRightImage, px);
		}
	}
	else if (abs(y) >= abs(z)) {
		if (y < 0.0) {
			scale = -1.0 / y;
			px.x = ( x*scale + 1.0) / 2.0;
			px.y = ( z*scale + 1.0) / 2.0;
			src = texture(cubeTopImage, px);
		}
		else {
			scale = 1.0 / y;
			px.x = ( x*scale + 1.0) / 2.0;
			px.y = (-z*scale + 1.0) / 2.0;
			src = texture(cubeBottomImage, px);
		}
	}
	else {
		if (z < 0.0) {
			scale = -1.0 / z;
			px.x = (-x*scale + 1.0) / 2.0;
			px.y = ( y*scale + 1.0) / 2.0;
			src = texture(cubeBackImage, px);
		}
		else {
			scale = 1.0 / z;
			px.x = ( x*scale + 1.0) / 2.0;
			px.y = ( y*scale + 1.0) / 2.0;
			src = texture(cubeFrontImage, px);
		}
	}

	FragColor = src;
}
