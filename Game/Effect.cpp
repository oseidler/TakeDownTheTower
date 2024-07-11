#include "Game/Effect.hpp"
#include "Game/Enemy.hpp"
#include "Game/Player.hpp"


//
//constructor
//
Effect::Effect(EffectDefinition const* definition, int stack, Player* playerOwner, Enemy* enemyOwner)
	: m_definition(definition)
	, m_stack(stack)
	, m_playerOwner(playerOwner)
	, m_enemyOwner(enemyOwner)
{
}


bool Effect::operator==(Effect const& otherEffect) const
{
	return m_definition->m_name == otherEffect.m_definition->m_name;
}
