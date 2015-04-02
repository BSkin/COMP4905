#include "Game.h"

AssetManager * Game::assetManager = NULL;
using namespace Utilities;

Game::Game(char* gameName)
{
	title[127] = '\0';
	if (title == NULL) this->title[0] = '\0';
	else strncpy(this->title, gameName,127);
	frameRate = 0;
	activeWindow = false;
	running = true;

	bits = 32;
	flags = 0;
	title[128];
	ignoreShaders = false;
	
	displayWindow =	NULL;
	displayContext = NULL;
	assetLoaderThread = NULL;

	state = 0;

	tickindex=ticksum=0;
	for (int i = 0; i < MAXFRAMESAMPLES; i++) ticklist[i] = 0;
}

Game::~Game()
{
	
}

void Game::loadConfig()
{
	SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, false};
	CreateDirectory("Config", &sa);

	FILE * file;
	char * configName = "Config\\settings.ini";
	file = fopen(configName, "r");
	if (file == NULL) return; //no file found, just use defaults

	ifstream fileStream(configName);
	string s;
	string var, val;
	while (fileStream.peek() != -1)
	{
		s = var = val = "";
		getline(fileStream, s);
		bool eq = false;
		int i = 0;

		while (i < s.size())
		{
			if (s[i] == '=')	eq = true;
			else if (eq)		val += s[i];
			else				var += s[i];
			i++;
		}

		if (var == "maxFps")			Settings::setMaxFPS(atoi(val.c_str()));
		else if (var == "windowWidth")	Settings::setWindowWidth(atoi(val.c_str()));
		else if (var == "windowHeight")	Settings::setWindowHeight(atoi(val.c_str()));
		else if (var == "windowState")	Settings::setWindowState(atoi(val.c_str()));
		else if (var == "maxParticles")	Settings::setMaxParticles(atoi(val.c_str()));
	}

	//fclose(file);
}

void Game::saveConfig()
{
	ofstream output;
	char * configName = "Config\\settings.ini";
	output.open(configName);
		
	const unsigned char * ver = glGetString(GL_VERSION);
	string v = "";
	int i = 0;
	while (ver[i] != ' ') { v += ver[i]; i++; }

	float version = atof(v.c_str());

	output 
		<< "maxFps=" << Settings::getMaxFPS() << '\n'
		<< "windowWidth=" << Settings::getWindowWidth() << '\n'
		<< "windowHeight=" << Settings::getWindowHeight() << '\n'
		<< "windowState=" << Settings::getWindowState() << '\n'
		<< "maxParticles=" << Settings::getMaxParticles();

	output.close();
}

int Game::drawTTFText(int x, int y, const char * s, TTF_Font * font, glm::mat4 * projViewMatrix)
{
	if (font == 0) return -1;

	SDL_Color clrFg = {255,255,255,255};  // Blue ("Fg" is foreground)
	SDL_Surface * sText = TTF_RenderText_Blended( font, s, clrFg );

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sText->w, sText->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, sText->pixels);


	activeShader->setUniformf1("enableLighting", 0.0f);
	glActiveTexture(GL_TEXTURE0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glm::mat4 scale = glm::scale(glm::vec3(sText->w, sText->h, 0.0f));
	glm::mat4 translation = glm::translate(glm::vec3((-x) + Settings::getWindowWidth()/2 - sText->w/2, (-y) + Settings::getWindowHeight()/2 - sText->h/2, 0));

	glm::mat4 matrix = *projViewMatrix * translation * scale;
	activeShader->setUniformMatrixf4("worldViewProj", matrix);
	sdlquad->render();

	glDeleteTextures(1, &texture);
	SDL_FreeSurface( sText );

	return 0;
}

int Game::init()
{
	loadConfig();
	initSDL();
	initGlew();
	initObjects();
	time = 0;

	return 0;
}

int Game::initSDL()
{
	SDL_Init(SDL_INIT_EVERYTHING);

	Uint32 windowFlags;

	if (Settings::getWindowState() == FULLSCREEN)		windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN;
	else if (Settings::getWindowState() == WINDOWED)	windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;
	else if (Settings::getWindowState() == BORDERLESS) windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_BORDERLESS;

	displayWindow = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Settings::getWindowWidth(), Settings::getWindowHeight(), windowFlags);
	
	displayContext = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, displayContext); 
	
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY); 

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, bits);
	
    initOpenGL();
	resizeWindow(Settings::getWindowWidth(), Settings::getWindowHeight());

	SDL_SetRelativeMouseMode(SDL_TRUE);
    
	TTF_Init();
	loadTTFFonts();

	return 0;
}

int Game::loadTTFFonts()
{
	fontArial = TTF_OpenFont("Fonts\\arial.ttf", 24);

	return 0;
}

int Game::initOpenGL()
{
	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping ( NEW )
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);  // clockwise oriented polys are front faces
	glCullFace(GL_BACK); // cull out the inner polygons... Set Culling property appropriately

	glShadeModel( GL_SMOOTH );							// Enable smooth shading
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );				// Set the background black
	glClearDepth( 1.0f );								// Depth buffer setup
	glEnable( GL_DEPTH_TEST );							// Enables Depth Testing
	glDepthFunc( GL_LEQUAL );							// The Type Of Depth Test To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);			// filled polys
	
	return 0;
}

int Game::initObjects()
{
	assetManager = new AssetManager();
	assetManager->setStatics(&diplayWindow, &loadingThreadContext, &primaryContext);
	assetLoaderThread = SDL_CreateThread(startAssetLoader, "AssetLoaderThread", (void*)NULL);
	inputManager.init(&time);	

	ParticleSystem::setStatics(assetManager);

	tempFrameBuffer = new FrameBuffer(1024, 1024);
	tempFrameBuffer->addDepthTexture();

	camera = Camera();
	camera.activate();

	renderObject = true;
	particleToggle = false;

	avgFPS = 0;
	trackFPS = false;

	return 0;
}

int Game::cleanupObjects()
{
	delete tempFrameBuffer;
	delete precipitation;

	assetManager->shutdown();
	SDL_WaitThread(assetLoaderThread, NULL);
	delete assetManager;
	return 0;
}

int Game::initGlew()
{
	static bool alreadyExecuted = false;
	if (alreadyExecuted) return -1;
	alreadyExecuted = true;

	const bool loggingExtensions = true; //Set to true to have extensions logged...
	char *extensions = (char *) glGetString (GL_EXTENSIONS);

	//bool isARBVertexBufferObjectExtensionPresent = isExtensionSupported (extensions, "GL_ARB_vertex_buffer_object");
	//if (isARBVertexBufferObjectExtensionPresent) {
		glBindBufferARB = (PFNGLBINDBUFFERARBPROC) wglGetProcAddress ("glBindBufferARB");
		glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) wglGetProcAddress ("glDeleteBuffersARB");
		glGenBuffersARB = (PFNGLGENBUFFERSARBPROC) wglGetProcAddress ("glGenBuffersARB");
		glIsBufferARB = (PFNGLISBUFFERARBPROC) wglGetProcAddress ("glIsBufferARB");
		glBufferDataARB = (PFNGLBUFFERDATAARBPROC) wglGetProcAddress ("glBufferDataARB");
		glBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC) wglGetProcAddress ("glBufferSubDataARB");
		glGetBufferSubDataARB = (PFNGLGETBUFFERSUBDATAARBPROC) wglGetProcAddress ("glGetBufferSubDataARB");
		glMapBufferARB = (PFNGLMAPBUFFERARBPROC) wglGetProcAddress ("glMapBufferARB");
		glUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC) wglGetProcAddress ("glUnmapBufferARB");
		glGetBufferParameterivARB = (PFNGLGETBUFFERPARAMETERIVARBPROC) wglGetProcAddress ("glGetBufferParameterivARB");
		glGetBufferPointervARB = (PFNGLGETBUFFERPOINTERVARBPROC) wglGetProcAddress ("glGetBufferPointervARB");
	//}
	
	//bool isGL_ARB_shader_objectsExtensionPresent = isExtensionSupported (extensions, "GL_ARB_shader_objects");
	//if (isGL_ARB_shader_objectsExtensionPresent) {
		glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)wglGetProcAddress ("glAttachObjectARB");
		glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)wglGetProcAddress ("glCompileShaderARB");
		glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)wglGetProcAddress ("glCreateProgramObjectARB");
		glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)wglGetProcAddress ("glCreateShaderObjectARB");
		glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)wglGetProcAddress ("glDeleteObjectARB");
		glDetachObjectARB = (PFNGLDETACHOBJECTARBPROC)wglGetProcAddress ("glDetachObjectARB");
		glGetActiveUniformARB = (PFNGLGETACTIVEUNIFORMARBPROC)wglGetProcAddress ("glGetActiveUniformARB");
		glGetAttachedObjectsARB = (PFNGLGETATTACHEDOBJECTSARBPROC)wglGetProcAddress ("glGetAttachedObjectsARB");
		glGetHandleARB = (PFNGLGETHANDLEARBPROC)wglGetProcAddress ("glGetHandleARB");
		glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)wglGetProcAddress ("glGetInfoLogARB");
		glGetObjectParameterfvARB = (PFNGLGETOBJECTPARAMETERFVARBPROC)wglGetProcAddress ("glGetObjectParameterfvARB");
		glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)wglGetProcAddress ("glGetObjectParameterivARB");
		glGetShaderSourceARB = (PFNGLGETSHADERSOURCEARBPROC)wglGetProcAddress ("glGetShaderSourceARB");
		glGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC)wglGetProcAddress ("glGetUniformLocationARB");
		glGetUniformfvARB = (PFNGLGETUNIFORMFVARBPROC)wglGetProcAddress ("glGetUniformfvARB");
		glGetUniformivARB = (PFNGLGETUNIFORMIVARBPROC)wglGetProcAddress ("glGetUniformivARB");
		glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)wglGetProcAddress ("glLinkProgramARB");
		glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)wglGetProcAddress ("glShaderSourceARB");
		glUniform1fARB = (PFNGLUNIFORM1FARBPROC)wglGetProcAddress ("glUniform1fARB");
		glUniform1fvARB = (PFNGLUNIFORM1FVARBPROC)wglGetProcAddress ("glUniform1fvARB");
		glUniform1iARB = (PFNGLUNIFORM1IARBPROC)wglGetProcAddress ("glUniform1iARB");
		glUniform1ivARB = (PFNGLUNIFORM1IVARBPROC)wglGetProcAddress ("glUniform1ivARB");
		glUniform2fARB = (PFNGLUNIFORM2FARBPROC)wglGetProcAddress ("glUniform2fARB");
		glUniform2fvARB = (PFNGLUNIFORM2FVARBPROC)wglGetProcAddress ("glUniform2fvARB");
		glUniform2iARB = (PFNGLUNIFORM2IARBPROC)wglGetProcAddress ("glUniform2iARB");
		glUniform2ivARB = (PFNGLUNIFORM2IVARBPROC)wglGetProcAddress ("glUniform2ivARB");
		glUniform3fARB = (PFNGLUNIFORM3FARBPROC)wglGetProcAddress ("glUniform3fARB");
		glUniform3fvARB = (PFNGLUNIFORM3FVARBPROC)wglGetProcAddress ("glUniform3fvARB");
		glUniform3iARB = (PFNGLUNIFORM3IARBPROC)wglGetProcAddress ("glUniform3iARB");
		glUniform3ivARB = (PFNGLUNIFORM3IVARBPROC)wglGetProcAddress ("glUniform3ivARB");
		glUniform4fARB = (PFNGLUNIFORM4FARBPROC)wglGetProcAddress ("glUniform4fARB");
		glUniform4fvARB = (PFNGLUNIFORM4FVARBPROC)wglGetProcAddress ("glUniform4fvARB");
		glUniform4iARB = (PFNGLUNIFORM4IARBPROC)wglGetProcAddress ("glUniform4iARB");
		glUniform4ivARB = (PFNGLUNIFORM4IVARBPROC)wglGetProcAddress ("glUniform4ivARB");
		glUniformMatrix2fvARB = (PFNGLUNIFORMMATRIX2FVARBPROC)wglGetProcAddress ("glUniformMatrix2fvARB");
		glUniformMatrix3fvARB = (PFNGLUNIFORMMATRIX3FVARBPROC)wglGetProcAddress ("glUniformMatrix3fvARB");
		glUniformMatrix4fvARB = (PFNGLUNIFORMMATRIX4FVARBPROC)wglGetProcAddress ("glUniformMatrix4fvARB");
		glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)wglGetProcAddress ("glUseProgramObjectARB");
		glValidateProgramARB = (PFNGLVALIDATEPROGRAMARBPROC)wglGetProcAddress ("glValidateProgramARB");

		// Other Shader Stuff
		glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress ("glCompileShader");
		glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress ("glCreateProgram");
		glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress ("glCreateShader");
		glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress ("glDeleteProgram");
		glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress ("glDeleteShader");
		glDetachShader = (PFNGLDETACHSHADERPROC)wglGetProcAddress ("glDetachShader");
		glGetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC)wglGetProcAddress ("glGetAttachedShaders");
		glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress ("glGetUniformLocation");
		glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress ("glUniform1f");
		glUniform2f = (PFNGLUNIFORM2FPROC)wglGetProcAddress ("glUniform2f");
		glUniform3f = (PFNGLUNIFORM3FPROC)wglGetProcAddress ("glUniform3f");
		glUniform4f = (PFNGLUNIFORM4FPROC)wglGetProcAddress ("glUniform4f");
		glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress ("glUniform1i");		
	//}
	glewInit();

	return 0;
}

int Game::loadLevel(string input)
{
	particleToggle = false;
	precipitation = new StaticBufferParticleSystem();
	precipitation->setSpawnRate(0.0f);
	precipitation->updateDirection(0.0f, -1.0f, 0.0f);

	return 0;
}

int Game::start()
{
	double elapsedTime = 0.0;
	double timePerFrame = 1000.0 / Settings::getMaxFPS();
	long lastTime = 0;
	
	double targetSleep = 0.0;
	double sleepRemainder = 0.0;
	double startSleep = 0.0;
	double sleepTime = 0.0;

	assetManager->forceLoadModel("Models\\quad.obj");
	quad = assetManager->getModel("Models\\quad.obj");
	assetManager->forceLoadModel("Models\\sdlquad.obj");
	sdlquad = assetManager->getModel("Models\\sdlquad.obj");
	assetManager->forceLoadModel("Models\\house.obj");
	house = assetManager->getModel("Models\\house.obj");
	assetManager->forceLoadModel("Models\\umbrella.obj");
	umbrella = assetManager->getModel("Models\\umbrella.obj");
		
	assetManager->forceLoadTexture("Textures\\floor.tga");
	assetManager->forceLoadTexture("Textures\\house.tga");
	assetManager->forceLoadTexture("Textures\\umbrella.tga");
	umbrellaTexture = assetManager->getTexture("Textures\\umbrella.tga");
	houseTexture = assetManager->getTexture("Textures\\house.tga");
	floorTexture = assetManager->getTexture("Textures\\floor.tga");

	assetManager->forceLoadShader("Shaders\\textShader.glsl");
	textShader = assetManager->getShader("Shaders\\textShader.glsl");
	assetManager->forceLoadShader("Shaders\\depthShader.glsl");
	depthShader = assetManager->getShader("Shaders\\depthShader.glsl");
	assetManager->forceLoadShader("Shaders\\floorShader.glsl");
	floorShader = assetManager->getShader("Shaders\\floorShader.glsl");
	assetManager->forceLoadShader("Shaders\\defaultShader.glsl");
	defaultShader = assetManager->getShader("Shaders\\defaultShader.glsl");
	defaultShader->activate();
		
	loadLevel("");
		
	while(running)
	{
		time++;
		lastTime = SDL_GetTicks();

		inputManager.clearTempValues();
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT)	return 1;
			else if (e.type == SDL_MOUSEBUTTONDOWN) inputManager.mouseKeyDown(e.button.button);
			else if (e.type == SDL_MOUSEBUTTONUP)	inputManager.mouseKeyUp(e.button.button);
			else if (e.type == SDL_KEYDOWN)			inputManager.keyDown(e.key.keysym.sym);
			else if (e.type == SDL_KEYUP)			inputManager.keyUp(e.key.keysym.sym);
			else if (e.type == SDL_MOUSEMOTION)		inputManager.updateMouse(e.motion.xrel, e.motion.yrel, e.motion.x, e.motion.y); 
			else if (e.type == SDL_MOUSEWHEEL)		{ inputManager.updateScroll(e.wheel.y); }
			else {}
		}
		
		if (update(elapsedTime+sleepTime, SDL_GetTicks()) == -1) break;
		renderFrame(elapsedTime+sleepTime, SDL_GetTicks());
		
		double averageElapsedTime = calcFps(elapsedTime+sleepTime);
		frameRate = 1000.0/averageElapsedTime;

		//Framerate Limit Calculations
		elapsedTime = SDL_GetTicks() - lastTime;
		targetSleep = timePerFrame - elapsedTime + sleepRemainder;
		if (targetSleep > 0) sleepRemainder = targetSleep - (Uint32)targetSleep;

		startSleep = SDL_GetTicks();
		while ((Uint32)(startSleep+targetSleep) > SDL_GetTicks());
		sleepTime = SDL_GetTicks() - startSleep;
	}
    return 0;
}

int Game::startAssetLoader(void * data)
{
	assetManager->startAssetLoader();
	return 0;
}

int Game::cleanup()
{
	cleanupObjects();
	defaultShader = 0;
	quad = 0;

	saveConfig();
	
	killSDL();

	/*#if DEBUG
		_CrtDumpMemoryLeaks();
	#endif*/

	PostQuitMessage(0);
	return 0;
}

int Game::killSDL() 
{
	deleteTTFFonts();
	TTF_Quit();

	wglDeleteContext(loadingThreadContext);
	SDL_GL_DeleteContext(displayContext);
    SDL_DestroyWindow(displayWindow);
	SDL_Quit();

	return 0;
}

int Game::deleteTTFFonts()
{
	TTF_CloseFont(fontArial);

	return 0;
}

int Game::resizeWindow(int width, int height) {
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);
	if (width == desktop.right && height == desktop.bottom)
		width++;
	
	SDL_SetWindowSize(displayWindow, width, height); 

	//Setup a new viewport.
	glViewport (0, 0, width, height);
	Settings::setWindowWidth(width); 
	Settings::setWindowHeight(height);

	//Setup a new perspective matrix.
	GLdouble verticalFieldOfViewInDegrees = 40;
	GLdouble aspectRatio = height == 0 ? 1.0 : (GLdouble) width / (GLdouble) height;
	GLdouble nearDistance = 1.0;
	GLdouble farDistance = 2000.0;

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (verticalFieldOfViewInDegrees, aspectRatio, nearDistance, farDistance);

	//Get back to default mode.
	glMatrixMode (GL_MODELVIEW);

	return 0;
}

double Game::calcFps(double newtick)
{
	ticksum-=ticklist[tickindex];  /* subtract value falling off */
    ticksum+=newtick;              /* add new value */
    ticklist[tickindex]=newtick;   /* save new value so it can be subtracted later */
    if(++tickindex==MAXFRAMESAMPLES)    /* inc buffer index */
        tickindex=0;

    /* return average */
    return((ticksum)/MAXFRAMESAMPLES);
}

int Game::update(long elapsedTime, long totalElapsedTime)
{
	if (isKeyPressed(IM_ESCAPE)) return -1;

	camera.turn(inputManager.getMouseMovement());
	if (isKeyDown(IM_W)) camera.moveForward(elapsedTime*0.01f);
	if (isKeyDown(IM_S)) camera.moveBack(elapsedTime*0.01f);
	if (isKeyDown(IM_A)) camera.moveLeft(elapsedTime*0.01f);
	if (isKeyDown(IM_D)) camera.moveRight(elapsedTime*0.01f);
	if (isKeyDown(IM_SPACE)) camera.moveUp(elapsedTime*0.01f);
	if (isKeyDown(IM_LCTRL)) camera.moveDown(elapsedTime*0.01f);

	if (isKeyPressed(IM_P)) renderObject = !renderObject;
	if (isKeyPressed(IM_M)) toggleParticleSystem();

	if (isKeyDown(IM_UP)) precipitation->modSpawnRate(0.05f * elapsedTime);
	if (isKeyDown(IM_DOWN)) precipitation->modSpawnRate(-0.05f * elapsedTime);
	if (isKeyDown(IM_RIGHT)) precipitation->modSpawnRate(0.0005f * elapsedTime);
	if (isKeyDown(IM_LEFT)) precipitation->modSpawnRate(-0.0005f * elapsedTime);

	if (isKeyDown(IM_M1)) precipitation->updateDirection(camera.getLookAtVector());

	if (inputManager.isMouseScrollUp()) precipitation->modSpeed(0.1f);
	if (inputManager.isMouseScrollDown()) precipitation->modSpeed(-0.1f);

	if (isKeyPressed(IM_L)) toggleFPSTracking();

	if (trackFPS) {
		fpsList.push_back(frameRate);
	}

	return 0;
}


void Game::toggleFPSTracking()
{
	trackFPS = !trackFPS;

	if (trackFPS) {
		fpsList = list<double>();
		avgFPS = 0.0f;
	}
	else {
		list<double>::iterator iter;
		double total = 0.0f;
		for (iter = fpsList.begin(); iter != fpsList.end(); iter++) {
			total += *iter;
		}
		avgFPS = total / fpsList.size();
	}
}

void Game::toggleParticleSystem()
{
	if (precipitation == NULL) return;
	
	delete precipitation;
	particleToggle = !particleToggle;

	if (particleToggle)
		precipitation = new DynamicBufferParticleSystem();
	else
		precipitation = new StaticBufferParticleSystem();

	precipitation->setSpawnRate(0.0f);
	precipitation->updateDirection(0.0f, -1.0f, 0.0f);
}

int Game::renderFrame(long elapsedTime, long totalElapsedTime) {
	//Depth Prepass
	FrameBuffer::setActiveFrameBuffer(tempFrameBuffer);
	glClearColor (0.3, 0.3, 0.8, 1.0);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glActiveTexture(GL_TEXTURE0);

	depthShader->activate();
	
	calculateOrthographicProjection(&projection, 25.0f, 25.0f, 0.0f, 100.0f);
	calculateView(&view, precipitation->getPosition(), glm::vec3(0,0,0), precipitation->getUp());
	glm::mat4 particleMatrix = projection * view;

	glm::mat4 worldMatrix, normalMatrix;
	if (renderObject) worldMatrix = projection * view * glm::rotate(glm::mat4(), totalElapsedTime*0.01f, glm::vec3(0,1,0)) * glm::translate(glm::vec3(0,2,5));
	else worldMatrix = projection * view;
	activeShader->setUniformMatrixf4("worldViewProj", worldMatrix);
	
	//Render all rain collision objects
	if (renderObject) umbrella->render();
	else house->render();

	worldMatrix = projection * view * glm::rotate(90.0f, glm::vec3(1,0,0)) * glm::scale(glm::vec3(25.0f, 25.0f, 1.0f));
	activeShader->setUniformMatrixf4("worldViewProj", worldMatrix);
	quad->render();

	//Update Particles
	glActiveTexture(GL_TEXTURE0);
	tempFrameBuffer->getDepthTexture()->bindTexture();
	precipitation->update(frameRate, SDL_GetTicks(), tempFrameBuffer->getDepthTexture(), &particleMatrix);

	//Normal Rendering
	FrameBuffer::setActiveFrameBuffer(0);
	glClearColor (0.1, 0.1, 0.3, 1.0);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
	defaultShader->activate();

	camera.activate();
	activeCamera->calculateView();
	activeCamera->calculateProjection();

	if (renderObject) worldMatrix = glm::rotate(glm::mat4(), totalElapsedTime*0.01f, glm::vec3(0,1,0)) * glm::translate(glm::vec3(0,2,5));
	else worldMatrix = glm::mat4();
	normalMatrix = glm::transpose(glm::inverse(worldMatrix));
	worldMatrix = *activeCamera->getProjection() * *activeCamera->getView() * worldMatrix;
	activeShader->setUniformMatrixf4("worldViewProj", worldMatrix);
	activeShader->setUniformMatrixf4("normalMatrix", normalMatrix);
	activeShader->setUniformf1("enableLighting", 1.0f);
	glActiveTexture(GL_TEXTURE0);

	if (renderObject) {
		umbrellaTexture->bindTexture();
		umbrella->render();
	}
	else {
		houseTexture->bindTexture();
		house->render();
	}

	floorShader->activate();
	glActiveTexture(GL_TEXTURE0);
	tempFrameBuffer->getDepthTexture()->bindTexture();
	glActiveTexture(GL_TEXTURE1);
	floorTexture->bindTexture();
	glUniform1i(glGetUniformLocation(activeShader->getShaderHandle(), "depthSampler"), 0);
	glUniform1i(glGetUniformLocation(activeShader->getShaderHandle(), "colourSampler"), 1);

	worldMatrix = glm::rotate(90.0f, glm::vec3(1,0,0)) * glm::scale(glm::vec3(25.0f, 25.0f, 1.0f));
	normalMatrix = glm::transpose(glm::inverse(worldMatrix));
	glm::mat4 worldViewProj = *activeCamera->getProjection() * *activeCamera->getView() * worldMatrix;
	
	activeShader->setUniformMatrixf4("worldViewProj", worldViewProj);
	activeShader->setUniformMatrixf4("normalMatrix", normalMatrix);
	activeShader->setUniformMatrixf4("worldMatrix", worldMatrix);
	activeShader->setUniformf1("enableLighting", 1.0f);
	activeShader->setUniformMatrixf4("particleMatrix", particleMatrix);
	quad->render();
	
	defaultShader->activate();
	if (particleToggle) ((DynamicBufferParticleSystem*)(precipitation))->drawQuad();
	glActiveTexture(GL_TEXTURE0);
	tempFrameBuffer->getDepthTexture()->bindTexture();
	precipitation->render(totalElapsedTime);

	render2D(time);

	SDL_GL_SwapWindow(displayWindow);
	return 0;
}

void Game::drawRect(float x, float y, float z)
{
	glm::mat4 world;
	world = glm::translate(glm::vec3(x,y,z));
	glm::mat4 matrix = *activeCamera->getProjection() * *activeCamera->getView() * world;
	glm::mat4 normalMatrix = glm::transpose(glm::inverse(world));
	activeShader->setUniformMatrixf4("worldViewProj", matrix);
	activeShader->setUniformMatrixf4("normalMatrix", normalMatrix);

	quad->render();
}

void Game::drawRect(glm::vec3 direction, glm::vec3 position, glm::vec3 up, glm::vec3 size)
{
	glm::mat4 world, scale;
	world = glm::inverse((glm::lookAt(position, position+direction, up)));
	scale = glm::scale(size);
	glm::mat4 matrix = *activeCamera->getProjection() * *activeCamera->getView() * world * scale;
	glm::mat4 normalMatrix = glm::transpose(glm::inverse(world));
	activeShader->setUniformMatrixf4("worldViewProj", matrix);
	activeShader->setUniformMatrixf4("normalMatrix", normalMatrix);

	quad->render();
}

void Game::drawRect(float x, float y, float z, float rotation)
{
	glm::mat4 world;
	world = glm::translate(glm::vec3(x,y,z));
	world = glm::rotate(world, rotation, glm::vec3(0,1,0));
	glm::mat4 matrix = *activeCamera->getProjection() * *activeCamera->getView() * world;
	glm::mat4 normalMatrix = glm::transpose(glm::inverse(world));
	activeShader->setUniformMatrixf4("worldViewProj", matrix);
	activeShader->setUniformMatrixf4("normalMatrix", normalMatrix);

	quad->render();
}

int Game::render2D(long time)
{

	glDepthFunc(GL_ALWAYS);

	glm::mat4 proj, view;
	calculateOrthographicProjection(&proj, Settings::getWindowWidth(), Settings::getWindowHeight(), -100.0f, 100.0f);
	calculateView(&view, glm::vec3(0,0,-1), glm::vec3(0,0,0), glm::vec3(0,1,0));
	glm::mat4 viewProj = proj * view;

	textShader->activate();
	activeShader->setUniformf1("screenWidth", (float)Settings::getWindowWidth());
	activeShader->setUniformf1("screenHeight", (float)Settings::getWindowHeight());

	stringstream ss; ss.str(string());
	ss << "FPS: " << (int)(frameRate+0.5f);
	drawTTFText(10, 10, ss.str().c_str(), fontArial, &viewProj);

	ss.str(string());
	ss << "Speed: " << precipitation->getSpeed();
	drawTTFText(135, 10, ss.str().c_str(), fontArial, &viewProj);

	if (particleToggle) {
		ss.str(string());
		ss << "Spawn Rate: " << precipitation->getSpawnRate();
		drawTTFText(310, 10, ss.str().c_str(), fontArial, &viewProj);
	}

	if (!trackFPS) {
		ss.str(string());
		ss << "Avg FPS: " << avgFPS;
		drawTTFText(500, 10, ss.str().c_str(), fontArial, &viewProj);
	}

	glDepthFunc(GL_LEQUAL);

	return 0;
}