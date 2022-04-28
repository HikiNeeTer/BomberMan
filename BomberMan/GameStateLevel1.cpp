
#include "GameStateLevel1.h"
#include "CDT.h"
#include "GameObject.h"
#include "irrKlang.h"

#include <stdlib.h>
#include <string>

// -------------------------------------------
// Defines

#define MESH_MAX					32				// The total number of Mesh (Shape)
#define TEXTURE_MAX					32				// The total number of texture
#define GAME_OBJ_INST_MAX			1024			// The total number of different game object instances
#define FLAG_INACTIVE		0
#define FLAG_ACTIVE			1

#define SCREEN_HEIGHT 768
#define SCREEN_WIDTH 1024

#define MAP_HEIGHT 15
#define MAP_WIDTH 21

#define ENEMY_AMOUNT 3
#define ENEMY_SPEED 50.0f 
#define PLAYER_SIZE 45
#define OBJECT_SIZE 50
#define BOMBER_TIME 2.0f
#define EXPLODE_TIME 0.75f

#define ENTITY_FAKE_SIZE 55.0f


#define DEBUG 0

// -------------------------------------------

enum GAMEOBJ_TYPE
{
	// ENTITIES
	BLOCK = 0, GROUND, BRICK, 

	// PLAYER 
	F_PLAYER, S_PLAYER,

	// ENEMY
	V_ENEMY, H_ENEMY,

	// WEAPON
	BOMB, EXPLODE_BEGIN, EXPLODE_MID, EXPLODE_END
	// BUFF
	, SPEED_UP, BOMB_RADIUS_UP, BOMB_UP
	, SPEED_DOWN, BOMB_RADIUS_DOWN, BOMB_DOWN

	// UI
	, PLAYER_1_WIN, PLAYER_2_WIN, DRAW, RESTART
};

enum Key
{
	UP, LEFT, DOWN, RIGHT
};

static bool fPlayerKey[4] = { 0 };
static bool sPlayerKey[4] = { 0 };
static bool doOnce = true;

// -------------------------------------------
// Level variable, static - visible only in this file
// -------------------------------------------

static CDTMesh		sMeshArray[MESH_MAX];							// Store all unique shape/mesh in your game
static int			sNumMesh;
static CDTTex		sTexArray[TEXTURE_MAX];							// Corresponding texture of the mesh
static int			sNumTex;
static GameObject	sGameObjInstArray[GAME_OBJ_INST_MAX];			// Store all game object instance
static int			sNumGameObj;

static GameObject* fPlayer = nullptr;										// Pointer to the First Player game object instance
static GameObject* sPlayer = nullptr;
static double gameEndTime = 0.0f;

irrklang::ISoundEngine* engine;

/*
	B -> BLOCK (Undestroyable)
	X -> Will not generate Brick
	1 -> 1st Player
	2 -> 2nd Player
	O -> BRICK
	Q -> BOMB (RUN-TIME)
	E -> ENEMY (RUN-TIME)
*/


const std::string mapdata[] = { "BBBBBBBBBBBBBBBBBBBBB",
								"B1XX----------------B",
								"BXB-B-B-B-B-B-B-B-B-B",
								"BX------------------B",
								"B-B-B-B-B-B-B-B-B-B-B",
								"B-------------------B",
								"B-B-B-B-B-B-B-B-B-B-B",
								"B-------------------B",
								"B-B-B-B-B-B-B-B-B-B-B",
								"B-------------------B",
								"B-B-B-B-B-B-B-B-B-B-B",
								"B------------------XB",
								"B-B-B-B-B-B-B-B-B-BXB",
								"B----------------XX2B",
								"BBBBBBBBBBBBBBBBBBBBB" };

std::string map[] = { "BBBBBBBBBBBBBBBBBBBBB",
					  "B1XX----------------B",
					  "BXB-B-B-B-B-B-B-B-B-B",
					  "BX------------------B",
					  "B-B-B-B-B-B-B-B-B-B-B",
					  "B-------------------B",
					  "B-B-B-B-B-B-B-B-B-B-B",
					  "B-------------------B",
					  "B-B-B-B-B-B-B-B-B-B-B",
					  "B-------------------B",
					  "B-B-B-B-B-B-B-B-B-B-B",
					  "B------------------XB",
					  "B-B-B-B-B-B-B-B-B-BXB",
					  "B----------------XX2B",
					  "BBBBBBBBBBBBBBBBBBBBB" };

bool actionSpace = false;
bool actionEnter = false;
int currentEnemy = 0;

// functions to create/destroy a game object instance
static GameObject* gameObjInstCreate(int type, glm::vec3 pos, glm::vec3 vel, glm::vec3 scale, float orient);
static void	gameObjInstDestroy(GameObject& pInst);


// -------------------------------------------
// Game object instant functions
// -------------------------------------------

GameObject* gameObjInstCreate(int type, glm::vec3 pos, glm::vec3 vel, glm::vec3 scale, float orient)
{
	// loop through all object instance array to find the free slot
	for (int i = 0; i < GAME_OBJ_INST_MAX; i++) {
		GameObject* pInst = sGameObjInstArray + i;

		if (pInst->getFlag() == FLAG_INACTIVE) {

			/* GameObject */
			pInst->setMesh(sMeshArray + type);
			pInst->setTexture(sTexArray + type);
			pInst->setType(type);
			pInst->setFlag(FLAG_ACTIVE);
			pInst->setPosition(pos);
			pInst->setVelocity(vel);
			pInst->setScale(scale);
			pInst->setOrientation(orient);
			pInst->setModelMatrix(glm::mat4(1.0f));
			pInst->setCurrentFrame(0);
			pInst->setTexPosX(0.0f);
			pInst->setTexPosY(0.0f);
				
			/* Bomb */
			if (type == BOMB)
			{
				pInst->setTimer(BOMBER_TIME);
			}
			/* Player */
			else if (type == F_PLAYER || type == S_PLAYER)
			{
				// Reset Player's Bomb & Speed
				pInst->setAllBomb(1);
				pInst->setBombAmount(1);
				pInst->setBombRadius(1);
				pInst->SpeedDown();
			}

			sNumGameObj++;
			return pInst;
		}
	}

	// Cannot find empty slot => return 0
	return NULL;
}

void gameObjInstDestroy(GameObject& pInst)
{
	// Lazy deletion, not really delete the object, just set it as inactive
	if (pInst.getFlag() == FLAG_INACTIVE)
		return;

	if (pInst.getType() == F_PLAYER || pInst.getType() == S_PLAYER)
	{
		engine->play2D("Die.wav");
	}

	sNumGameObj--;
	pInst.setFlag(FLAG_INACTIVE);
}

void GetPlayerInput()
{
	if (fPlayer->getFlag() == FLAG_ACTIVE && fPlayer->getType() == F_PLAYER)
	{
		/*1st Player Controller*/
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			fPlayer->setVelocityY(fPlayer->getSpeedY());
			if (fPlayerKey[UP] == false)
			{
				fPlayer->setTexPosY(6.0f / 8.0f);
				fPlayerKey[UP] = true;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			fPlayer->setVelocityY(-fPlayer->getSpeedY());
			if (fPlayerKey[DOWN] == false)
			{
				fPlayer->setTexPosY(1.0f);
				fPlayerKey[DOWN] = true;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			fPlayer->setVelocityX(-fPlayer->getSpeedX());
			if (fPlayerKey[LEFT] == false)
			{
				fPlayer->setTexPosY(5.0f / 8.0f);
				fPlayerKey[LEFT] = true;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			fPlayer->setVelocityX(fPlayer->getSpeedX());
			if (fPlayerKey[RIGHT] == false)
			{
				fPlayer->setTexPosY(7.0f / 8.0f);
				fPlayerKey[RIGHT] = true;
			}
		}

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE)
		{
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE && fPlayerKey[UP] == true)
			{
				if (fPlayerKey[DOWN] == false && fPlayerKey[LEFT] == false && fPlayerKey[RIGHT] == false)
				{
					fPlayer->setTexPosX(0.0f);
					fPlayer->setTexPosY(6.0 / 8.0f);
				}
				fPlayerKey[UP] = false;
			}
			else if(glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE && fPlayerKey[DOWN] == true)
			{
				if (fPlayerKey[UP] == false && fPlayerKey[LEFT] == false && fPlayerKey[RIGHT] == false)
				{
					fPlayer->setTexPosX(0.0f);
					fPlayer->setTexPosY(1.0f);
				}
				fPlayerKey[DOWN] = false;
			}
			fPlayer->setVelocityY(0.0f);
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE)
		{
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE && fPlayerKey[LEFT] == true)
			{
				if (fPlayerKey[UP] == false && fPlayerKey[DOWN] == false && fPlayerKey[RIGHT] == false)
				{
					fPlayer->setTexPosX(0.0f);
					fPlayer->setTexPosY(5.0 / 8.0f);
				}
				fPlayerKey[LEFT] = false;
			}
			else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE && fPlayerKey[RIGHT] == true)
			{
				if (fPlayerKey[UP] == false && fPlayerKey[DOWN] == false && fPlayerKey[LEFT] == false)
				{
					fPlayer->setTexPosX(0.0f);
					fPlayer->setTexPosY(7.0 / 8.0f);
				}
				fPlayerKey[RIGHT] = false;
			}
			fPlayer->setVelocityX(0.0f);
		}

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !actionSpace && fPlayer->getBombAmount() > 0)
		{
			if (fPlayer->getFlag() == FLAG_ACTIVE)
			{
				glm::vec3 actualPos = CalculateBombPosition(fPlayer->getPosition().x, fPlayer->getPosition().y);
				if (map[(int)actualPos.y / 50][(int)actualPos.x / 50] != 'Q')
				{
					// Create Bomb
					GameObject* obj = gameObjInstCreate(BOMB, actualPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(50.0f, 50.0f, 1.0f), 0.0f);
					engine->play2D("BombPlace.wav");
					// Decrease BombAmount of 1st Player at that time
					fPlayer->setBombAmount(fPlayer->getBombAmount() - 1);
					// Setting-Up current Bomb
					obj->setBomber(F_PLAYER);
					obj->setBombRadius(fPlayer->getBombRadius());
					// Change map on that position to 'Q' (Bomb Placed)
					map[(int)obj->getPosition().y / OBJECT_SIZE][(int)obj->getPosition().x / OBJECT_SIZE] = 'Q';
					// Detect Long Press Space
					actionSpace = true;
				}
			}
		}
		else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) { actionSpace = false; }
	}

	/*-------------------------------------------------------------------------------------------*/

	if (sPlayer->getFlag() == FLAG_ACTIVE && sPlayer->getType() == S_PLAYER)
	{
		/*2nd Player Controller*/
		if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
		{
			sPlayer->setVelocityY(sPlayer->getSpeedY());
			if (sPlayerKey[UP] == false)
			{
				sPlayer->setTexPosY(6.0f / 8.0f);
				sPlayerKey[UP] = true;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
		{
			sPlayer->setVelocityY(-sPlayer->getSpeedY());
			if (sPlayerKey[DOWN] == false)
			{
				sPlayer->setTexPosY(1.0f);
				sPlayerKey[DOWN] = true;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		{
			sPlayer->setVelocityX(-sPlayer->getSpeedX());
			if (sPlayerKey[LEFT] == false)
			{
				sPlayer->setTexPosY(5.0f / 8.0f);
				sPlayerKey[LEFT] = true;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		{
			sPlayer->setVelocityX(sPlayer->getSpeedX());
			if (sPlayerKey[RIGHT] == false)
			{
				sPlayer->setTexPosY(7.0f / 8.0f);
				sPlayerKey[RIGHT] = true;
			}
		}

		if (glfwGetKey(window, GLFW_KEY_I) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_K) == GLFW_RELEASE)
		{
			if (glfwGetKey(window, GLFW_KEY_I) == GLFW_RELEASE && sPlayerKey[UP] == true)
			{
				if (sPlayerKey[DOWN] == false && sPlayerKey[LEFT] == false && sPlayerKey[RIGHT] == false)
				{
					sPlayer->setTexPosX(0.0f);
					sPlayer->setTexPosY(6.0 / 8.0f);
				}
				sPlayerKey[UP] = false;
			}
			else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_RELEASE && sPlayerKey[DOWN] == true)
			{
				if (sPlayerKey[UP] == false && sPlayerKey[LEFT] == false && sPlayerKey[RIGHT] == false)
				{
					sPlayer->setTexPosX(0.0f);
					sPlayer->setTexPosY(1.0f);
				}
				sPlayerKey[DOWN] = false;
			}
			sPlayer->setVelocityY(0.0f);
		}

		if (glfwGetKey(window, GLFW_KEY_J) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE)
		{
			if (glfwGetKey(window, GLFW_KEY_J) == GLFW_RELEASE && sPlayerKey[LEFT] == true)
			{
				if (sPlayerKey[UP] == false && sPlayerKey[DOWN] == false && sPlayerKey[RIGHT] == false)
				{
					sPlayer->setTexPosX(0.0f);
					sPlayer->setTexPosY(5.0 / 8.0f);
				}
				sPlayerKey[LEFT] = false;
			}
			else if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE && sPlayerKey[RIGHT] == true)
			{
				if (sPlayerKey[UP] == false && sPlayerKey[DOWN] == false && sPlayerKey[LEFT] == false)
				{
					sPlayer->setTexPosX(0.0f);
					sPlayer->setTexPosY(7.0 / 8.0f);
				}
				sPlayerKey[RIGHT] = false;
			}
			sPlayer->setVelocityX(0.0f);
		}

		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !actionEnter && sPlayer->getBombAmount() > 0)
		{
			if (sPlayer->getFlag() == FLAG_ACTIVE)
			{
				glm::vec3 actualPos = CalculateBombPosition(sPlayer->getPosition().x, sPlayer->getPosition().y);
				if (map[(int)actualPos.y / 50][(int)actualPos.x / 50] != 'Q')
				{
					// Create Bomb
					GameObject* obj = gameObjInstCreate(BOMB, actualPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(50.0f, 50.0f, 1.0f), 0.0f);
					engine->play2D("BombPlace.wav");
					// Decrease BombAmount of 1st Player at that time
					sPlayer->setBombAmount(sPlayer->getBombAmount() - 1);
					// Setting-Up current Bomb
					obj->setBomber(S_PLAYER);
					obj->setBombRadius(sPlayer->getBombRadius());
					// Change map on that position to 'Q' (Bomb Placed)
					map[(int)obj->getPosition().y / OBJECT_SIZE][(int)obj->getPosition().x / OBJECT_SIZE] = 'Q';
					// Detect Long Press Enter
					actionEnter = true;
				}
			}

		}
		else if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) { actionEnter = false; }
	}

}


// -------------------------------------------
// Game states function
// -------------------------------------------

void GameStateLevel1Load(void) {

	// clear the Mesh array
	memset(sMeshArray, 0, sizeof(CDTMesh) * MESH_MAX);
	sNumMesh = 0;

	//+ clear the Texture array
	memset(sTexArray, 0, sizeof(CDTTex) * TEXTURE_MAX);
	sNumTex = 0;

	//+ clear the game object instance array
	for (int idx = 0; idx < GAME_OBJ_INST_MAX; idx++)
	{
		gameObjInstDestroy(sGameObjInstArray[idx]);
	}


	// --------------------------------------------------------------------------
	// Create all of the unique meshes/textures and put them in MeshArray/TexArray
	//		- The order of mesh should follow enum GAMEOBJ_TYPE 
	/// --------------------------------------------------------------------------

	// Temporary variable for creating mesh
	CDTMesh* pMesh;
	CDTTex* pTex;
	std::vector<CDTVertex> vertices;
	CDTVertex v1, v2, v3, v4;

	// Create GameObject mesh/texture
	vertices.clear();
	v1.x = -0.5f; v1.y = -0.5f; v1.z = 0.0f; v1.r = 1.0f; v1.g = 0.0f; v1.b = 0.0f; v1.u = 0.0f; v1.v = 0.0f;
	v2.x = 0.5f; v2.y = -0.5f; v2.z = 0.0f; v2.r = 0.0f; v2.g = 1.0f; v2.b = 0.0f; v2.u = 1.0f; v2.v = 0.0f;
	v3.x = 0.5f; v3.y = 0.5f; v3.z = 0.0f; v3.r = 0.0f; v3.g = 0.0f; v3.b = 1.0f; v3.u = 1.0f; v3.v = 1.0f;
	v4.x = -0.5f; v4.y = 0.5f; v4.z = 0.0f; v4.r = 1.0f; v4.g = 1.0f; v4.b = 0.0f; v4.u = 0.0f; v4.v = 1.0f;
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v1);
	vertices.push_back(v3);
	vertices.push_back(v4);

	// Create Block(Undestroyable) Mesh & Texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("Block.png");

	// Create Ground Mesh & Texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("Ground.png");

	// Create Brick(Destroyable) Mesh & Texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("brick.png");


	// Create GameObject mesh/texture
	vertices.clear();
	v1.x = -0.5f; v1.y = -0.5f; v1.z = 0.0f; v1.r = 1.0f; v1.g = 0.0f; v1.b = 0.0f; v1.u = 0.0f; v1.v = 7.0f / 8.0f;
	v2.x = 0.5f; v2.y = -0.5f; v2.z = 0.0f; v2.r = 0.0f; v2.g = 1.0f; v2.b = 0.0f; v2.u = 1.0f / 11.0f; v2.v = 7.0f / 8.0f;
	v3.x = 0.5f; v3.y = 0.5f; v3.z = 0.0f; v3.r = 0.0f; v3.g = 0.0f; v3.b = 1.0f; v3.u = 1.0f / 11.0f; v3.v = 1.0f;
	v4.x = -0.5f; v4.y = 0.5f; v4.z = 0.0f; v4.r = 1.0f; v4.g = 1.0f; v4.b = 0.0f; v4.u = 0.0f; v4.v = 1.0f;
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v1);
	vertices.push_back(v3);
	vertices.push_back(v4);

	// Create 1st Player Mesh & Texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("BomberMan_Blue.png");

	// Create 2nd Player Mesh & Texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("BomberMan_Red.png");

	// Create Vertical Enemy mesh/texture
	vertices.clear();
	v1.x = -0.5f; v1.y = -0.5f; v1.z = 0.0f; v1.r = 1.0f; v1.g = 0.0f; v1.b = 0.0f; v1.u = 0.0f; v1.v = 5.0f / 8.0f;
	v2.x = 0.5f; v2.y = -0.5f; v2.z = 0.0f; v2.r = 0.0f; v2.g = 1.0f; v2.b = 0.0f; v2.u = 1.0f / 11.0f; v2.v = 5.0f / 8.0f;
	v3.x = 0.5f; v3.y = 0.5f; v3.z = 0.0f; v3.r = 0.0f; v3.g = 0.0f; v3.b = 1.0f; v3.u = 1.0f / 11.0f; v3.v = 6.0f / 8.0f;
	v4.x = -0.5f; v4.y = 0.5f; v4.z = 0.0f; v4.r = 1.0f; v4.g = 1.0f; v4.b = 0.0f; v4.u = 0.0f; v4.v = 6.0f / 8.0f;
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v1);
	vertices.push_back(v3);
	vertices.push_back(v4);


	// Create Vertical Enemy Mesh & Texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("Enemy.png");

	// Create Horizontal Enemy mesh/texture
	vertices.clear();
	v1.x = -0.5f; v1.y = -0.5f; v1.z = 0.0f; v1.r = 1.0f; v1.g = 0.0f; v1.b = 0.0f; v1.u = 0.0f; v1.v = 6.0f / 8.0f;
	v2.x = 0.5f; v2.y = -0.5f; v2.z = 0.0f; v2.r = 0.0f; v2.g = 1.0f; v2.b = 0.0f; v2.u = 1.0f / 11.0f; v2.v = 6.0f / 8.0f;
	v3.x = 0.5f; v3.y = 0.5f; v3.z = 0.0f; v3.r = 0.0f; v3.g = 0.0f; v3.b = 1.0f; v3.u = 1.0f / 11.0f; v3.v = 7.0f / 8.0f;
	v4.x = -0.5f; v4.y = 0.5f; v4.z = 0.0f; v4.r = 1.0f; v4.g = 1.0f; v4.b = 0.0f; v4.u = 0.0f; v4.v = 7.0f / 8.0f;
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v1);
	vertices.push_back(v3);
	vertices.push_back(v4);

	// Create Horizontal Enemy Mesh & Texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("Enemy.png");

	// Create Bomb mesh/texture
	vertices.clear();
	v1.x = -0.5f; v1.y = -0.5f; v1.z = 0.0f; v1.r = 1.0f; v1.g = 0.0f; v1.b = 0.0f; v1.u = 0.0f; v1.v = 0.0f;
	v2.x = 0.5f; v2.y = -0.5f; v2.z = 0.0f; v2.r = 0.0f; v2.g = 1.0f; v2.b = 0.0f; v2.u = 1.0f / 5.0f; v2.v = 0.0f;
	v3.x = 0.5f; v3.y = 0.5f; v3.z = 0.0f; v3.r = 0.0f; v3.g = 0.0f; v3.b = 1.0f; v3.u = 1.0f / 5.0f; v3.v = 1.0f;
	v4.x = -0.5f; v4.y = 0.5f; v4.z = 0.0f; v4.r = 1.0f; v4.g = 1.0f; v4.b = 0.0f; v4.u = 0.0f; v4.v = 1.0f;
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v1);
	vertices.push_back(v3);
	vertices.push_back(v4);

	// Create Bomb Mesh & Texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("Bomb.png");

	// Create GameObject mesh/texture
	vertices.clear();
	v1.x = -0.5f; v1.y = -0.5f; v1.z = 0.0f; v1.r = 1.0f; v1.g = 0.0f; v1.b = 0.0f; v1.u = 0.0f; v1.v = 0.0f;
	v2.x = 0.5f; v2.y = -0.5f; v2.z = 0.0f; v2.r = 0.0f; v2.g = 1.0f; v2.b = 0.0f; v2.u = 1.0f; v2.v = 0.0f;
	v3.x = 0.5f; v3.y = 0.5f; v3.z = 0.0f; v3.r = 0.0f; v3.g = 0.0f; v3.b = 1.0f; v3.u = 1.0f; v3.v = 1.0f;
	v4.x = -0.5f; v4.y = 0.5f; v4.z = 0.0f; v4.r = 1.0f; v4.g = 1.0f; v4.b = 0.0f; v4.u = 0.0f; v4.v = 1.0f;
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v1);
	vertices.push_back(v3);
	vertices.push_back(v4);

	// Create Explode Mesh & Texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("ExplodeBegin.png");
	
	// Create Explode Mesh & Texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("ExplodeMid.png");

	// Create Explode Mesh & Texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("ExplodeEnd.png");

	// Create Buff Speed Mesh & Texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("SpeedUp.png");

	// Create Buff Bomb Radius Mesh & Texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("RadiusUp.png");

	// Create Buff Bomb Amount Mesh & Texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("BombUp.png");

	// Create Neft Speed Mesh & Texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("SpeedDown.png");

	// Create Buff Bomb Radius Mesh & Texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("RadiusDown.png");

	// Create Buff Bomb Amount Mesh & Texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("BombDown.png");

	// Create PlayerUI Mesh & Texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("Player1Win.png");

	// Create PlayerUI Mesh & Texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("Player2Win.png");

	// Create PlayerUI Mesh & Texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("Draw.png");

	// Create PlayerUI Mesh & Texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("Restart.png");

	printf("Level1: Load\n");
}


void GameStateLevel1Init(void) {

	engine = irrklang::createIrrKlangDevice();

	if (!engine)
		return;

	// Set BGM Sound
	irrklang::ISoundSource* BGM = engine->addSoundSourceFromFile("Bomberman Theme.mp3");
	BGM->setDefaultVolume(0.15f);
	engine->play2D(BGM, true);

	// Move Camera to get Left Corner be (0,0)
	MoveCam(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 35.0f);

	for (int y = 0; y < MAP_HEIGHT; y++)
	{
		for (int x = 0; x < MAP_WIDTH; x++)
		{
			// 50.0f is a default size of all entity
			float xPos = x * OBJECT_SIZE;
			float yPos = y * OBJECT_SIZE;

			gameObjInstCreate(GROUND, glm::vec3(xPos, yPos, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(OBJECT_SIZE, OBJECT_SIZE, 1.0f), 0.0f);

			if (map[y][x] == 'B') // BLOCK
			{
				gameObjInstCreate(BLOCK, glm::vec3(xPos, yPos, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(OBJECT_SIZE, OBJECT_SIZE, 1.0f), 0.0f);
			}
			else if (map[y][x] == '1') // 1st Player
			{
				fPlayer = gameObjInstCreate(F_PLAYER, glm::vec3(xPos, yPos, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(ENTITY_FAKE_SIZE, ENTITY_FAKE_SIZE, 1.0f), 0.0f);
			}
			else if (map[y][x] == '2') // 2nd Player
			{
				sPlayer = gameObjInstCreate(S_PLAYER, glm::vec3(xPos, yPos, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(ENTITY_FAKE_SIZE, ENTITY_FAKE_SIZE, 1.0f), 0.0f);
			}
			else if (map[y][x] == '-') // Empty
			{
				int rand_num = rand() % 20;
				if (currentEnemy < ENEMY_AMOUNT && CheckEnemySpawn(x,y) && rand_num == rand() % 20) // Generate Enemy(Possibility 1/20)
				{
					GameObject* obj = gameObjInstCreate(CheckEnemyType(x,y), glm::vec3(xPos, yPos, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(PLAYER_SIZE, PLAYER_SIZE, 1.0f), 0.0f);
					map[y][x] = 'E';
					if (obj->getType() == V_ENEMY)
					{
						obj->setVelocityY(ENEMY_SPEED);
					}
					else
					{
						obj->setVelocityX(ENEMY_SPEED);
					}
					currentEnemy++;
				}
				else if (rand_num % 5 != 0) // Create Brick (Possibility 15/20)
				{
					if (!DEBUG)
					{
						gameObjInstCreate(BRICK, glm::vec3(xPos, yPos, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(OBJECT_SIZE, OBJECT_SIZE, 1.0f), 0.0f);
						// Set that grid to O (BRICK)
						map[y][x] = 'O';
					}
				}
			}
		}
	}

	for (int i = 0; i < GAME_OBJ_INST_MAX; i++)
	{
		GameObject* pInst1 = sGameObjInstArray + i;
		if (pInst1->getType() == BRICK)
		{
			int x = pInst1->getPosition().x / OBJECT_SIZE;
			int y = pInst1->getPosition().y / OBJECT_SIZE;
			if (map[y-1][x] == 'E' || map[y+1][x] == 'E' || map[y][x+1] == 'E' || map[y][x-1] == 'E')
			{
				gameObjInstDestroy(*pInst1);
				map[y][x] = mapdata[y][x];
			}
		}
	}
}


void GameStateLevel1Update(double dt, long frame, int& state) {

	//printf("%.2f\n", gameEndTime);
	if (fPlayer->getType() != F_PLAYER || fPlayer->getFlag() == FLAG_INACTIVE || sPlayer->getType() != S_PLAYER || sPlayer->getFlag() == FLAG_INACTIVE)
	{
		if (gameEndTime > 1.5f)
		{
			if (doOnce)
			{
				if ((fPlayer->getType() != F_PLAYER || fPlayer->getFlag() == FLAG_INACTIVE) && (sPlayer->getType() != S_PLAYER || sPlayer->getFlag() == FLAG_INACTIVE))
				{
					gameObjInstCreate(DRAW, glm::vec3(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 3.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(SCREEN_WIDTH, SCREEN_HEIGHT, 1.0f), 0.0f);
					printf("Draw\n");
				}
				else if (fPlayer->getType() != F_PLAYER || fPlayer->getFlag() == FLAG_INACTIVE)
				{
					gameObjInstCreate(PLAYER_2_WIN, glm::vec3(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 3.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(SCREEN_WIDTH, SCREEN_HEIGHT, 1.0f), 0.0f);
					printf("Player 2 Win\n");
				}
				else
				{
					gameObjInstCreate(PLAYER_1_WIN, glm::vec3(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 3.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(SCREEN_WIDTH, SCREEN_HEIGHT, 1.0f), 0.0f);
					printf("Player 1 Win\n");
				}
				gameObjInstCreate(RESTART, glm::vec3(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 35.0f, 3.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(SCREEN_WIDTH, SCREEN_HEIGHT, 1.0f), 0.0f);
				engine->removeAllSoundSources();
				engine->play2D("WinSound.wav");
				doOnce = false;
			}
		}
		else
		{
			gameEndTime += dt;
		}
	}

	GetPlayerInput();

	//-----------------------------------------
	// Check for collsion, O(n^2)
	//-----------------------------------------

	for (int i = 0; i < GAME_OBJ_INST_MAX; i++) {
		GameObject* pInst1 = sGameObjInstArray + i;

		// skip inactive object
		if (pInst1->getFlag() == FLAG_INACTIVE)
			continue;

		// if pInst1 is an First Player || Second Player
		if (pInst1->getType() == F_PLAYER || pInst1->getType() == S_PLAYER) {

			// compare pInst1 with all game obj instances 
			for (int j = 0; j < GAME_OBJ_INST_MAX; j++)
			{
				GameObject* pInst2 = sGameObjInstArray + j;

				// skip inactive object
				if (pInst2->getFlag() == FLAG_INACTIVE)
					continue;


				if (pInst2->getType() == BLOCK || pInst2->getType() == BRICK || pInst2->getType() == BOMB)
				{
					float leftA = pInst1->getLeftBound();
					float rightA = pInst1->getRightBound();
					float topA = pInst1->getTopBound();
					float bottomA = pInst1->getBottomBound();

					// If Player overlap with BOMB -> Player just place a bomb -> mustn't detect collision
					if (pInst2->getType() == BOMB && CheckCollision(pInst1->getPosition(), pInst1->getScale(), pInst2->getPosition(), glm::vec3(OBJECT_SIZE / 2, OBJECT_SIZE / 2, 1.0f)))
					{
						continue;
					}

					// Collision Left Side
					if (CheckCollision(glm::vec3(leftA, pInst1->getPosition().y, pInst1->getPosition().z), glm::vec3(0.1f, PLAYER_SIZE - 7.5f, 0.0f),
						pInst2->getPosition(), pInst2->getScale()))
					{
						if (pInst1->getVelocity().x < 0.0f)
						{
							printf("Left Collide\n");
							pInst1->setVelocityX(0.0f);
						}
					}
					// Collision Right Side
					if (CheckCollision(glm::vec3(rightA, pInst1->getPosition().y, pInst1->getPosition().z), glm::vec3(0.1f, PLAYER_SIZE - 7.5f, 0.0f),
						pInst2->getPosition(), pInst2->getScale()))
					{
						if (pInst1->getVelocity().x > 0.0f)
						{
							printf("Right Collide\n");
							pInst1->setVelocityX(0.0f);
						}
					}
					// Collision Top Side
					if (CheckCollision(glm::vec3(pInst1->getPosition().x, topA, pInst1->getPosition().z), glm::vec3(PLAYER_SIZE - 7.5f, 0.1f, 0.0f),
						pInst2->getPosition(), pInst2->getScale()))
					{
						if (pInst1->getVelocity().y > 0.0f)
						{
							printf("Top Collide\n");
							pInst1->setVelocityY(0.0f);
						}
					}
					// Collision Bottom Side
					if (CheckCollision(glm::vec3(pInst1->getPosition().x, bottomA, pInst1->getPosition().z), glm::vec3(PLAYER_SIZE - 7.5f, 0.1f, 0.0f),
						pInst2->getPosition(), pInst2->getScale()))
					{
						if (pInst1->getVelocity().y < 0.0f)
						{
							printf("Bottom Collide\n");
							pInst1->setVelocityY(0.0f);
						}
					}

				}
				else if (pInst2->getType() == F_PLAYER || pInst2->getType() == S_PLAYER)
				{
					if (CheckCollision(pInst1->getPosition(), pInst1->getScale(), pInst2->getPosition(), pInst2->getScale()))
					{
						// Collide at Left Side
						if (pInst1->getPosition().x > pInst2->getPosition().x)
						{
							printf("Player Left Collide\n");
							if (pInst1->getVelocity().x < 0.0f) { pInst1->setVelocityX(0.0f); }
							if (pInst2->getVelocity().x > 0.0f) { pInst2->setVelocityX(0.0f); }
						}
						// Collide at Right Side
						if (pInst1->getPosition().x < pInst2->getPosition().x)
						{
							printf("Player Right Collide\n");
							if (pInst1->getVelocity().x > 0.0f) { pInst1->setVelocityX(0.0f); }
							if (pInst2->getVelocity().x < 0.0f) { pInst2->setVelocityX(0.0f); }
						}
						// Collide at Bottom Side
						if (pInst1->getPosition().y > pInst2->getPosition().y)
						{
							printf("Player Bottom Collide\n");
							if (pInst1->getVelocity().y < 0.0f) { pInst1->setVelocityY(0.0f); }
							if (pInst2->getVelocity().y > 0.0f) { pInst2->setVelocityY(0.0f); }
						}
						// Collide at Top Side
						if (pInst1->getPosition().y < pInst2->getPosition().y)
						{
							printf("Player Top Collide\n");
							if (pInst1->getVelocity().y > 0.0f) { pInst1->setVelocityY(0.0f); }
							if (pInst2->getVelocity().y < 0.0f) { pInst2->setVelocityY(0.0f); }
						}
					}
				}
				else if (pInst2->getType() == SPEED_UP)
				{
					if (CheckCollision(pInst1->getPosition(), pInst1->getScale(), pInst2->getPosition(), glm::vec3(pInst2->getScale().x / 2, pInst2->getScale().y / 2, 1.0f)))
					{
						engine->play2D("Power-Up.wav");
						pInst1->SpeedUp();
						gameObjInstDestroy(*pInst2);
					}
				}
				else if (pInst2->getType() == BOMB_RADIUS_UP)
				{
					if (CheckCollision(pInst1->getPosition(), pInst1->getScale(), pInst2->getPosition(), glm::vec3(pInst2->getScale().x / 2, pInst2->getScale().y / 2, 1.0f)))
					{
						engine->play2D("Power-Up.wav");
						pInst1->BombRadiusIncrease();
						gameObjInstDestroy(*pInst2);
					}
				}
				else if (pInst2->getType() == BOMB_UP)
				{
					if (CheckCollision(pInst1->getPosition(), pInst1->getScale(), pInst2->getPosition(), glm::vec3(pInst2->getScale().x / 2, pInst2->getScale().y / 2, 1.0f)))
					{
						engine->play2D("Power-Up.wav");
						pInst1->BombIncerase();
						gameObjInstDestroy(*pInst2);
					}
				}
				else if (pInst2->getType() == SPEED_DOWN)
				{
					if (CheckCollision(pInst1->getPosition(), pInst1->getScale(), pInst2->getPosition(), glm::vec3(pInst2->getScale().x / 2, pInst2->getScale().y / 2, 1.0f)))
					{
						engine->play2D("Power-Down.wav");
						pInst1->SpeedDown();
						gameObjInstDestroy(*pInst2);
					}
				}
				else if (pInst2->getType() == BOMB_RADIUS_DOWN)
				{
					if (CheckCollision(pInst1->getPosition(), pInst1->getScale(), pInst2->getPosition(), glm::vec3(pInst2->getScale().x / 2, pInst2->getScale().y / 2, 1.0f)))
					{
						engine->play2D("Power-Down.wav");
						pInst1->BombRadiusDecrease();
						gameObjInstDestroy(*pInst2);
					}
				}
				else if (pInst2->getType() == BOMB_DOWN)
				{
					if (CheckCollision(pInst1->getPosition(), pInst1->getScale(), pInst2->getPosition(), glm::vec3(pInst2->getScale().x / 2, pInst2->getScale().y / 2, 1.0f)))
					{
						engine->play2D("Power-Down.wav");
						pInst1->BombDecrease();
						gameObjInstDestroy(*pInst2);
					}
				}
				else if (pInst2->getType() == EXPLODE_BEGIN || pInst2->getType() == EXPLODE_MID || pInst2->getType() == EXPLODE_END )
				{
					// If Player collide with exploded bomb -> DIE!!
					if (CheckCollision(pInst1->getPosition(), glm::vec3(pInst1->getScale().x - 10.0f, pInst1->getScale().y - 10.0f, 1.0f), pInst2->getPosition(), pInst2->getScale()))
					{
						gameObjInstDestroy(*pInst1);
					}
				}
			}
		}

		else if (pInst1->getType() == EXPLODE_BEGIN || pInst1->getType() == EXPLODE_MID || pInst1->getType() == EXPLODE_END)
		{
			// compare pInst1 with all game obj instances 
			for (int j = 0; j < GAME_OBJ_INST_MAX; j++)
			{
				GameObject* pInst2 = sGameObjInstArray + j;

				// skip inactive object
				if (pInst2->getFlag() == FLAG_INACTIVE)
					continue;

				if (pInst2->getType() == BRICK || pInst2->getType() == V_ENEMY || pInst2->getType() == H_ENEMY)
				{
					// If EXPLODE collide with BRICK -> Destroy that BRICK
					if (CheckCollision(pInst1->getPosition(), glm::vec3(pInst1->getScale().x - 0.1f, pInst1->getScale().y - 0.1f, 1.0f), pInst2->getPosition(), pInst2->getScale()))
					{
						map[(int)pInst2->getPosition().y / OBJECT_SIZE][(int)pInst2->getPosition().x / OBJECT_SIZE] = '-';
						int rand_buff = (pInst2->getType() == BRICK ? (rand() % 20) + SPEED_UP : (rand() % 3) + SPEED_UP);
						// Clamp Position of Item Drop
						glm::vec3 actualPos = CalculateBombPosition(pInst2->getPosition().x, pInst2->getPosition().y);
						actualPos.z = 2.0f;
						if (rand_buff == SPEED_UP)
						{
							GameObject* obj = gameObjInstCreate(SPEED_UP, actualPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(OBJECT_SIZE, OBJECT_SIZE, 0.0f), 0.0f);
							obj->setTimeBeforeActive(EXPLODE_TIME + 0.1f);
						}
						else if (rand_buff == BOMB_RADIUS_UP)
						{
							GameObject* obj = gameObjInstCreate(BOMB_RADIUS_UP, actualPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(OBJECT_SIZE, OBJECT_SIZE, 0.0f), 0.0f);
							obj->setTimeBeforeActive(EXPLODE_TIME + 0.1f);
						}
						else if (rand_buff == BOMB_UP)
						{
							GameObject* obj = gameObjInstCreate(BOMB_UP, actualPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(OBJECT_SIZE, OBJECT_SIZE, 0.0f), 0.0f);
							obj->setTimeBeforeActive(EXPLODE_TIME + 0.1f);
						}
						if (rand_buff == SPEED_DOWN)
						{
							GameObject* obj = gameObjInstCreate(SPEED_DOWN, actualPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(OBJECT_SIZE, OBJECT_SIZE, 0.0f), 0.0f);
							obj->setTimeBeforeActive(EXPLODE_TIME + 0.1f);
						}
						else if (rand_buff == BOMB_RADIUS_DOWN)
						{
							GameObject* obj = gameObjInstCreate(BOMB_RADIUS_DOWN, actualPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(OBJECT_SIZE, OBJECT_SIZE, 0.0f), 0.0f);
							obj->setTimeBeforeActive(EXPLODE_TIME + 0.1f);
						}
						else if (rand_buff == BOMB_DOWN)
						{
							GameObject* obj = gameObjInstCreate(BOMB_DOWN, actualPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(OBJECT_SIZE, OBJECT_SIZE, 0.0f), 0.0f);
							obj->setTimeBeforeActive(EXPLODE_TIME + 0.1f);
						}
						gameObjInstDestroy(*pInst2);
					}
				}
				else if (pInst2->getType() >= SPEED_UP && pInst2->getTimeBeforeActive() <= 0.0f)
				{
					if (CheckCollision(pInst1->getPosition(), pInst1->getScale(), pInst2->getPosition(), glm::vec3(pInst2->getScale().x / 2, pInst2->getScale().y / 2, 1.0f)))
					{
						gameObjInstDestroy(*pInst2);
					}
				}
			}
		}

		else if (pInst1->getType() == V_ENEMY || pInst1->getType() == H_ENEMY)
		{
			// compare pInst1 with all game obj instances 
			for (int j = 0; j < GAME_OBJ_INST_MAX; j++)
			{
				GameObject* pInst2 = sGameObjInstArray + j;
				if (pInst2->getFlag() == FLAG_INACTIVE)
					continue;

				if (pInst2->getType() == BOMB || pInst2->getType() == BLOCK || pInst2->getType() == BRICK)
				{
					float leftA = pInst1->getLeftBound();
					float rightA = pInst1->getRightBound();
					float topA = pInst1->getTopBound();
					float bottomA = pInst1->getBottomBound();

					// Collision Left Side
					if (CheckCollision(glm::vec3(leftA, pInst1->getPosition().y, pInst1->getPosition().z), glm::vec3(1.0f, PLAYER_SIZE - 5.0f, 0.0f),
						pInst2->getPosition(), pInst2->getScale()))
					{
						if (pInst1->getVelocity().x < 0.0f)
						{
							pInst1->setTexPosX(0.0f);
							pInst1->setTexPosY(8.0f / 8.0f);
							pInst1->setVelocityX(ENEMY_SPEED);
						}
					}
					// Collision Right Side
					if (CheckCollision(glm::vec3(rightA, pInst1->getPosition().y, pInst1->getPosition().z), glm::vec3(1.0f, PLAYER_SIZE - 5.0f, 0.0f),
						pInst2->getPosition(), pInst2->getScale()))
					{
						if (pInst1->getVelocity().x > 0.0f)
						{
							pInst1->setTexPosX(0.0f);
							pInst1->setTexPosY(6.0f / 8.0f);
							pInst1->setVelocityX(-ENEMY_SPEED);
						}
					}
					// Collision Top Side
					if (CheckCollision(glm::vec3(pInst1->getPosition().x, topA, pInst1->getPosition().z), glm::vec3(PLAYER_SIZE - 5.0f, 1.0f, 0.0f),
						pInst2->getPosition(), pInst2->getScale()))
					{
						if (pInst1->getVelocity().y > 0.0f)
						{
							pInst1->setTexPosX(0.0f);
							pInst1->setTexPosY(10.0 / 8.0f);
							pInst1->setVelocityY(-ENEMY_SPEED);
						}
					}
					// Collision Bottom Side
					if (CheckCollision(glm::vec3(pInst1->getPosition().x, bottomA, pInst1->getPosition().z), glm::vec3(PLAYER_SIZE - 5.0f, 1.0f, 0.0f),
						pInst2->getPosition(), pInst2->getScale()))
					{
						if (pInst1->getVelocity().y < 0.0f)
						{
							pInst1->setTexPosX(0.0f);
							pInst1->setTexPosY(0.0f / 8.0f);
							pInst1->setVelocityY(ENEMY_SPEED);
						}
					}
				}
				else if (pInst2->getType() == F_PLAYER || pInst2->getType() == S_PLAYER)
				{
					float leftA = pInst1->getLeftBound();
					float rightA = pInst1->getRightBound();
					float topA = pInst1->getTopBound();
					float bottomA = pInst1->getBottomBound();

					// Collision Left Side
					if (CheckCollision(glm::vec3(leftA, pInst1->getPosition().y, pInst1->getPosition().z), glm::vec3(1.0f, PLAYER_SIZE - 5.0f, 0.0f),
						pInst2->getPosition(), pInst2->getScale()))
					{
						gameObjInstDestroy(*pInst2);
					}
					// Collision Right Side
					if (CheckCollision(glm::vec3(rightA, pInst1->getPosition().y, pInst1->getPosition().z), glm::vec3(1.0f, PLAYER_SIZE - 5.0f, 0.0f),
						pInst2->getPosition(), pInst2->getScale()))
					{
						gameObjInstDestroy(*pInst2);
					}
					// Collision Top Side
					if (CheckCollision(glm::vec3(pInst1->getPosition().x, topA, pInst1->getPosition().z), glm::vec3(PLAYER_SIZE - 5.0f, 1.0f, 0.0f),
						pInst2->getPosition(), pInst2->getScale()))
					{
						gameObjInstDestroy(*pInst2);
					}
					// Collision Bottom Side
					if (CheckCollision(glm::vec3(pInst1->getPosition().x, bottomA, pInst1->getPosition().z), glm::vec3(PLAYER_SIZE - 5.0f, 1.0f, 0.0f),
						pInst2->getPosition(), pInst2->getScale()))
					{
						gameObjInstDestroy(*pInst2);
					}
				}
			}
		}
	}

	//---------------------------------------------------------
	// Update all game obj position using velocity 
	//---------------------------------------------------------

	for (int i = 0; i < GAME_OBJ_INST_MAX; i++) {
		GameObject* pInst = sGameObjInstArray + i;

		// skip inactive object
		if (pInst->getFlag() == FLAG_INACTIVE)
			continue;

		pInst->setCurrentFrame(pInst->getCurrentFrame() + 1);

		if (pInst->getType() == F_PLAYER)
		{
			if (pInst->getCurrentFrame() % 5 == 0)
			{
				if (fPlayerKey[UP] == false && fPlayerKey[LEFT] == false && fPlayerKey[DOWN] == false && fPlayerKey[RIGHT] == false)
				{
					continue;
				}
				pInst->setTexPosX(pInst->getTexPosX() + 1.0f / 11.0f);
				if (pInst->getTexPosX() > 2.1f / 11.0f)
				{
					pInst->setTexPosX(1.0 / 11.0f);
				}
			}
			// Update Character Position
			pInst->setPosition(pInst->getPosition() + (pInst->getVelocity() * glm::vec3(dt, dt, 0.0f)));
		}
		else if (pInst->getType() == S_PLAYER)
		{
			if (pInst->getCurrentFrame() % 5 == 0)
			{
				if (sPlayerKey[UP] == false && sPlayerKey[LEFT] == false && sPlayerKey[DOWN] == false && sPlayerKey[RIGHT] == false)
				{
					continue;
				}
				pInst->setTexPosX(pInst->getTexPosX() + 1.0f / 11.0f);
				if (pInst->getTexPosX() > 2.1f / 11.0f)
				{
					pInst->setTexPosX(1.0 / 11.0f);
				}
			}
			// Update Character Position
			pInst->setPosition(pInst->getPosition() + (pInst->getVelocity() * glm::vec3(dt, dt, 0.0f)));
		}
		else if (pInst->getType() == V_ENEMY)
		{
			pInst->setPosition(pInst->getPosition() + (pInst->getVelocity() * glm::vec3(dt, dt, 0.0f)));
			if (pInst->getCurrentFrame() % 5 == 0)
			{
				pInst->setTexPosX(pInst->getTexPosX() + 1.0f / 11.0f);
				if (pInst->getTexPosX() > 2.1f / 11.0f)
				{
					pInst->setTexPosX(1.0 / 11.0f);
				}
			}
		}
		else if (pInst->getType() == H_ENEMY)
		{
			pInst->setPosition(pInst->getPosition() + (pInst->getVelocity() * glm::vec3(dt, dt, 0.0f)));
			if (pInst->getCurrentFrame() % 5 == 0)
			{
				pInst->setTexPosX(pInst->getTexPosX() + 1.0f / 11.0f);
				if (pInst->getTexPosX() > 2.1f / 11.0f)
				{
					pInst->setTexPosX(1.0 / 11.0f);
				}
			}
		}
		else if (pInst->getType() == BOMB)
		{
			if (pInst->getCurrentFrame() % 5 == 0)
			{
				pInst->setTexPosX(pInst->getTexPosX() + (1.0f / 5.0f));
				if (pInst->getTexPosX() > 4.1f / 5.0f)
				{
					pInst->setTexPosX(1.0f / 5.0f);
				}
			}

			// BOMB timer decrease
			pInst->setTimer(pInst->getTimer() - dt);

			// If BOMB ready to explode
			if (pInst->getTimer() < 0.0f)
			{
				printf("BOOOOM!\n");

				engine->play2D("Explode.wav");

				int x = pInst->getPosition().x;
				int y = pInst->getPosition().y;

				// Set from 'Q' to '-'(Empty)
				map[y / OBJECT_SIZE][x / OBJECT_SIZE] = '-';

				// Check when explode with brick -> Stop explode ar that direction
				bool continue_left = true;
				bool continue_right = true;
				bool continue_top = true;
				bool continue_bottom = true;

				// Center Explode
				GameObject* obj = gameObjInstCreate(EXPLODE_BEGIN, glm::vec3(x, y, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(OBJECT_SIZE, OBJECT_SIZE, 1.0f), 0.0f);
				obj->setExplodeTime(EXPLODE_TIME);
				for (int radius = 1; radius <= pInst->getBombRadius(); radius++)
				{
					int left = x - radius * OBJECT_SIZE;
					int right = x + radius * OBJECT_SIZE;
					int top = y + radius * OBJECT_SIZE;
					int bottom = y - radius * OBJECT_SIZE;

					int ExplodeType = (radius == pInst->getBombRadius() ? EXPLODE_END : EXPLODE_MID);
					// Left Explode
					if (continue_left && left > 0 && map[(int)y / OBJECT_SIZE][left / OBJECT_SIZE] != 'B' && map[(int)y / OBJECT_SIZE][left / OBJECT_SIZE] != 'Q')
					{
						GameObject* obj = gameObjInstCreate(ExplodeType, glm::vec3(left, y, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(OBJECT_SIZE, OBJECT_SIZE, 1.0f), 0.0f);
						obj->setExplodeTime(EXPLODE_TIME);
						if (map[(int)y / OBJECT_SIZE][left / OBJECT_SIZE] == 'O') { continue_left = false; }
					}
					else { continue_left = false; }
					// Right Explode
					if (continue_right && right < MAP_WIDTH * OBJECT_SIZE && map[(int)y / OBJECT_SIZE][right / OBJECT_SIZE] != 'B' && map[(int)y / OBJECT_SIZE][right / OBJECT_SIZE] != 'Q')
					{
						GameObject* obj = gameObjInstCreate(ExplodeType, glm::vec3(right, y, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(OBJECT_SIZE, OBJECT_SIZE, 1.0f), 180.0f);
						obj->setExplodeTime(EXPLODE_TIME);
						if (map[(int)y / OBJECT_SIZE][right / OBJECT_SIZE] == 'O') { continue_right = false; }
					}
					else { continue_right = false; }
					// Top Explode
					if (continue_top && top < MAP_HEIGHT * OBJECT_SIZE && map[top / OBJECT_SIZE][(int)x / OBJECT_SIZE] != 'B' && map[top / OBJECT_SIZE][(int)x / OBJECT_SIZE] != 'Q')
					{
						GameObject* obj = gameObjInstCreate(ExplodeType, glm::vec3(x, top, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(OBJECT_SIZE, OBJECT_SIZE, 1.0f), -90.0f);
						obj->setExplodeTime(EXPLODE_TIME);
						if (map[top / OBJECT_SIZE][(int)x / OBJECT_SIZE] == 'O') { continue_top = false; }
					}
					else { continue_top = false; }
					// Bottom Explode
					if (continue_bottom && bottom > 0.0f && map[bottom / OBJECT_SIZE][(int)x / OBJECT_SIZE] != 'B' && map[bottom / OBJECT_SIZE][(int)x / OBJECT_SIZE] != 'Q')
					{
						GameObject* obj = gameObjInstCreate(ExplodeType, glm::vec3(x, bottom, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(OBJECT_SIZE, OBJECT_SIZE, 1.0f), 90.0f);
						obj->setExplodeTime(EXPLODE_TIME);
						if (map[bottom / OBJECT_SIZE][(int)x / OBJECT_SIZE] == 'O') { continue_bottom = false; }
					}
					else { continue_bottom = false; }
				}

				// If Explode done -> return BOMB to player
				if (pInst->getBomber() == F_PLAYER)
				{
					fPlayer->setBombAmount(fPlayer->getBombAmount() + 1);
				}
				else
				{
					sPlayer->setBombAmount(sPlayer->getBombAmount() + 1);
				}

				// Destroy BOMB
				gameObjInstDestroy(*pInst);
			}
		}
		else if (pInst->getType() == EXPLODE_BEGIN || pInst->getType() == EXPLODE_MID || pInst->getType() == EXPLODE_END)
		{
			// Explode time decrease
			pInst->setExplodeTime(pInst->getExplodeTime() - dt);

			// Explode effect disappear
			if (pInst->getExplodeTime() < 0.0f)
			{
				gameObjInstDestroy(*pInst);
			}
		}
		else if (pInst->getType() >= SPEED_UP && pInst->getType() <= BOMB_DOWN)
		{
			if (pInst->getTimeBeforeActive() > 0.0f)
			{
				pInst->setTimeBeforeActive(pInst->getTimeBeforeActive() - dt);
			}
		}
	}

	//-----------------------------------------
	// Update modelMatrix of all game obj
	//-----------------------------------------

	for (int i = 0; i < GAME_OBJ_INST_MAX; i++) {
		GameObject* pInst = sGameObjInstArray + i;

		// skip inactive object
		if (pInst->getFlag() == FLAG_INACTIVE)
			continue;

		glm::mat4 rMat = glm::mat4(1.0f);
		glm::mat4 sMat = glm::mat4(1.0f);
		glm::mat4 tMat = glm::mat4(1.0f);

		// Compute the scaling matrix
		sMat = glm::scale(glm::mat4(1.0f), pInst->getScale());

		//+ Compute the rotation matrix, we should rotate around z axis 
		rMat = glm::rotate(glm::mat4(1.0f), glm::radians(pInst->getOrientation()), glm::vec3(0.0f, 0.0f, 1.0f));

		//+ Compute the translation matrix
		tMat = glm::translate(glm::mat4(1.0f), pInst->getPosition());

		// Concatenate the 3 matrix to from Model Matrix
		pInst->setModelMatrix(tMat * sMat * rMat);
	}

	//printf("Life> %i\n", fPlayerLives);
	//printf("Score> %i\n", sScore);
}

void GameStateLevel1Draw(void) {

	// Clear the screen
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// draw all game object instance in the sGameObjInstArray
	for (int i = 0; i < GAME_OBJ_INST_MAX; i++) {
		GameObject* pInst = sGameObjInstArray + i;

		// skip inactive object
		if (pInst->getFlag() == FLAG_INACTIVE)
			continue;

		// 4 steps to draw sprites on the screen
		//	1. SetRenderMode()
		//	2. SetTexture()
		//	3. SetTransform()
		//	4. DrawMesh()

		SetRenderMode(CDT_TEXTURE, 1.0f);
		SetTexture(*pInst->getTexture(), pInst->getTexPosX(), pInst->getTexPosY());
		SetTransform(pInst->getModelMatrix());
		DrawMesh(*pInst->getMesh());
	}
	// Swap the buffer, to present the drawing
	glfwSwapBuffers(window);
}

void GameStateLevel1Free(void) {

	//+ call gameObjInstDestroy for all object instances in the sGameObjInstArray
	for (int i = 0; i < GAME_OBJ_INST_MAX; i++)
	{
		gameObjInstDestroy(sGameObjInstArray[i]);
	}

	// Reset Map
	for (int y = 0; y < MAP_HEIGHT; y++)
	{
		for (int x = 0; x < MAP_WIDTH; x++)
		{
			map[y][x] = mapdata[y][x];
		}
	}

	// Reset Enemy Amount
	currentEnemy = 0;
	gameEndTime = 0.0f;
	doOnce = true;

	// reset camera
	ResetCam();

	engine->drop();

	printf("Level1: Free\n");
}

void GameStateLevel1Unload(void) {

	// Unload all meshes in MeshArray
	for (int i = 0; i < sNumMesh; i++) {
		UnloadMesh(sMeshArray[i]);
	}

	//+ Unload all textures in TexArray
	for (int i = 0; i < sNumTex; i++)
	{
		TextureUnload(sTexArray[i]);
	}


	printf("Level1: Unload\n");
}

bool CheckCollision(glm::vec3 aPos, glm::vec3 aSize, glm::vec3 bPos, glm::vec3 bSize)
{
	float leftA = aPos.x - aSize.x / 2;
	float rightA = aPos.x + aSize.x / 2;
	float topA = aPos.y + aSize.y / 2;
	float bottomA = aPos.y - aSize.y / 2;

	float leftB = bPos.x - bSize.x / 2;
	float rightB = bPos.x + bSize.x / 2;
	float topB = bPos.y + bSize.y / 2;
	float bottomB = bPos.y - bSize.y / 2;

	return (leftA <= rightB && leftB <= rightA && bottomA <= topB && bottomB <= topA ? true : false);
}

int CheckEnemyType(int x, int y)
{
	if (map[y + 1][x] == '-' || map[y - 1][x] == '-')
	{
		return V_ENEMY;
	}
	else
	{
		return H_ENEMY;
	}
}

bool CheckEnemySpawn(int x, int y)
{
	if (map[y - 1][x] == 'X' || map[y + 1][x] == 'X' || map[y][x + 1] == 'X' || map[y][x - 1] == 'X')
	{
		return false;
	}
	return true;
}

glm::vec3 CalculateBombPosition(float pos_x, float pos_y)
{
	// 50.0 -> Default Size of all Entities
	int x = (int)((pos_x + OBJECT_SIZE / 2.0f) / OBJECT_SIZE) * OBJECT_SIZE;
	int y = (int)((pos_y + OBJECT_SIZE / 2.0f) / OBJECT_SIZE) * OBJECT_SIZE;
	return glm::vec3(x, y, 2.0f);
}

// For Debugging
void printMap()
{
	for (int y = 0; y < MAP_HEIGHT; y++)
	{
		for (int x = 0; x < MAP_WIDTH; x++)
		{
			printf("%c ", map[y][x]);
		}
		printf("\n");
	}
	printf("\n\n");
}