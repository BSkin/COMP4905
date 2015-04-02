#ifndef UTILITIES_H
#define UTILITIES_H

#include "SDL2-2.0.3\include\SDL.h"
#include "SDL2-2.0.3\include\SDL_opengl.h"

#include <string>
#include <list>
using std::string;
using std::list;

namespace Utilities 
{
	template<typename T>
	static bool listContainsElement(list<T> * l, T element)
	{
		for (list<string>::iterator iter = l->begin(); iter != l->end(); iter++)
			if (*iter == element) return true;
		return false;
	}

	static float magSqr(glm::vec3 val) { return val.x*val.x + val.y*val.y + val.z*val.z; }
	static float magnitude(glm::vec3 val) { return sqrt(magSqr(val)); }
	static int mod(int a, int b) { return (a%b+b)%b; }
};

#endif