// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

/*
 *
 *  LevelEditor
 *
 */

// vvv must be the same as Engine's stdafx.h vvv
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

// vvv specific to LevelEditor vvv
#include "../Engine/Globals.h"
