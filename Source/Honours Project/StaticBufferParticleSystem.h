#ifndef STATIC_BUFFER_PARTICLE_SYSTEM_H
#define STATIC_BUFFER_PARTICLE_SYSTEM_H

#include "ParticleSystem.h"
#include <vector>

using std::vector;

class StaticBufferParticleSystem : public ParticleSystem
{
public:
	StaticBufferParticleSystem();
	~StaticBufferParticleSystem();
	virtual void render(long totalElapsedTime);
	virtual glm::vec3 getPosition();

private:
	GLuint vertexBuffer;
	glm::mat4 worldMatrix;
	Shader * particleShader;
};

#endif