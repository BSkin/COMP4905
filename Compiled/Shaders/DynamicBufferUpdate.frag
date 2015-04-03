#version 130

uniform float elapsedTime; //Since last frame
uniform float totalElapsedTime;
uniform float spawnRate;
uniform float speed;
uniform vec3 spawnerBounds;
uniform vec3 spawnerDir;
uniform mat4 particleMatrix;
uniform sampler2D depthSampler;
uniform sampler2D positionSampler;

in vec2 uvVar;

out vec4 outColour;

uint random(uint seed) 
{
	uint sum = 0u;
	uint delta = 0x9e3779b9u;

	uint result = 0u;
	for (int n = 0; n < 3; n++) {
		sum = (delta + sum) % 10000000u;
		result = ( ((seed << 4) + 0xa341316cu) ^ (seed + sum) ^ ((seed >> 5) + 0xc8013ea4u) + result) % 10000000u;
	}

	return result;
}

vec4 createParticle()
{
	uint seed = uint(uvVar.x*0x294ab15c + uvVar.y*0x12dfa762 + totalElapsedTime);

	vec4 newPosition;

	seed = random(seed);
	newPosition.x = (seed % uint(spawnerBounds.x*1000.0f)) * 0.001f - spawnerBounds.x / 2.0f;
	newPosition.x += (spawnerDir.x/spawnerDir.y)*spawnerBounds.y;

	newPosition.y = spawnerBounds.y;

	seed = random(seed);	
	newPosition.z = (seed % uint(spawnerBounds.z*1000.0f)) * 0.001f - spawnerBounds.z / 2.0f;
	newPosition.z += (spawnerDir.z/spawnerDir.y)*spawnerBounds.y;

	newPosition.w = 1.0f;
	
	return newPosition;
}

bool spawnParticle()
{
	uint seed = uint(uvVar.x*0x3ba789ca + uvVar.y*0x224a7f32 + totalElapsedTime);

	seed = random(seed);
	return ((seed % 10000000u) < uint(0.5f+100.0f*spawnRate*elapsedTime));
}

void main()
{
	vec4 worldPos = texture(positionSampler, uvVar);	
	worldPos = ((worldPos.xyz == vec3(0.0f, 0.0f, 0.0f)) && spawnParticle()) ? createParticle() : worldPos;
	worldPos += (worldPos.xyz == vec3(0.0f, 0.0f, 0.0f)) 
		? vec4(0.0f, 0.0f, 0.0f, 0.0f) : vec4(spawnerDir, 0.0f) * elapsedTime * speed * 0.001f;

	vec4 particlePos = particleMatrix * vec4(worldPos.xyz, 1.0f);
	float x = (particlePos.x+1.0f)*0.5f;
	float y = (particlePos.y+1.0f)*0.5f;
	float z = particlePos.z;
	
	worldPos = ((x <= 1.0f && x >= 0.0f) && (y <= 1.0f && y >= 0.0f) &&
		((z+1.0f) > 2.0f*texture(depthSampler, vec2(x, y)).r))
		? vec4(0.0f, 0.0f, 0.0f, 0.0f) : worldPos;
	worldPos = ((z+1.0f) > 0.75f) ? vec4(0.0f, 0.0f, 0.0f, 0.0f) : worldPos;

	outColour = vec4(worldPos.xyz, 1.0f);
}