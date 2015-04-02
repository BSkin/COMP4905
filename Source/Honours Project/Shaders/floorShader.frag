#version 130

uniform sampler2D depthSampler;
uniform sampler2D colourSampler;
uniform mat4 particleMatrix;

in float lighting;
in vec2 uvVar;
in vec3 normalVar;
in vec3 worldPos;

out vec4 outColour;

void main()
{
	vec3 sunDir = normalize(vec3(0.1f, 0.5f, 0.5f));
	float intensity = min(1.0f, max(dot(sunDir, normalVar), 0.0f) + 0.2f);

	vec4 particlePos = particleMatrix * vec4(worldPos.xyz, 1.0f);
	float x = (particlePos.x+1.0f)*0.5f;
	float y = (particlePos.y+1.0f)*0.5f;
	float z = particlePos.z;
	
	vec4 snowColour = vec4(0.65f, 0.65f, 0.65f, 1.0f);
	vec4 textureColour = texture(colourSampler, uvVar);
	vec4 combColour;

	combColour = ((x <= 1.0f && x >= 0.0f) && (y <= 1.0f && y >= 0.0f) &&
		((z+1.0f) > 2.0f*(texture(depthSampler, vec2(x, y)).r)+0.01f))// ||
		//((z+1.0f) > 0.95f))
		? textureColour : textureColour*0.0f + snowColour*1.0f;

	outColour = ((lighting >= 0.5f) ? vec4(intensity*combColour.xyz, 1.0f) : combColour);
}