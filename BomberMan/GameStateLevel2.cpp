#include "GameStateLevel2.h"


void GameStateLevel2Load(void) {

	printf("Level2: Load\n");
}


void GameStateLevel2Init(void) {

	printf("Level2: Init\n");

}

void GameStateLevel2Update(double dt, long frame, int& state) {

	double fps = 1.0 / dt;
	printf("Level2: Update @> %f fps\n", fps);

}

void GameStateLevel2Draw(void) {

	printf("Level2: Draw\n");

	static float blue = 0.0f;
	blue += 0.01f;

	// Clear the screen
	glClearColor(0.0f, 0.0f, glm::abs(glm::sin(blue)), 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);





	glfwSwapBuffers(window);

}

void GameStateLevel2Free(void) {

	printf("Level2: Free\n");

}

void GameStateLevel2Unload(void) {

	printf("Level2: Unload\n");

}
