#pragma once

#include "Entity.h"

enum class PetType {SMALL_DOG, BEIGE_CAT, BROWN_CAT, BIG_DOG, YELLOW_CAT, GREY_CAT, ORANGE_CAT, PATCH_CAT};

class PetEntity : public Entity
{
public:
	PetType petType;
	int hp;
	int maxHP;
	std::vector<Sprite *> spriteList;

	PetEntity()
	{
		typeID = ENTITYALIEN;
		petType = PetType::BIG_DOG;
		maxHP = hp = 100;
	}

	//Damage() returns true if we should kill this object
	bool Damage(int damage)
	{
		hp -= damage;
		if (hp < 1) return true;
		else if (hp < maxHP / 2) sprite = spriteList[1];

		return false;
	}

	void Draw();
};

PetEntity * MakePet(PetType ptype, b2Vec2 pixelCoords, int maximumHP);