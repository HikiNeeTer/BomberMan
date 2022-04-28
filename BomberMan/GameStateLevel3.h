#ifndef GAME_STATE_LEVEL3
#define GAME_STATE_LEVEL3

#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
extern GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>

// ---------------------------------------------------------------------------

void GameStateLevel3Load(void);
void GameStateLevel3Init(void);
void GameStateLevel3Update(double dt);
void GameStateLevel3Draw(void);
void GameStateLevel3Free(void);
void GameStateLevel3Unload(void);

// ---------------------------------------------------------------------------

#endif // GAME_STATE_LEVEL3