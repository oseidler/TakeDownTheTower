#pragma once
#include "Game/EnemyDefinition.hpp"
#include "Game/Effect.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"


//forward declarations
class Encounter;
class EffectDefinition;


class Enemy
{
//public member functions
public:
	//constructor
	explicit Enemy(EnemyDefinition const* definition, Encounter* encounter, AABB2 renderBounds);

	//game flow functions
	void Update();
	void Render() const;

	//enemy utilities
	void PerformCurrentIntention();
	void ChooseNextIntention();
	void TakeDamage(int damageAmount);
	void GainBlock(int blockAmount);
	void ReceiveEffect(EffectDefinition const* definition, int stack);

//public member variables
public:
	//enemy parameters
	int m_currentHealth = 0;
	int m_currentBlock = 0;
	Intention const* m_currentIntention = nullptr;

	EnemyDefinition const* m_definition = nullptr;
	Encounter* m_encounter = nullptr;

	AABB2 m_renderBounds = AABB2();
	Rgba8 m_renderColor = Rgba8();

	std::vector<Effect> m_effects;
};
