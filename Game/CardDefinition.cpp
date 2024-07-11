#include "Game/CardDefinition.hpp"
#include "Game/EffectDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/StringUtils.hpp"


//static variable declaration
std::vector<CardDefinition> CardDefinition::s_cardDefs;


//
//constructor
//
CardDefinition::CardDefinition(XmlElement const& element)
{
	m_name = ParseXmlAttribute(element, "name", m_name);
	m_description = ParseXmlAttribute(element, "description", m_description);
	ReplacePartOfString(m_description, "\\n", "\n");	//this has to be done because tinyxml reads in \n incorrectly
	
	std::string textureFilePath = ParseXmlAttribute(element, "sprite", "invalid file path");
	m_sprite = g_theRenderer->CreateOrGetTextureFromFile(textureFilePath.c_str());

	std::string typeString = ParseXmlAttribute(element, "type", "Invalid");
	if (typeString == "Attack")
	{
		m_type = CardType::ATTACK;
	}
	else if (typeString == "Skill")
	{
		m_type = CardType::SKILL;
	}
	else if (typeString == "Power")
	{
		m_type = CardType::POWER;
	}
	else if (typeString == "Status")
	{
		m_type = CardType::STATUS;
	}
	else
	{
		m_type = CardType::INVALID;
	}

	std::string rarityString = ParseXmlAttribute(element, "rarity", "Invalid");
	if (rarityString == "Starter")
	{
		m_rarity = CardRarity::STARTER;
	}
	else if (rarityString == "Common")
	{
		m_rarity = CardRarity::COMMON;
	}
	else if (rarityString == "Uncommon")
	{
		m_rarity = CardRarity::UNCOMMON;
	}
	else if (rarityString == "Rare")
	{
		m_rarity = CardRarity::RARE;
	}
	else
	{
		m_rarity = CardRarity::INVALID;
	}
	
	std::string targetModeString = ParseXmlAttribute(element, "targetMode", "None");
	if (targetModeString == "One")
	{
		m_targetMode = TargetMode::ONE;
	}
	else if (targetModeString == "All")
	{
		m_targetMode = TargetMode::ALL;
	}
	else if (targetModeString == "Self")
	{
		m_targetMode = TargetMode::SELF;
	}
	else
	{
		m_targetMode = TargetMode::NONE;
	}

	std::string attackTypeString = ParseXmlAttribute(element, "attackType", "None");
	if (attackTypeString == "Slice")
	{
		m_attackType = AttackType::SLICE;
	}
	else if (attackTypeString == "Pierce")
	{
		m_attackType = AttackType::PIERCE;
	}
	else if (attackTypeString == "LightImpact")
	{
		m_attackType = AttackType::LIGHT_IMPACT;
	}
	else if (attackTypeString == "HeavyImpact")
	{
		m_attackType = AttackType::HEAVY_IMPACT;
	}
	else if (attackTypeString == "Fire")
	{
		m_attackType = AttackType::FIRE;
	}
	else if (attackTypeString == "Magic")
	{
		m_attackType = AttackType::MAGIC;
	}
	else
	{
		m_attackType = AttackType::NONE;
	}
	
	m_cost = ParseXmlAttribute(element, "cost", m_cost);
	m_damage = ParseXmlAttribute(element, "damage", m_damage);
	m_numHits = ParseXmlAttribute(element, "numHits", m_numHits);
	m_block = ParseXmlAttribute(element, "block", m_block);
	m_restoreHP = ParseXmlAttribute(element, "restoreHP", m_restoreHP);
	m_cardsDrawn = ParseXmlAttribute(element, "cardsDrawn", m_cardsDrawn);
	m_energyGain = ParseXmlAttribute(element, "energyGain", m_energyGain);

	std::string inflictEffect = ParseXmlAttribute(element, "inflictEffect", "invalid effect");
	m_inflictEffect = EffectDefinition::GetEffectDefinition(inflictEffect);
	m_inflictEffectStack = ParseXmlAttribute(element, "inflictEffectStack", 0);

	std::string gainEffect = ParseXmlAttribute(element, "gainEffect", "invalid effect");
	m_gainEffect = EffectDefinition::GetEffectDefinition(gainEffect);
	m_gainEffectStack = ParseXmlAttribute(element, "gainEffectStack", 0);

	m_exhaust = ParseXmlAttribute(element, "exhaust", m_exhaust);
	m_isPlayable = ParseXmlAttribute(element, "isPlayable", m_isPlayable);
}


//
//static functions
//
void CardDefinition::InitializeCardDefs()
{
	XmlDocument cardDefsXml;
	char const* filePath = "Data/Definitions/CardDefinitions.xml";
	XmlError result = cardDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "Failed to open card definitions xml file!");

	XmlElement* rootElement = cardDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement != nullptr, "Failed to read card definitions root element!");

	XmlElement* cardDefElement = rootElement->FirstChildElement();
	uint8_t currentCardID = 0;
	while (cardDefElement != nullptr)
	{
		std::string elementName = cardDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "CardDefinition", "Child element names in card definitions xml file must be <CardDefinition>!");
		CardDefinition newCardDef = CardDefinition(*cardDefElement);
		newCardDef.m_id = currentCardID;
		s_cardDefs.emplace_back(newCardDef);
		currentCardID++;
		cardDefElement = cardDefElement->NextSiblingElement();
	}
}


CardDefinition const* CardDefinition::GetCardDefinition(std::string name)
{
	for (int defIndex = 0; defIndex < s_cardDefs.size(); defIndex++)
	{
		if (s_cardDefs[defIndex].m_name == name)
		{
			return &s_cardDefs[defIndex];
		}
	}

	//return null if it wasn't found
	return nullptr;
}
