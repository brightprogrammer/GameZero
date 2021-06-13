//glsl version 4.5
#version 450

// input color
layout(location = 0) in vec3 inFragColor;

//output write
layout (location = 0) out vec4 outFragColor;

void main()
{
	outFragColor = vec4(inFragColor,1.0f);
}