#define GLM_FORCE_RADIANS
#define _USE_MATH_DEFINES

#include <Windows.h>

#include "glew.h"
#include <gl/GLU.h>
#include <gl/GL.h>
#include "SDL2-2.0.3\include\SDL.h"
#include "SDL2-2.0.3\include\SDL_thread.h"
#include "SDL2-2.0.3\include\SDL_opengl.h"
#include "SDL2_ttf-2.0.12\include\SDL_ttf.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"	

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>

#include "InputManager.h"
#include "Camera.h"
#include "AssetManager.h"
#include "Shader.h"
#include "Texture.h"
#include "Model.h"
#include "FrameBuffer.h"

#include "StaticBufferParticleSystem.h"
#include "DynamicBufferParticleSystem.h"

#define DEBUG 0

#if DEBUG
	#include <vld.h>	
#endif

using std::string;
using std::getline;
using std::stringstream;
using std::ostringstream;
using std::ifstream;
using std::ofstream;

class Game
{
public:
	Game(char* gameName);
	~Game(void);

	int init();
	int start();
	int cleanup();

	//static LRESULT CALLBACK globalWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
	int update(long elapsedTime, long totalElapsedTime);
	int renderFrame(long elapsedTime, long totalElapsedTime);
	int render2D(long time);
	int loadLevel(string asdf);

	void loadConfig();
	void saveConfig();
	
	int setViewport(int width, int height);
	int resizeWindow(int width, int height);

	int drawTTFText(int x, int y, const char * s, TTF_Font * font, glm::mat4 * projViewMatrix);

	int initOpenGL();
	int initGlew();
	int initSDL();
	int loadTTFFonts();
	int deleteTTFFonts();
	int initGLContexts();
	int initObjects();
	int cleanupObjects();
	int killSDL();

	int initBullet(void);
	int cleanBullet(void);
	int killBullet(void);
	int applyCustomContacts();

	//Shortcut Functions
	inline bool isKeyDown(long e) { return inputManager.isKeyDown(e); }
	inline bool isKeyUp(long e) { return inputManager.isKeyUp(e); }
	inline bool isKeyPressed(long e) { return inputManager.isKeyPressed(e); }
	inline bool isKeyReleased(long e) { return inputManager.isKeyReleased(e); }
	#define activeShader Shader::getActiveShader()
	#define activeCamera Camera::getActiveCamera()

	bool activeWindow, running;
	short bits;
	int flags;
	char title[128];
	
	HDC diplayWindow;
	HGLRC primaryContext, loadingThreadContext;
	SDL_Window * displayWindow;
	SDL_GLContext displayContext;
	SDL_Thread * assetLoaderThread;
	static int startAssetLoader(void*);

	TTF_Font * fontArial;

	//GLuint base;
	long time;
	InputManager inputManager;
	static AssetManager * assetManager;

	double frameRate;
	#define MAXFRAMESAMPLES 100
	int tickindex;
	double ticksum;
	double ticklist[MAXFRAMESAMPLES];

	double calcFps(double newtick);

	glm::mat4 view, projection;
	Camera camera;
	bool ignoreShaders;

	int state;
	Model * quad;
	Model * sdlquad;
	Model * house;
	Model * umbrella;
	Texture * umbrellaTexture;
	Texture * houseTexture;
	Texture * floorTexture;
	Shader * textShader;
	Shader * defaultShader;
	Shader * depthShader;
	Shader * floorShader;
	FrameBuffer * tempFrameBuffer;

	bool renderObject;

	double avgFPS;
	bool trackFPS;
	list<double> fpsList;
	void toggleFPSTracking();

	bool particleToggle;
	void toggleParticleSystem();
	ParticleSystem * precipitation;

	void drawRect(float x, float y, float z);
	void drawRect(float x, float y, float z, float rotation);
	void drawRect(glm::vec3 lookAt, glm::vec3 position, glm::vec3 up, glm::vec3 size);
	
	void calculateView(glm::mat4 * view, glm::vec3 position, glm::vec3 lookAt, glm::vec3 upVector)
	{ *view = glm::lookAt(position, lookAt, upVector); }
	void calculateProjection(glm::mat4 * proj, float vFoV, float aspectRatio, float nearPlane, float farPlane) 
	{ *proj = glm::perspective(vFoV, aspectRatio, nearPlane, farPlane); }
	void calculateOrthographicProjection(glm::mat4 * proj, float width, float height, float nearPlane, float farPlane)
	{ *proj = glm::ortho(-width/2.0f, width/2.0f, -height/2.0f, height/2.0f, nearPlane, farPlane); }
};