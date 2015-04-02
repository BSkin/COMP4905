#include "DynamicBufferParticleSystem.h"

DynamicBufferParticleSystem::DynamicBufferParticleSystem() : ParticleSystem()
{
	frameToggle = false;
	numParticles = Settings::getMaxParticles();
	width = height = (int)sqrt(numParticles);
	frameA = new FrameBuffer(width, height);
	frameA->addDepthBuffer();
	frameA->addColorFloatTexture();

	frameB = new FrameBuffer(width, height);
	frameB->addDepthBuffer();
	frameB->addColorFloatTexture();

	assetManager->forceLoadShader("Shaders\\DynamicBufferUpdate.glsl");
	updateShader = assetManager->getShader("Shaders\\DynamicBufferUpdate.glsl");

	assetManager->forceLoadShader("Shaders\\DynamicBufferRender.glsl");
	renderShader = assetManager->getShader("Shaders\\DynamicBufferRender.glsl");
	
	assetManager->forceLoadModel("Models\\quad.obj");
	quad = assetManager->getModel("Models\\quad.obj");

	vertexBuffer = 0;
	vector<float> vertices;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			vertices.push_back((float)x / (float)width + 0.5f / (float)width);
			vertices.push_back((float)y / (float)height + 0.5f / (float)height);
		}
	}

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), &vertices[0], GL_STATIC_DRAW);
}

DynamicBufferParticleSystem::~DynamicBufferParticleSystem()
{
	delete frameA;
	delete frameB;
	if (vertexBuffer != 0)	glDeleteBuffers(1, &vertexBuffer);
}

glm::vec3 DynamicBufferParticleSystem::getPosition() { return -direction*(25.0f-10.0f); }

void DynamicBufferParticleSystem::update(long frameRate, long totalElapsedTime, Texture * depthTexture, glm::mat4 * particleMatrix)
{
	frameToggle = !frameToggle;

	if (frameToggle)	FrameBuffer::setActiveFrameBuffer(frameA);
	else				FrameBuffer::setActiveFrameBuffer(frameB);
	glClearColor (0.5, 0.0, 0.0, 1.0);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glm::mat4 viewMatrix;
	calculateView(&viewMatrix, glm::vec3(0, 0, -0.1), glm::vec3(0,0,0), glm::vec3(0,1,0));

	glm::mat4 projMatrix;
	calculateOrthographicProjection(&projMatrix, (float)((int)sqrt(numParticles)), (float)((int)sqrt(numParticles)), 0.0f, 10.0f);

	glm::mat4 worldMatrix;
	calculateWorld(&worldMatrix, glm::vec3(0,0,0), glm::vec3(0, 0, -0.1), glm::vec3(0,1,0));

	glm::mat4 matrix = projMatrix * viewMatrix * worldMatrix * glm::scale(glm::vec3(width, height, 1.0f));
	updateShader->activate();
	activeShader->setUniformMatrixf4("worldViewProj", matrix);
	activeShader->setUniformf1("elapsedTime", 1000.0/frameRate);
	activeShader->setUniformf1("totalElapsedTime", totalElapsedTime);
	activeShader->setUniformf1("spawnRate", spawnRate);
	activeShader->setUniformf1("speed", speed);
	activeShader->setUniformMatrixf4("particleMatrix", *particleMatrix);
	activeShader->setUniformf3("spawnerBounds", 25.0f, 25.0f, 25.0f);
	glm::vec3 normalDirection = glm::normalize(direction);
	activeShader->setUniformf3("spawnerDir", normalDirection.x, normalDirection.y, normalDirection.z);

	glActiveTexture(GL_TEXTURE0);
	if (depthTexture != NULL) depthTexture->bindTexture();
	glActiveTexture(GL_TEXTURE1);
	if (frameToggle)	frameB->getTexture(0)->bindTexture();
	else				frameA->getTexture(0)->bindTexture();

	glUniform1i(glGetUniformLocation(activeShader->getShaderHandle(), "depthSampler"), 0);
	glUniform1i(glGetUniformLocation(activeShader->getShaderHandle(), "positionSampler"), 1);

	if (activeShader == NULL) return;

	quad->render();
}

void DynamicBufferParticleSystem::drawQuad()
{
	glm::mat4 viewMatrix;
	calculateView(&viewMatrix, glm::vec3(0, 0, -0.1), glm::vec3(0,0,0), glm::vec3(0,1,0));

	glm::mat4 projMatrix;
	calculateOrthographicProjection(&projMatrix, (float)Settings::getWindowWidth(), (float)Settings::getWindowHeight(), 0.0f, 10.0f);

	glm::mat4 worldMatrix;
	float scale = Settings::getWindowHeight()*0.3f;
	calculateWorld(&worldMatrix, glm::vec3(Settings::getWindowWidth()/2 - scale/2 - 20, Settings::getWindowHeight()/2  - scale/2 - 50, 0), glm::vec3(Settings::getWindowWidth()/2 - scale/2 - 20, Settings::getWindowHeight()/2  - scale/2 - 50, -0.1), glm::vec3(0,1,0));

	glm::mat4 matrix = projMatrix * viewMatrix * worldMatrix * glm::scale(glm::vec3(scale, scale, 1.0f));
	glm::mat4 normalMatrix = glm::transpose(glm::inverse(worldMatrix));
	
	activeShader->setUniformMatrixf4("worldViewProj", matrix);
	activeShader->setUniformMatrixf4("normalMatrix", normalMatrix);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(activeShader->getShaderHandle(), "texS"), 0);
	if (frameToggle)	frameA->getTexture(0)->bindTexture();
	else				frameB->getTexture(0)->bindTexture();

	if (activeShader == NULL) return;

	quad->render();
}

void DynamicBufferParticleSystem::render(long totalElapsedTime)
{
	renderShader->activate();
	
	glm::mat4 matrix = *activeCamera->getProjection() * *activeCamera->getView();
	activeShader->setUniformMatrixf4("worldViewProj", matrix);

	glActiveTexture(GL_TEXTURE0);
	if (frameToggle)	frameA->getTexture(0)->bindTexture();
	else				frameB->getTexture(0)->bindTexture();
	glUniform1i(glGetUniformLocation(activeShader->getShaderHandle(), "texS"), 0);

	activeShader->setUniformf1("totalElapsedTime", totalElapsedTime);
	glm::vec3 xDir, zDir;
	if (direction == glm::vec3(0,1,0) || direction == glm::vec3(0,-1,0)) {
		xDir = glm::vec3(1, 0, 0);
		zDir = glm::vec3(0, 0, 1);
	}
	else {
		xDir = glm::normalize(glm::cross(direction, glm::vec3(0,1,0)));
		zDir = glm::normalize(glm::cross(xDir, direction));
	}
	activeShader->setUniformf3("xDir", xDir.x, xDir.y, xDir.z);
	activeShader->setUniformf3("zDir", zDir.x, zDir.y, zDir.z);

	if (activeShader == NULL) return;

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	int uvID = glGetAttribLocation(activeShader->getShaderHandle(), "uv");
	glEnableVertexAttribArray(uvID);
	glVertexAttribPointer(
	    uvID,			// attribute
	    2,				// size
	    GL_FLOAT,		// type
	    GL_FALSE,		// normalized?
	    0,				// stride
	    (void*)0		// array buffer offset
	);

	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glDrawArrays(GL_POINTS, 0, width*height);

	glDisableVertexAttribArray(uvID);
}