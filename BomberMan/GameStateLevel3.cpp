#include "GameStateLevel3.h"


void GameStateLevel3Load(void) {

	printf("Level3: Load\n");
}


void GameStateLevel3Init(void) {

	printf("Level3: Init\n");

}


void GameStateLevel3Update(double dt) {

	double fps = 1.0 / dt;
	printf("Level3: Update @> %f fps\n", fps);

}

void GameStateLevel3Draw(void) {

	printf("Level3: Draw\n");

	static float blue = 0.0f;
	blue += 0.01f;

	// Clear the screen
	glClearColor(0.0f, 0.0f, glm::abs(glm::sin(blue)), 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);





	glfwSwapBuffers(window);

}

void GameStateLevel3Free(void) {

	printf("Level3: Free\n");

}

void GameStateLevel3Unload(void) {

	printf("Level3: Unload\n");

}
