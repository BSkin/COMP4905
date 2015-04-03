#version 130

uniform mat4 worldViewProj;

attribute vec2 uv;
attribute vec3 vertex;

out vec2 uvVar;

void main()
{	
	uvVar = uv;
	gl_Position = worldViewProj * vec4(vertex, 1.0f);
}