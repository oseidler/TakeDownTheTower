#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Texture.hpp"


//enum for the different possible things that the effect counter can represent
enum class EffectType
{
	BUFF,
	DEBUFF,
	INVALID
};

enum class StackType
{
	DURATION,
	INTENSITY,
	COUNTER, 
	NONE
};


class EffectDefinition
{
//public member functions
public:
	//constructor
	explicit EffectDefinition(XmlElement const& element);

	//static functions
	static void InitializeEffectDefs();
	static EffectDefinition const* GetEffectDefinition(std::string name);

//public member variables
public:
	//effect parameters
	std::string m_name = "null effect";
	Texture* m_sprite = nullptr;
	EffectType m_type = EffectType::INVALID;
	StackType m_stackType = StackType::NONE;

	bool m_modDealtDamage = false;
	bool m_modReceivedDamage = false;
	bool m_modBlock = false;

	bool m_usePercentage = false;
	float m_percentModifier = 1.0f;

	bool m_blockDebuff = false;
	
	//static variables
	static std::vector<EffectDefinition> s_effectDefs;
};
