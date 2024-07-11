#pragma once
#include "Engine/Core/EngineCommon.hpp"


//forward declarations
class Texture;
class CardDefinition;
class EffectDefinition;


//enums
enum class IntentionMode
{
	LOOP_ALL,
	LOOP_LAST,
	RANDOM,
	INVALID
};


//struct for enemy intentions
struct Intention
{
	int m_damage = 0;
	int m_block = 0;
	CardDefinition const* m_cardToAdd = nullptr;
	EffectDefinition const* m_inflictEffect = nullptr;
	int m_inflictEffectStack = 0;
	EffectDefinition const* m_gainEffect = nullptr;
	int m_gainEffectStack = 0;
	bool m_preparing = true;
};


class EnemyDefinition
{
//public member functions
public:
	//constructor
	explicit EnemyDefinition(XmlElement const& element);

	//static functions
	static void InitializeEnemyDefs();
	static EnemyDefinition const* GetEnemyDefinition(std::string name);

//public member variables
public:
	//enemy parameters
	std::string			   m_name = "invalid enemy";
	Texture*			   m_sprite = nullptr;
	int					   m_maxHealth = 0;
	IntentionMode		   m_intentionMode = IntentionMode::INVALID;
	std::vector<Intention> m_intentions;
	
	//static variables
	static std::vector<EnemyDefinition> s_enemyDefs;
};
