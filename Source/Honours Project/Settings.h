#ifndef SETTINGS_H
#define SETTINGS_H

#define FULLSCREEN	0
#define WINDOWED	1
#define BORDERLESS	2

class Settings
{
public:
	Settings();
	static void setWindowWidth(int x) { windowWidth = x; }
	static void setWindowHeight(int x) { windowHeight = x; }
	static void setWindowState(int x) { windowState = x; }
	static void setMaxFPS(int x) { maxFPS = x; }
	static void setMaxParticles(int x) { maxParticles = x; }

	static int getWindowWidth() { return windowWidth; }
	static int getWindowHeight() { return windowHeight; }
	static int getWindowState() { return windowState; }
	static int getMaxFPS() { return maxFPS; }
	static int getMaxParticles() { return maxParticles; }
private:
	static int windowWidth;
	static int windowHeight;
	static int windowState;
	static int maxFPS;
	static int maxParticles;
};

#endif