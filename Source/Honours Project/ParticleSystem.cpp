#include "ParticleSystem.h"

AssetManager * ParticleSystem::assetManager = NULL;

ParticleSystem::ParticleSystem()
{
	updateDirection(-0.5, -1, 0); 
	spawnRate = 0.0f;
	speed = 1.0f;
}

void ParticleSystem::updateDirection(glm::vec3 dir)
{
	direction = glm::normalize(dir); 
	updateUp();
}

void ParticleSystem::updateUp()
{
	if (direction == glm::vec3(0,1,0) || direction == glm::vec3(0,-1,0)) 
		up =  glm::vec3(0,0,1);
	else
		up = glm::normalize(glm::cross(glm::vec3(0,1,0), direction));
}