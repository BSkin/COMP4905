#version 130

uniform mat4 worldViewProj;

attribute vec3 vertex;

void main()
{
	gl_Position = worldViewProj * vec4(vertex, 1.0f);
}