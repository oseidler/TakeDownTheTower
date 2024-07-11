#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Texture.hpp"


//forward declarations
class Texture;
class EffectDefinition;


//constants
constexpr int NUM_STARTER_CARDS = 2;
constexpr int NUM_STATUS_CARDS = 2;


//enums
enum class CardType
{
	ATTACK,
	SKILL,
	POWER,
	STATUS,
	INVALID
};

enum class TargetMode
{
	ONE,
	ALL,
	SELF,
	NONE
};

enum class CardRarity
{
	STARTER,
	COMMON,
	UNCOMMON,
	RARE,
	INVALID
};

enum class AttackType
{
	SLICE,
	PIERCE,
	LIGHT_IMPACT,
	HEAVY_IMPACT,
	FIRE,
	MAGIC,
	NONE
};


class CardDefinition
{
//public member functions
public:
	//constructor
	explicit CardDefinition(XmlElement const& element);

	//static functions
	static void InitializeCardDefs();
	static CardDefinition const* GetCardDefinition(std::string name);

//public member variables
public:
	//card parameters
	uint8_t		m_id = 0;
	std::string m_name = "invalid card";
	Texture*	m_sprite = nullptr;
	CardType	m_type = CardType::INVALID;
	CardRarity	m_rarity = CardRarity::INVALID;
	TargetMode	m_targetMode = TargetMode::NONE;
	AttackType  m_attackType = AttackType::NONE;
	int			m_cost = 0;
	int			m_damage = 0;
	int			m_numHits = 0;
	int			m_block = 0;
	int			m_restoreHP = 0;
	int			m_cardsDrawn = 0;
	int			m_energyGain = 0;

	EffectDefinition const* m_inflictEffect = nullptr;
	int						m_inflictEffectStack = 0;
	EffectDefinition const* m_gainEffect = nullptr;
	int						m_gainEffectStack = 0;

	bool		m_exhaust = false;
	bool		m_isPlayable = true;

	std::string m_description = "invalid description";

	//static variables
	static std::vector<CardDefinition> s_cardDefs;
};
