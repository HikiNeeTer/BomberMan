#include "GameObject.h"

// Function
float GameObject::getLeftBound() const 
{
	return position.x - scale.x / 2;
}

float GameObject::getRightBound() const
{
	return position.x + scale.x / 2;
}

float GameObject::getTopBound() const
{
	return position.y + scale.y / 2;
}


float GameObject::getBottomBound() const
{
	return position.y - scale.y / 2;
}

/* GameObject */
// Getter Implement
CDTMesh* GameObject::getMesh() const { return this->mesh; }
CDTTex* GameObject::getTexture() const { return this->tex; }
int GameObject::getType() const { return this->type; }
int GameObject::getFlag() const { return this->flag; }
glm::vec3 GameObject::getPosition() const { return this->position; }
glm::vec3 GameObject::getVelocity() const { return this->velocity; }
glm::vec3 GameObject::getScale() const { return this->scale; }
float GameObject::getOrientation() const { return this->orientation; }
glm::mat4 GameObject::getModelMatrix() const { return this->modelMatrix; }
int GameObject::getCurrentFrame() const { return currentFrame; }
float GameObject::getTexPosX() const { return texPosX; }
float GameObject::getTexPosY() const { return texPosY; }

// Setter Implement
void GameObject::setMesh(CDTMesh* mesh) { this->mesh = mesh; }
void GameObject::setTexture(CDTTex* texture) { this->tex = texture; }
void GameObject::setType(int type) { this->type = type; }
void GameObject::setFlag(int flag) { this->flag = flag; }
void GameObject::setPosition(glm::vec3 position) { this->position = position; }
void GameObject::setPositionX(float position_x) { this->position.x = position_x; }
void GameObject::setPositionY(float position_y) { this->position.y = position_y; }

void GameObject::setVelocity(glm::vec3 velocity) { this->velocity = velocity; }
void GameObject::setVelocityX(float velocity_x) { this->velocity.x = velocity_x; }
void GameObject::setVelocityY(float velocity_y) { this->velocity.y = velocity_y; }

void GameObject::setScale(glm::vec3 scale) { this->scale = scale; }
void GameObject::setOrientation(float orientation) { this->orientation = orientation; }
void GameObject::setModelMatrix(glm::mat4 modelMatrix) { this->modelMatrix = modelMatrix; }

void GameObject::setCurrentFrame(int frame) { this->currentFrame = frame; }

void GameObject::setTexPosX(float x) { this->texPosX = x; }
void GameObject::setTexPosY(float y) { this->texPosY = y; }

/* Player */
// Function Implement
// Buff
void GameObject::SpeedUp()
{
	if (this->speed.x <= PLAYER_SPEED)
	{
		this->speed.x = PLAYER_SPEED_UP;
		this->speed.y = PLAYER_SPEED_UP;
	}
}
void GameObject::BombRadiusIncrease()
{
	if (this->bombRadius < MAX_BOMB_RADIUS)
	{
		this->bombRadius++;
	}
}
void GameObject::BombIncerase()
{
	if (this->allBomb < MAX_BOMB)
	{
		this->allBomb++;
		this->bombAmount++;
	}
}
// DeBuff
void GameObject::SpeedDown()
{
	if (this->speed.x > PLAYER_SPEED)
	{
		this->speed.x = PLAYER_SPEED;
		this->speed.y = PLAYER_SPEED;
	}
}
void GameObject::BombRadiusDecrease()
{
	if (this->bombRadius > 1)
	{
		this->bombRadius--;
	}
}
void GameObject::BombDecrease()
{
	if (this->allBomb > 1)
	{
		this->allBomb--;
		this->bombAmount--;
	}
}

// Getter Implement
float GameObject::getSpeedX() const { return speed.x; }
float GameObject::getSpeedY() const { return speed.y; }
int GameObject::getBombAmount() const { return bombAmount; }
int GameObject::getBomber() const { return bomber; }
int GameObject::getBombRadius() const { return bombRadius; }
int GameObject::getAllBomb() const { return allBomb; }

// Setter Implement
void GameObject::setSpeedX(float speed) { this->speed.x = speed; }
void GameObject::setSpeedY(float speed) { this->speed.y = speed; }
void GameObject::setBombAmount(int amount) { this->bombAmount = amount; }
void GameObject::setBomber(int owner) { this->bomber = owner; }
void GameObject::setBombRadius(int radius) { this->bombRadius = radius; }
void GameObject::setAllBomb(int allBomb) { this->allBomb = allBomb; }

/* Bomber */
// Getter Implement
float GameObject::getTimer() const { return bombTimer; }
// Setter Implement
void GameObject::setTimer(float timer) { this->bombTimer = timer; }

/* Explode */
// Getter Implement
float GameObject::getExplodeTime() const { return explodeTime; }
// Setter Implement
void GameObject::setExplodeTime(float time) { this->explodeTime = time; }


/*Abilities*/
// Getter Implement
float GameObject::getTimeBeforeActive() const { return timeBeforeActive; }
// Setter Implement
void GameObject::setTimeBeforeActive(float time) { this->timeBeforeActive = time; }