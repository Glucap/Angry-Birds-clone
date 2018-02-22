/*
	"Angry Birds" style example
*/
#include "Blit3D.h"

#include "Box2d/Box2d.h"
#include "GroundEntity.h"
#include "ShotEntity.h"
#include "Particle.h"
#include "Physics.h"
#include "MyContactListener.h" 
#include "EdgeEntity.h"
#include "Cannon.h"
#include "Meter.h"
#include "Camera.h"
#include "BlockEntity.h"
#include "CollisionMask.h"


int scale1, scale2 = 0;

Blit3D *blit3D = NULL;

//memory leak detection
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

//GLOBAL DATA
b2World *world;
// Prepare for simulation. Typically we use a time step of 1/60 of a
// second (60Hz) and ~10 iterations. This provides a high quality simulation
// in most game scenarios.
int32 velocityIterations = 8;
int32 positionIterations = 3;
float timeStep = 1.f / 60.f; //one 60th of a second
float elapsedTime = 0; //used for calculating time passed
float settleTime = 0;

//contact listener to handle collisions between important objects
MyContactListener *contactListener;

enum GameState { START, PLAYING, SETTLING, WIN };
GameState gameState = START;
bool attachedShot = true; //is the ball ready to be launched from the paddle?
int lives = 3;

std::vector<Entity *> blockEntityList; //bricks go here
std::vector<Entity *> shotEntityList; //track the balls seperately from everything else
std::vector<Entity *> entityList; //other entities in our game go here
std::vector<Entity *> deadEntityList; //dead entities
std::vector<Entity *> enemyList;

std::vector<Particle *> particleList;

Sprite *cannonballSprite = NULL;
Sprite *groundSprite = NULL;
Sprite *cactusSprite = NULL;
Sprite *StartSprite = NULL;
Sprite *Credits = NULL;

Sprite* Win = NULL;
Sprite* Yay = NULL;



Cannon cannon;

bool gameOver;
bool fireShotNow = false;
bool followingShot = false; //is the camera tracking the shot?
double gameOverTimer = 0;
Meter meter;

Camera2D *camera; //pans the view

std::vector<Sprite *> blockSprites;
std::vector<Sprite *> debrisList;

//ensures that entities are only added ONCE to the deadEntityList
void AddToDeadList(Entity *e)
{
	bool unique = true;

	for (auto ent : deadEntityList)
	{
		if (ent == e)
		{
			unique = false;
			break;
		}
	}

	if (unique) deadEntityList.push_back(e);
}

//Function that applies damage etc when a block collides with anything
void BlockCollide(Entity *A, float maxImpulseAB)
{
	BlockEntity *blockEntity = (BlockEntity *)A;
	//damage?
	if (maxImpulseAB > 0.7f) //cutoff for no damage
	{
		//apply some damage
		if (blockEntity->Damage((int)(maxImpulseAB *30.f)))
		{
			//Damage() returned true, need to kill this block
			AddToDeadList(A);

			//spawn particles here
			//debrisList
			if (blockEntity->typeID == EntityTypes::ENTITYBLOCK)
			for (int particleCount = 0; particleCount < 10; ++particleCount)
			{
				Particle *p = new Particle();
				p->coords = Physics2Pixels(A->body->GetPosition());
				p->angle = rand() % 360;
				p->direction = deg2vec(rand() % 360);
				p->rotationSpeed = (float)(rand() % 1000) / 100 - 5;
				p->startingSpeed = rand() % 200;
				p->targetSpeed = rand() % 200;
				p->totalTimeToLive = 0.3f;

				p->startingScaleX = (float)(rand() % 100) / 200 + 0.1;
				p->startingScaleY = (float)(rand() % 100) / 200 + 0.1;
				p->targetScaleX = (float)(rand() % 100) / 1000 + 0.05;
				p->targetScaleY = (float)(rand() % 100) / 1000 + 0.05;
				
				p->spriteList.push_back(debrisList[(rand() % 3) + ((int)blockEntity->materialType) * 3]);
				particleList.push_back(p);
			}
		}
	}
}

void Init()
{
	blit3D->ShowCursor(false);

	//make a camera
	camera = new Camera2D();

	//set it's valid pan area
	camera->minX = blit3D->screenWidth / 2;
	camera->minY = blit3D->screenHeight / 2;
	camera->maxX = blit3D->screenWidth * 2 - blit3D->screenWidth/2;
	camera->maxY = blit3D->screenHeight / 2 + 400;

	camera->PanTo(blit3D->screenWidth / 2, blit3D->screenHeight / 2);

	//load the sprites
	cannonballSprite = blit3D->MakeSprite(0, 0, 36, 36, "Media\\Cannonball.png");
	cannon.sprite = blit3D->MakeSprite(0, 0, 440, 128, "Media\\barrel-h.png");

	cannon.positionPixels = b2Vec2(100, 150);

	meter.sprite = blit3D->MakeSprite(0, 0, 100, 100, "Media\\meter.png");
	meter.positionPixels = cannon.positionPixels;

	//ground block + cactus
	groundSprite = blit3D->MakeSprite(280, 70, 70, 70, "Media\\spritesheet_tiles.png");
	cactusSprite = blit3D->MakeSprite(83, 81, 45, 59, "Media\\spritesheet_tiles.png");

	//load the block sprites
	
	//trigangle
		//wood
		blockSprites.push_back(blit3D->MakeSprite(581, 1841, 138, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(941, 791 , 138, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(581, 1911, 138, 68, "Media\\spritesheet_elements.png"));
		//metal
		blockSprites.push_back(blit3D->MakeSprite(1001, 1871, 138, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1361, 351, 138, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1141, 1861, 138, 68, "Media\\spritesheet_elements.png"));
		//glass
		blockSprites.push_back(blit3D->MakeSprite(1221, 721, 138, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1141, 1931, 138, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1001, 931, 138, 68, "Media\\spritesheet_elements.png"));
		//rock
		blockSprites.push_back(blit3D->MakeSprite(1001, 1941, 138, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(941, 791, 138, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1081, 711, 138, 68, "Media\\spritesheet_elements.png"));
	//rectangle 
		//wood
		blockSprites.push_back(blit3D->MakeSprite(1  , 1331, 218, 138, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1  , 351 , 218, 138, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(441, 701 , 218, 138, "Media\\spritesheet_elements.png"));
		//metal
		blockSprites.push_back(blit3D->MakeSprite(221, 421, 218, 138, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(221, 981, 218, 138, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(221, 281, 218, 138, "Media\\spritesheet_elements.png"));
		//glass
		blockSprites.push_back(blit3D->MakeSprite(1, 491, 218, 138, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1, 771, 218, 138, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1, 1611, 218, 138, "Media\\spritesheet_elements.png"));
		//rock
		blockSprites.push_back(blit3D->MakeSprite(441, 911, 218, 138, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(441, 1051, 218, 138, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(441, 1, 218, 138, "Media\\spritesheet_elements.png"));
	//long bar
		//wood
		blockSprites.push_back(blit3D->MakeSprite(1841, 711, 68, 218, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1561, 931 , 68, 218, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1491, 781, 68, 218, "Media\\spritesheet_elements.png"));
		//metal
		blockSprites.push_back(blit3D->MakeSprite(1631, 1721, 68, 218, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1631, 1001, 68, 218, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1771, 711, 68, 218, "Media\\spritesheet_elements.png"));
		//glass
		blockSprites.push_back(blit3D->MakeSprite(1841, 1711, 68, 218, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1846, 1141, 68, 218, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1701, 1761, 68, 218, "Media\\spritesheet_elements.png"));
		//rock
		blockSprites.push_back(blit3D->MakeSprite(1421, 921, 68, 218, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1421, 1701, 68, 218, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1491, 1281, 68, 218, "Media\\spritesheet_elements.png"));
	//short bar
		//wood
		blockSprites.push_back(blit3D->MakeSprite(941, 1, 138, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(941, 791 , 138, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(581, 1771, 138, 68, "Media\\spritesheet_elements.png"));
		//metal
		blockSprites.push_back(blit3D->MakeSprite(1281, 1791, 138, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1281, 1721, 138, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(581, 1911, 138, 68, "Media\\spritesheet_elements.png"));
		//glass
		blockSprites.push_back(blit3D->MakeSprite(861, 1931, 138, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(861, 1721, 138, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(941, 861, 138, 68, "Media\\spritesheet_elements.png"));
		//rock
		blockSprites.push_back(blit3D->MakeSprite(581, 1841, 138, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1281, 861, 138, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(581, 1911, 138, 68, "Media\\spritesheet_elements.png"));
	//ball
		//wood
		blockSprites.push_back(blit3D->MakeSprite(1491, 1571, 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1701, 1691 , 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1501, 141, 68, 68, "Media\\spritesheet_elements.png"));
		//metal
		blockSprites.push_back(blit3D->MakeSprite(1701, 1551, 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1701, 1131, 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1501, 431, 68, 68, "Media\\spritesheet_elements.png"));
		//glass
		blockSprites.push_back(blit3D->MakeSprite(1911, 1281, 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1911, 861, 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1771, 641, 68, 68, "Media\\spritesheet_elements.png"));
		//rock
		blockSprites.push_back(blit3D->MakeSprite(1561, 1861, 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1631, 1651, 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1501, 431, 68, 68, "Media\\spritesheet_elements.png"));
	//small triangle
		//wood
		blockSprites.push_back(blit3D->MakeSprite(1421, 1491, 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1701, 1621 , 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1491, 1781, 68, 68, "Media\\spritesheet_elements.png"));
		//metal
		blockSprites.push_back(blit3D->MakeSprite(1701, 1201, 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1701, 1271, 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1501, 641, 68, 68, "Media\\spritesheet_elements.png"));
		//glass
		blockSprites.push_back(blit3D->MakeSprite(1911, 1071, 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(941, 791, 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1771, 931, 68, 68, "Media\\spritesheet_elements.png"));
		//rock
		blockSprites.push_back(blit3D->MakeSprite(1701, 1411, 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1631, 1941, 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1421, 1211, 68, 68, "Media\\spritesheet_elements.png"));
	//big square
		//wood
		blockSprites.push_back(blit3D->MakeSprite(861, 1511 , 138, 138, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(801, 651, 138, 138, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(581, 1411, 138, 138, "Media\\spritesheet_elements.png"));
		//metal
		blockSprites.push_back(blit3D->MakeSprite(1361, 141, 138, 138, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1081, 211, 138, 138, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(861, 1151, 138, 138, "Media\\spritesheet_elements.png"));
		//glass
		blockSprites.push_back(blit3D->MakeSprite(801, 791, 138, 138, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1361, 1, 138, 138, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1281, 1291, 138, 138, "Media\\spritesheet_elements.png"));
		//rock
		blockSprites.push_back(blit3D->MakeSprite(1281, 1861, 138, 138, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(801, 441, 138, 138, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1081, 1, 138, 138, "Media\\spritesheet_elements.png"));
	//small square
		//wood
		blockSprites.push_back(blit3D->MakeSprite(1771, 1571, 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1771, 1861 , 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1491, 1501, 68, 68, "Media\\spritesheet_elements.png"));
		//metal
		blockSprites.push_back(blit3D->MakeSprite(1701, 851, 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1701, 571, 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1841, 1931, 68, 68, "Media\\spritesheet_elements.png"));
		//glass
		blockSprites.push_back(blit3D->MakeSprite(1911, 931, 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1911, 721, 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1771, 571, 68, 68, "Media\\spritesheet_elements.png"));
		//rock
		blockSprites.push_back(blit3D->MakeSprite(1561, 1431, 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1491, 1001, 68, 68, "Media\\spritesheet_elements.png"));
		blockSprites.push_back(blit3D->MakeSprite(1561, 1931, 68, 68, "Media\\spritesheet_elements.png"));
	//load pet sprites
		blockSprites.push_back(blit3D->MakeSprite(0, 0, 68, 66, "media\\pet1.png"));
		blockSprites.push_back(blit3D->MakeSprite(0, 0, 68, 66, "media\\pet1.png"));
		blockSprites.push_back(blit3D->MakeSprite(0, 0, 84, 60, "media\\pet2.png"));

		blockSprites.push_back(blit3D->MakeSprite(0, 0, 68, 72, "media\\pet3.png"));
		blockSprites.push_back(blit3D->MakeSprite(0, 0, 68, 72, "media\\pet3.png"));
		blockSprites.push_back(blit3D->MakeSprite(0, 0, 68, 72, "media\\pet4.png"));

		blockSprites.push_back(blit3D->MakeSprite(0, 0, 74, 84, "media\\pet5.png"));
		blockSprites.push_back(blit3D->MakeSprite(0, 0, 74, 84, "media\\pet5.png"));
		blockSprites.push_back(blit3D->MakeSprite(0, 0, 74, 84, "media\\pet6.png"));

		blockSprites.push_back(blit3D->MakeSprite(0, 0, 92, 88, "media\\pet7.png"));
		blockSprites.push_back(blit3D->MakeSprite(0, 0, 92, 88, "media\\pet7.png"));
		blockSprites.push_back(blit3D->MakeSprite(0, 0, 94, 80, "media\\pet8.png"));

	

	//load debris sprites
		//wood
	debrisList.push_back(blit3D->MakeSprite(0, 57, 64, 55, "Media\\spritesheet_debris.png"));
	debrisList.push_back(blit3D->MakeSprite(0, 168, 64, 52, "Media\\spritesheet_debris.png"));
	debrisList.push_back(blit3D->MakeSprite(0, 446, 68, 51, "Media\\spritesheet_debris.png"));
		//metal
	debrisList.push_back(blit3D->MakeSprite(0, 113, 64, 55, "Media\\spritesheet_debris.png"));
	debrisList.push_back(blit3D->MakeSprite(0, 220, 64, 52, "Media\\spritesheet_debris.png"));
	debrisList.push_back(blit3D->MakeSprite(0, 324, 68, 51, "Media\\spritesheet_debris.png"));
		//glass
	debrisList.push_back(blit3D->MakeSprite(0, 0, 64, 55, "Media\\spritesheet_debris.png"));
	debrisList.push_back(blit3D->MakeSprite(0, 272, 64, 52, "Media\\spritesheet_debris.png"));
	debrisList.push_back(blit3D->MakeSprite(0, 385, 68, 51, "Media\\spritesheet_debris.png"));
		//rock
	debrisList.push_back(blit3D->MakeSprite(0, 113, 64, 55, "Media\\spritesheet_debris.png"));
	debrisList.push_back(blit3D->MakeSprite(0, 220, 64, 52, "Media\\spritesheet_debris.png"));
	debrisList.push_back(blit3D->MakeSprite(0, 324, 68, 51, "Media\\spritesheet_debris.png"));

	//load pet sprites
	blockSprites.push_back(blit3D->MakeSprite(0, 0, 34, 33, "media\\pet1.png"));
	blockSprites.push_back(blit3D->MakeSprite(0, 0, 34, 33, "media\\pet1.png"));
	blockSprites.push_back(blit3D->MakeSprite(0, 0, 42, 30, "media\\pet2.png"));

	blockSprites.push_back(blit3D->MakeSprite(0, 0, 34, 36, "media\\pet3.png"));
	blockSprites.push_back(blit3D->MakeSprite(0, 0, 34, 36, "media\\pet3.png"));
	blockSprites.push_back(blit3D->MakeSprite(0, 0, 34, 36, "media\\pet4.png"));

	blockSprites.push_back(blit3D->MakeSprite(0, 0, 37, 42, "media\\pet5.png"));
	blockSprites.push_back(blit3D->MakeSprite(0, 0, 37, 42, "media\\pet5.png"));
	blockSprites.push_back(blit3D->MakeSprite(0, 0, 37, 42, "media\\pet6.png"));

	blockSprites.push_back(blit3D->MakeSprite(0, 0, 46, 44, "media\\pet7.png"));
	blockSprites.push_back(blit3D->MakeSprite(0, 0, 46, 44, "media\\pet7.png"));
	blockSprites.push_back(blit3D->MakeSprite(0, 0, 47, 40, "media\\pet8.png"));


	// Define the gravity vector.
	b2Vec2 gravity(0.f, -9.8f);

	// Construct a world object, which will hold and simulate the rigid bodies.
	world = new b2World(gravity);
	//world->SetGravity(gravity); <-can call this to change gravity at any time
	world->SetAllowSleeping(true); //set true to allow the physics engine to 'sleep" objects that stop moving

								   //_________GROUND OBJECT_____________
								   //make an entity for the ground
	GroundEntity *groundEntity = new GroundEntity();
	//A bodyDef for the ground
	b2BodyDef groundBodyDef;
	// Define the ground body.
	groundBodyDef.position.Set(0, 0);

	// Call the body factory which allocates memory for the ground body
	// from a pool and creates the ground box shape (also from a pool).
	// The body is also added to the world.
	groundEntity->body = world->CreateBody(&groundBodyDef);

	//an EdgeShape object, for the ground
	b2EdgeShape groundBox;

	// Define the ground as 1 edge shape at the bottom of the screen.
	b2FixtureDef boxShapeDef;

	boxShapeDef.shape = &groundBox;

	//collison masking
	boxShapeDef.filter.categoryBits = CMASK_GROUND;  //this is the ground
	boxShapeDef.filter.maskBits = CMASK_SHOT | CMASK_BLOCK | CMASK_ALIEN;		//it collides wth balls and powerups

																				//bottom
	groundBox.Set(b2Vec2(0, 70 / PTM_RATIO), b2Vec2(blit3D->screenWidth * 2 / PTM_RATIO, 70 / PTM_RATIO));
	//Create the fixture
	groundEntity->body->CreateFixture(&boxShapeDef);
	//add the userdata
	groundEntity->body->SetUserData(groundEntity);
	//add to the entity list
	entityList.push_back(groundEntity);

	//now make the other 3 edges of the screen on a seperate entity/body
	EdgeEntity * edgeEntity = new EdgeEntity();
	edgeEntity->body = world->CreateBody(&groundBodyDef);

	boxShapeDef.filter.categoryBits = CMASK_EDGES;  //this is the ground
	boxShapeDef.filter.maskBits = CMASK_SHOT | CMASK_BLOCK | CMASK_ALIEN;		//it collides wth balls

																				//left
	groundBox.Set(b2Vec2(0, blit3D->screenHeight * 2 / PTM_RATIO), b2Vec2(0, 70 / PTM_RATIO));
	edgeEntity->body->CreateFixture(&boxShapeDef);

	//top
	groundBox.Set(b2Vec2(0, blit3D->screenHeight * 2 / PTM_RATIO),
		b2Vec2(blit3D->screenWidth * 2 / PTM_RATIO, blit3D->screenHeight * 2 / PTM_RATIO));
	edgeEntity->body->CreateFixture(&boxShapeDef);

	//right
	groundBox.Set(b2Vec2(blit3D->screenWidth * 2 / PTM_RATIO, 70 / PTM_RATIO),
		b2Vec2(blit3D->screenWidth * 2 / PTM_RATIO, blit3D->screenHeight * 2 / PTM_RATIO));
	edgeEntity->body->CreateFixture(&boxShapeDef);

	edgeEntity->body->SetUserData(edgeEntity);
	entityList.push_back(edgeEntity);

	// Create contact listener and use it to collect info about collisions
	contactListener = new MyContactListener();
	world->SetContactListener(contactListener);

	BlockEntity *block = MakeBlock(BlockType::LARGE_TRIANGLE, MaterialType::WOOD, b2Vec2(1800.f, 100.f), 0.f, 200);
	blockEntityList.push_back(block);
	block = MakeBlock(BlockType::LARGE_TRIANGLE, MaterialType::GLASS, b2Vec2(1800.f, 300.f), 0.f, 200);
	blockEntityList.push_back(block);

	block = MakeBlock(BlockType::LONG_BAR, MaterialType::ROCK, b2Vec2(1000.f, 100.f), 0.f, 600);
	blockEntityList.push_back(block);

	block = MakeBlock(BlockType::LONG_BAR, MaterialType::METAL, b2Vec2(1150.f, 100.f), 0.f, 600);
	blockEntityList.push_back(block);

	block = MakeBlock(BlockType::BALL, MaterialType::METAL, b2Vec2(1075.f, 390.f), 0.f, 600);
	blockEntityList.push_back(block);

	block = MakeBlock(BlockType::BIG_SQUARE, MaterialType::GLASS, b2Vec2(1075.f, 320.f), 0.f, 600);
	blockEntityList.push_back(block);

	block = MakeBlock(BlockType::PET, MaterialType::WOOD, b2Vec2(1050.f, 150), 0.f, 100);
	enemyList.push_back(block);

	StartSprite = blit3D->MakeSprite(0, 0, 366, 520, "Media\\Start.png");

	Credits = blit3D->MakeSprite(0, 0, 150, 200, "Media\\Credit.png");

	Win = blit3D->MakeSprite(0, 0, 598, 200, "Media\\YouWin.png");
	Yay = blit3D->MakeSprite(0, 0, 625, 200, "Media\\WayToGo.png");


}

void DeInit(void)
{
	if (camera != NULL) delete camera;
	
	//delete all particles
	for (auto p : particleList) delete p;
	particleList.clear();

	//delete all the entities
	for (auto e : entityList) delete e;
	entityList.clear();

	for (auto s : shotEntityList) delete s;
	shotEntityList.clear();

	for (auto b : blockEntityList) delete b;
	blockEntityList.clear();

	for (auto b : enemyList) delete b;
	enemyList.clear();

	//delete the contact listener
	delete contactListener;

	//Free all physics game data we allocated
	delete world;
	//any sprites still allocated are freed automatcally by the Blit3D object when we destroy it
}

void Update(double seconds)
{
	
	
	switch (gameState)
	{
	case SETTLING:
	{
		camera->Update((float)seconds);
		elapsedTime += seconds;
		while (elapsedTime >= timeStep)
		{
			//update the physics world
			world->Step(timeStep, velocityIterations, positionIterations);

			// Clear applied body forces. 
			world->ClearForces();

			settleTime += timeStep;
			elapsedTime -= timeStep;
		}

		if (settleTime > 2) gameState = PLAYING;
	}
	break;
	case PLAYING:
	{
		elapsedTime += seconds;

		if (fireShotNow)
		{

			fireShotNow = false;
			followingShot = true;

			
			if (lives > 0)
			{
				//fire a shot!
				ShotEntity *shot = MakeShot();
				b2Vec2 pos = Pixels2Physics(cannon.positionPixels);

				b2Vec2 dirCannon = Pixels2Physics(deg2vec(cannon.angle, 220));

				shot->body->SetTransform(pos + dirCannon, 0);

				b2Vec2 dir = deg2vec(cannon.angle, meter.scale * 3 + 1);

				shot->body->ApplyLinearImpulse(dir, shot->body->GetPosition(), true);
				shotEntityList.push_back(shot);
				lives--;

			}

		}

		//don't apply physics unless at least a timestep worth of time has passed
		while (elapsedTime >= timeStep)
		{
			//update the physics world
			world->Step(timeStep, velocityIterations, positionIterations);

			// Clear applied body forces. 
			world->ClearForces();

			elapsedTime -= timeStep;

			//update game logic/animation
			for (auto e : entityList) e->Update(timeStep);
			for (auto b : shotEntityList) b->Update(timeStep);
			for (auto b : blockEntityList) b->Update(timeStep);
			for (auto b : enemyList) b->Update(timeStep); 

			//update shot meter
			meter.Update(timeStep);

			//update cannon
			cannon.Update(timeStep);

			//update camera
			if (followingShot)
			{
				//make sure there is a shot to follow
				int size = shotEntityList.size();
				if (size > 0)
				{
					//last shot on list is the current active shot,
					//so follow it
					b2Vec2 pos = shotEntityList[size - 1]->body->GetPosition();
					pos = Physics2Pixels(pos);
					camera->PanTo(pos.x, pos.y);
				}				
			}
			camera->Update(timeStep);

			//update the particle list and remove dead particles
			for (int i = particleList.size() - 1; i >= 0; --i)
			{
				if (particleList[i]->Update(timeStep))
				{
					//time to die!
					delete particleList[i];
					particleList.erase(particleList.begin() + i);
				}
			}

			//loop over contacts
			for (int pos = 0; pos < contactListener->contacts.size(); ++pos)
			{
				MyContact contact = contactListener->contacts[pos];

				//fetch the entities from the body userdata
				Entity *A = (Entity *)contact.fixtureA->GetBody()->GetUserData();
				Entity *B = (Entity *)contact.fixtureB->GetBody()->GetUserData();

				if (A != NULL && B != NULL) //if there is an entity for these objects...
				{
					if (A->typeID == EntityTypes::ENTITYBLOCK)
					{
						BlockCollide(A, contact.maxImpulseAB);
					}

					if (B->typeID == EntityTypes::ENTITYBLOCK)
					{
						BlockCollide(B, contact.maxImpulseAB);
					}
				}
			}

			//clean up dead entities
			for (auto e : deadEntityList)
			{
				//remove body from the physics world and free the body data
				world->DestroyBody(e->body);
				//remove the entity from the appropriate entityList
				if (e->typeID == ENTITYSHOT)
				{
					for (int i = 0; i < shotEntityList.size(); ++i)
					{
						if (e == shotEntityList[i])
						{
							delete shotEntityList[i];
							shotEntityList.erase(shotEntityList.begin() + i);
							break;
						}
					}
				}
				else if (e->typeID == ENTITYBLOCK)
				{
					BlockEntity* b = (BlockEntity*) e;
					if (b->blockType != BlockType::PET)
					for (int i = 0; i < blockEntityList.size(); ++i)
					{
						if (e == blockEntityList[i])
						{
							delete blockEntityList[i];
							blockEntityList.erase(blockEntityList.begin() + i);
							break;
						}
					}
					else
					for (int i = 0; i < enemyList.size(); ++i)
					{
						if (e == enemyList[i])
						{
							delete enemyList[i];
							enemyList.erase(enemyList.begin() + i);
							break;
						}
					}
				}
				else
				{
					for (int i = 0; i < entityList.size(); ++i)
					{
						if (e == entityList[i])
						{
							delete entityList[i];
							entityList.erase(entityList.begin() + i);
							break;
						}
					}
				}
			}

			deadEntityList.clear();
			if (enemyList.size() == 0)
				gameState = WIN;

		}
	}
	break;

	case START:
		camera->StopShaking();
		break;

	case WIN:

		camera->Update((float)seconds);
		elapsedTime += seconds;
		while (elapsedTime >= timeStep)
		{
			//update the physics world
			world->Step(timeStep, velocityIterations, positionIterations);

			// Clear applied body forces. 
			world->ClearForces();
			if (scale1 < 2)
				scale1 += timeStep;
			else
			{
				scale1 = 2;
				if (scale2 < 2)
					scale2 += timeStep;
				else
					scale2 = 2;
			}
						//update game logic/animation
			for (auto e : entityList) e->Update(timeStep);
			for (auto b : shotEntityList) b->Update(timeStep);
			for (auto b : blockEntityList) b->Update(timeStep);
			for (auto b : enemyList) b->Update(timeStep);

			for (int i = particleList.size() - 1; i >= 0; --i)
			{
				if (particleList[i]->Update(timeStep))
				{
					//time to die!
					delete particleList[i];
					particleList.erase(particleList.begin() + i);
				}
			}
			elapsedTime -= timeStep;
		}

			


	
	default:
		//do nada here
		break;
	}//end switch(gameState)
}

void Draw(void)
{
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);	//clear colour: r,g,b,a 	
											// wipe the drawing surface clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//draw stuff here
	switch (gameState)
	{
	case START:
		for (int i = 0; i < ((1920 * 2) / 70) + 2; ++i)
		{
			groundSprite->Blit(i * 70.f - 35.f, 35.f);
		}

		//throw in a few cactii
		cactusSprite->Blit(500, 90);
		cactusSprite->Blit(1500, 90);
		cactusSprite->Blit(2000, 90);
		cactusSprite->Blit(3000, 90);
		//loop over all entities and draw them
		camera->UnDraw();

		StartSprite->Blit(blit3D->screenWidth / 2, blit3D->screenHeight / 2);
		Credits->Blit(blit3D->screenWidth / 2, (blit3D->screenHeight / 2) - 370);
		break;

	case SETTLING:
	
	case WIN:		

		camera->UnDraw();

		//Draw the level background
		for (int i = 0; i < ((1920 * 2) / 70) + 2; ++i)
		{
			groundSprite->Blit(i * 70.f - 35.f, 35.f);
		}

		//throw in a few cactii
		cactusSprite->Blit(500, 90);
		cactusSprite->Blit(1500, 90);
		cactusSprite->Blit(2000, 90);
		cactusSprite->Blit(3000, 90);
		//loop over all entities and draw them
		for (auto e : entityList) e->Draw();
		for (auto b : blockEntityList) b->Draw();
		for (auto b : shotEntityList) b->Draw();
		for (auto p : particleList) p->Draw();
		for (auto p : enemyList) p->Draw();
		cannon.Draw();
		meter.Draw();

		Win->Blit(blit3D->screenWidth / 2, blit3D->screenHeight / 2 + 100);
		Yay->Blit(blit3D->screenWidth / 2, blit3D->screenHeight / 2 - 300);
	
		break;

	case PLAYING:
		camera->Draw();

		//Draw the level background
		for (int i = 0; i < ((1920 * 2) / 70) + 2; ++i)
		{
			groundSprite->Blit(i * 70.f - 35.f, 35.f);
		}

		//throw in a few cactii
		cactusSprite->Blit(500, 90);
		cactusSprite->Blit(1500, 90);
		cactusSprite->Blit(2000, 90);
		cactusSprite->Blit(3000, 90);
		//loop over all entities and draw them
		for (auto e : entityList) e->Draw();
		for (auto b : blockEntityList) b->Draw();
		for (auto b : shotEntityList) b->Draw();
		for (auto p : particleList) p->Draw();
		for (auto p : enemyList) p->Draw();
		cannon.Draw();
		meter.Draw();
		break;

	}
}

//the key codes/actions/mods for DoInput are from GLFW: check its documentation for their values
void DoInput(int key, int scancode, int action, int mods)
{	

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			blit3D->Quit(); //start the shutdown sequence
	
	switch (gameState)
	{
	case START:
		if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
			gameState = PLAYING;
		break;
	case PLAYING:


		if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
			cannon.rotateDir = -1.f;

		if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
			cannon.rotateDir = 1.f;

		if ((key == GLFW_KEY_A || key == GLFW_KEY_D) && action == GLFW_RELEASE)
			cannon.rotateDir = 0;

		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		{
			meter.shooting = true;
		}

		if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
		{
			meter.shooting = false;
			fireShotNow = true;
		}

		if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			camera->Pan(1, 0);
			followingShot = false;
		}

		if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			camera->Pan(-1, 0);
			followingShot = false;
		}

		if ((key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) && action == GLFW_RELEASE)
			camera->Pan(0, 0);

		if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			camera->Pan(0, 1);
			followingShot = false;
		}

		if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			camera->Pan(0, -1);
			followingShot = false;
		}

		if ((key == GLFW_KEY_DOWN || key == GLFW_KEY_UP) && action == GLFW_RELEASE)
			camera->Pan(0, 0);
		break;
	case SETTLING:
		break;
	case WIN:
		break;
	default:
		break;
	}
	
}

int main(int argc, char *argv[])
{
	//memory leak detection
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	blit3D = new Blit3D(Blit3DWindowModel::BORDERLESSFULLSCREEN_1080P, 720, 480);

	//set our callback funcs
	blit3D->SetInit(Init);
	blit3D->SetDeInit(DeInit);
	blit3D->SetUpdate(Update);
	blit3D->SetDraw(Draw);
	blit3D->SetDoInput(DoInput);

	//Run() blocks until the window is closed
	blit3D->Run(Blit3DThreadModel::SINGLETHREADED);
	if (blit3D) delete blit3D;
}