#pragma once

#include "CDT.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>

#define PLAYER_SPEED 125.0f
#define PLAYER_SPEED_UP PLAYER_SPEED * 1.5f

#define MAX_BOMB_RADIUS 3
#define MAX_BOMB 4

class GameObject
{
	private:
		CDTMesh* mesh;
		CDTTex* tex;
		int				type;				// enum GAMEOBJ_TYPE
		int				flag;				// 0 - inactive, 1 - active
		glm::vec3		position;			// usually we will use only x and y
		glm::vec3		velocity;			// usually we will use only x and y
		glm::vec3		scale;				// usually we will use only x and y
		float			orientation;		// 0 radians is 3 o'clock, PI/2 radian is 12 o'clock
		glm::mat4		modelMatrix;
		int currentFrame;
		float texPosX;
		float texPosY;

		/* Player */
		glm::vec3 speed = glm::vec3(PLAYER_SPEED, PLAYER_SPEED, 0.0f);
		int bombAmount;
		int allBomb;
		int bomber;

		/* Bomb */
		float bombTimer;
		int bombRadius;

		/* Explode */
		float explodeTime;

		/*Abilities*/
		float timeBeforeActive;

	public:

		// Function
		float getLeftBound() const;
		float getRightBound() const;
		float getTopBound() const;
		float getBottomBound() const;

		/* GameObject */
		// Getter
		CDTMesh* getMesh() const;
		CDTTex* getTexture() const;
		int getType() const;
		int getFlag() const;
		glm::vec3 getPosition() const;
		glm::vec3 getVelocity() const;
		glm::vec3 getScale() const;
		float getOrientation() const;
		glm::mat4 getModelMatrix() const;
		int getCurrentFrame() const;
		float getTexPosX() const;
		float getTexPosY() const;

		//Setter
		void setMesh(CDTMesh* mesh);
		void setTexture(CDTTex* texture);
		void setType(int type);
		void setFlag(int flag);
		void setPosition(glm::vec3 position);
		void setPositionX(float position_x);
		void setPositionY(float position_y);

		void setVelocity(glm::vec3 velocity);
		void setVelocityX(float velocity_x);
		void setVelocityY(float velocity_y);

		void setScale(glm::vec3 scale);
		void setOrientation(float orientation);
		void setModelMatrix(glm::mat4 modelMatrix);
		
		void setCurrentFrame(int frame);
		
		void setTexPosX(float x);
		void setTexPosY(float y);

		/* Player */
		// Function
		void SpeedUp();
		void BombRadiusIncrease();
		void BombIncerase();

		void SpeedDown();
		void BombRadiusDecrease();
		void BombDecrease();
		// Getter
		float getSpeedX() const;
		float getSpeedY() const;
		int getBombAmount() const;
		int getBomber() const;
		int getBombRadius() const;
		int getAllBomb() const;
		// Setter
		void setSpeedX(float speed);
		void setSpeedY(float speed);
		void setBombAmount(int amount);
		void setBomber(int owner);
		void setBombRadius(int radius);
		void setAllBomb(int allBomb);

		/* Bomb */
		// Getter
		float getTimer() const;
		// Setter
		void setTimer(float timer);

		/* Explode */
		// Getter
		float getExplodeTime() const;
		// Setter
		void setExplodeTime(float time);

		/*Abilities*/
		// Getter
		float getTimeBeforeActive() const;
		// Setter
		void setTimeBeforeActive(float time);

};