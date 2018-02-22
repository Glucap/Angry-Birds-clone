#include "BlockEntity.h"

extern b2World *world;
extern std::vector<Sprite *> blockSprites;

BlockEntity * MakeBlock(BlockType btype, MaterialType mtype, b2Vec2 pixelCoords,
	float angleInDegrees, int maximumHP)
{
	BlockEntity * blockEntity = new BlockEntity();
	blockEntity->blockType = btype;
	b2BodyDef bodyDef;
	
	bodyDef.type = b2_dynamicBody; //make it a dynamic body i.e. one moved by physics
	bodyDef.position = Pixels2Physics(pixelCoords); //set its position in the world
	bodyDef.angle = deg2rad(angleInDegrees);

	bodyDef.angularDamping = 1.8f;

	blockEntity->body = world->CreateBody(&bodyDef); //create the body and add it to the world
	int32 count;
	b2FixtureDef fixtureDef;
	
	b2Vec2 vertices3[3];
	b2Vec2 vertices4[4];

	float cx;
	float cy;
	
	// Define a shape for our body.
	b2PolygonShape polygon;
	b2CircleShape circle;

	switch (btype)
	{
	case BlockType::LARGE_TRIANGLE:
	{
		// This defines a triangle in CCW order.
		
		
		cx = 581 + 138 / 2;
		cy = 1841 + 68 / 2;

		vertices3[0].Set((650.f-cx) / PTM_RATIO, -(1841.0f - cy) / PTM_RATIO);
		vertices3[1].Set((581.0f-cx)/PTM_RATIO, -(1909.0f - cy)/PTM_RATIO);
		vertices3[2].Set((719.0f-cx)/PTM_RATIO, -(1909.0f-cy)/PTM_RATIO);

		count = 3;		

		polygon.Set(vertices3, count);

		fixtureDef.shape = &polygon;
	}
		break;
	case BlockType::BALL:
		circle.m_radius = 68.f / (2 * PTM_RATIO);

		fixtureDef.shape = &circle;
		break;

	case BlockType::BIG_SQUARE:

		cx = 861 + 138 / 2;
		cy = 1511 + 138 / 2;

		//made all the rectangular shapes with vertices for practice

		vertices4[0].Set((861.f - cx) / PTM_RATIO, -(1511.0f - cy) / PTM_RATIO);
		vertices4[1].Set((861.0f - cx) / PTM_RATIO, -(1649.0f - cy) / PTM_RATIO);
		vertices4[2].Set((999.0f - cx) / PTM_RATIO, -(1648.0f - cy) / PTM_RATIO);
		vertices4[3].Set((999.0f - cx) / PTM_RATIO, -(1511.0f - cy) / PTM_RATIO);

		count = 4;

		polygon.Set(vertices4, count);

		fixtureDef.shape = &polygon;

		break;
		
	case BlockType::LONG_BAR:

		cx = 1841 + 68 / 2;
		cy = 711 + 218 / 2;

		vertices4[0].Set((1841.f - cx) / PTM_RATIO, -(711.0f - cy) / PTM_RATIO);
		vertices4[1].Set((1841.0f - cx) / PTM_RATIO, -(929.0f - cy) / PTM_RATIO);
		vertices4[2].Set((1909.0f - cx) / PTM_RATIO, -(929.0f - cy) / PTM_RATIO);
		vertices4[3].Set((1909.0f - cx) / PTM_RATIO, -(711.0f - cy) / PTM_RATIO);

		count = 4;

		polygon.Set(vertices4, count);

		fixtureDef.shape = &polygon;

		break;
	case BlockType::RECTANGLE:

		cx = 1 + 218 / 2;
		cy = 1331 + 138 / 2;

		vertices4[0].Set((1.f - cx) / PTM_RATIO, -(1331.0f - cy) / PTM_RATIO);
		vertices4[1].Set((1.0f - cx) / PTM_RATIO, -(1469.0f - cy) / PTM_RATIO);
		vertices4[2].Set((219.0f - cx) / PTM_RATIO, -(1469.0f - cy) / PTM_RATIO);
		vertices4[3].Set((219.0f - cx) / PTM_RATIO, -(1331.0f - cy) / PTM_RATIO);

		count = 4;

		polygon.Set(vertices4, count);

		fixtureDef.shape = &polygon;

		break;

	case BlockType::SHORT_BAR:

		cx = 941 + 138 / 2;
		cy = 1 + 68 / 2;

		vertices4[0].Set((941.f - cx) / PTM_RATIO, -(1.0f - cy) / PTM_RATIO);
		vertices4[1].Set((941.0f - cx) / PTM_RATIO, -(69.0f - cy) / PTM_RATIO);
		vertices4[2].Set((1079.0f - cx) / PTM_RATIO, -(69.0f - cy) / PTM_RATIO);
		vertices4[3].Set((1079.0f - cx) / PTM_RATIO, -(1.0f - cy) / PTM_RATIO);

		count = 4;

		polygon.Set(vertices4, count);

		fixtureDef.shape = &polygon;

		break;

	case BlockType::SMALL_SQUARE:

		cx = 1771 + 68 / 2;
		cy = 1571 + 68 / 2;

		vertices4[0].Set((1771.f - cx) / PTM_RATIO, -(1571.0f - cy) / PTM_RATIO);
		vertices4[1].Set((1771.0f - cx) / PTM_RATIO, -(1639.0f - cy) / PTM_RATIO);
		vertices4[2].Set((1839.0f - cx) / PTM_RATIO, -(1639.0f - cy) / PTM_RATIO);
		vertices4[3].Set((1839.0f - cx) / PTM_RATIO, -(1571.0f - cy) / PTM_RATIO);

		count = 4;

		polygon.Set(vertices4, count);

		fixtureDef.shape = &polygon;

		break;

	case BlockType::SMALL_TRIANGLE:

		cx = 1701 + 68 / 2;
		cy = 1411 + 68 / 2;

		vertices3[0].Set((1701.f - cx) / PTM_RATIO, -(1411.0f - cy) / PTM_RATIO);
		vertices3[1].Set((1701.0f - cx) / PTM_RATIO, -(1479.0f - cy) / PTM_RATIO);
		vertices3[2].Set((1769.0f - cx) / PTM_RATIO, -(1411.0f - cy) / PTM_RATIO);

		count = 3;

		polygon.Set(vertices3, count);

		fixtureDef.shape = &polygon;

		break;

	case BlockType::PET:
		circle.m_radius = 74.f / (2 * PTM_RATIO);

		fixtureDef.shape = &circle;
		break;
	

	}//end switch(btype)

	switch (mtype)
	{
	case MaterialType::WOOD:
		fixtureDef.density = 1.f;
		fixtureDef.restitution = 0.05;
		fixtureDef.friction = 0.8;
		break;

	case MaterialType::GLASS:
		fixtureDef.density = .5f;
		fixtureDef.restitution = 0.01;
		fixtureDef.friction = 0.8;
		break;

	case MaterialType::METAL:
		fixtureDef.density = 1.5f;
		fixtureDef.restitution = 0.3;
		fixtureDef.friction = 0.8;
		break;

	case MaterialType::ROCK:
		fixtureDef.density = 2.f;
		fixtureDef.restitution = 0.1;
		fixtureDef.friction = 0.8;
		break;



	}//end switch(mtype)

	blockEntity->body->CreateFixture(&fixtureDef);

	blockEntity->body->SetUserData((void *)blockEntity);

	//add sprites
	blockEntity->sprite = blockSprites[(int)btype * 12 + (int)mtype *3];
	blockEntity->spriteList.push_back(blockSprites[(int)btype * 12 + (int)mtype * 3 + 1]);
	blockEntity->spriteList.push_back(blockSprites[(int)btype * 12 + (int)mtype * 3 + 2]);

	blockEntity->maxHP = blockEntity->hp = maximumHP;

	return blockEntity;	
}