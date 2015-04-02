#include "StaticBufferParticleSystem.h"

#define maxX 25
#define maxY 25
#define maxZ 25

StaticBufferParticleSystem::StaticBufferParticleSystem() : ParticleSystem()
{
	vertexBuffer = 0;

	vector<float> vertices;
	srand(GetCurrentTime());

	numParticles = Settings::getMaxParticles();
	for (int i = 0; i < numParticles; i++) {
		vertices.push_back((rand() % (maxX*100)) * 0.01f - maxX * 0.5f);
		vertices.push_back((rand() % (maxZ*100)) * 0.01f - maxZ * 0.5f);
		vertices.push_back((rand() % (maxY*100)) * 0.01f);
	}

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, numParticles * 3, &vertices[0], GL_STATIC_DRAW);

	assetManager->forceLoadShader("Shaders\\StaticBuffer.glsl");
	particleShader = assetManager->getShader("Shaders\\StaticBuffer.glsl");
}

StaticBufferParticleSystem::~StaticBufferParticleSystem()
{
	if (vertexBuffer != 0)	glDeleteBuffers(1, &vertexBuffer);
}

glm::vec3 StaticBufferParticleSystem::getPosition() { 
	return -direction*(maxY-10.0f); }

void StaticBufferParticleSystem::render(long totalElapsedTime)
{
	particleShader->activate();
	worldMatrix = *activeCamera->getProjection() * *activeCamera->getView() * glm::inverse(glm::lookAt(getPosition(), glm::vec3(0,0,0), getUp()));// * glm::translate(glm::vec3(-10.0f, 0, -10.0f));// * glm::rotate(45.0f, glm::vec3(1,0,0));
	activeShader->setUniformMatrixf4("worldViewProj", worldMatrix);
	activeShader->setUniformf1("time", totalElapsedTime);
	activeShader->setUniformf1("speed", speed);
	activeShader->setUniformf3("spawnerBounds", maxX, maxY, maxZ);
	glUniform1i(glGetUniformLocation(activeShader->getShaderHandle(), "texS"), 0);

	if (activeShader == NULL) return;

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	int vertexID = glGetAttribLocation(activeShader->getShaderHandle(), "vertex");
	glEnableVertexAttribArray(vertexID);
	glVertexAttribPointer(
	   vertexID,
	   3,                  // size
	   GL_FLOAT,           // type
	   GL_FALSE,           // normalized?
	   0,                  // stride
	   (void*)0            // array buffer offset
	);

	glEnable( GL_POINT_SMOOTH );
	glEnable(GL_PROGRAM_POINT_SIZE);
	glDrawArrays(GL_POINTS, 0, numParticles);
 
	glDisableVertexAttribArray(vertexID);
}