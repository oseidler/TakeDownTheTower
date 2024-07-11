#pragma once
#include "Game/EffectDefinition.hpp"


//forward declarations
class Player;
class Enemy;


class Effect
{
//public member functions
public:
	//constructor
	explicit Effect(EffectDefinition const* definition, int stack, Player* playerOwner, Enemy* enemyOwner);

	//operators
	bool operator==(Effect const& otherEffect) const;

//public member variables
public:
	EffectDefinition const* m_definition = nullptr;

	//only one of these should ever be set to not nullptr (in hindsight, I should've made the player and enemies subclasses of the same class, but I don't want to refactor them to do that)
	Player* m_playerOwner = nullptr;
	Enemy* m_enemyOwner = nullptr;

	int m_stack = 0;

	bool m_justAdded = true;
};
