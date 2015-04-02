#version 130

uniform mat4 worldViewProj;
uniform float time;
uniform float speed;
uniform vec3 spawnerBounds;
uniform vec3 spawnerDir;
uniform sampler2D texS;

attribute vec3 vertex;

void main()
{	

	vec3 worldPos = vertex + vec3(0.0f, 0.0f, time*speed*0.0005f);
	worldPos.z = -mod(worldPos.z, spawnerBounds.y);

	//snow sway
	float offsetX = 0.1f*sin((time+17.0f*mod(vertex.x*500.0f+vertex.y*473.0f, 1000.0f))*0.003f);
	float offsetY = 0.1f*sin((time+17.0f*mod(vertex.z*500.0f+vertex.y*473.0f, 1000.0f))*0.003f);
	gl_Position = worldViewProj * vec4(worldPos + vec3(offsetX, offsetY, 0.0f), 1.0f);	

	gl_PointSize = 2.5f / sqrt(gl_Position.z);

	float maxY = spawnerBounds.y;

	vec2 uv;
	uv.x = ((worldPos.x - spawnerBounds.x) / spawnerBounds.x) - 0.5f;
	uv.y = ((worldPos.y - spawnerBounds.z) / spawnerBounds.z) - 0.5f;

	gl_Position = ((maxY + worldPos.z) < maxY - 100.0f*textureLod(texS, uv, 0.0f).r) ? vec4(0.0f, 0.0f, -1.0f, 0.0f) : gl_Position;
}