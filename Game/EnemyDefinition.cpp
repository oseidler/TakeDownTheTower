#include "Game/EnemyDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Game/CardDefinition.hpp"
#include "Game/EffectDefinition.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//static variable declaration
std::vector<EnemyDefinition> EnemyDefinition::s_enemyDefs;


//
//constructor
//
EnemyDefinition::EnemyDefinition(XmlElement const& element)
{
	m_name = ParseXmlAttribute(element, "name", m_name);

	std::string textureFilePath = ParseXmlAttribute(element, "sprite", "invalid file path");
	m_sprite = g_theRenderer->CreateOrGetTextureFromFile(textureFilePath.c_str());

	m_maxHealth = ParseXmlAttribute(element, "maxHealth", m_maxHealth);
	
	std::string intentionModeString = ParseXmlAttribute(element, "intentionMode", "Invalid");
	if (intentionModeString == "LoopAll")
	{
		m_intentionMode = IntentionMode::LOOP_ALL;
	}
	else if (intentionModeString == "LoopLast")
	{
		m_intentionMode = IntentionMode::LOOP_LAST;
	}
	else if (intentionModeString == "Random")
	{
		m_intentionMode = IntentionMode::RANDOM;
	}
	else
	{
		m_intentionMode = IntentionMode::INVALID;
	}

	XmlElement const* intentionsRootElement = element.FirstChildElement();
	GUARANTEE_OR_DIE(intentionsRootElement != nullptr, "Failed to read intentions root element!");

	XmlElement const* intentionElement = intentionsRootElement->FirstChildElement();
	while (intentionElement != nullptr)
	{
		std::string elementName = intentionElement->Name();
		GUARANTEE_OR_DIE(elementName == "Intention", "Intention elements in enemy definitions xml must be <Intention>!");

		Intention intention;
		intention.m_damage = ParseXmlAttribute(*intentionElement, "damage", 0);
		intention.m_block = ParseXmlAttribute(*intentionElement, "block", 0);

		std::string cardToAdd = ParseXmlAttribute(*intentionElement, "cardToAdd", "invalid card");
		intention.m_cardToAdd = CardDefinition::GetCardDefinition(cardToAdd);

		std::string inflictEffect = ParseXmlAttribute(*intentionElement, "inflictEffect", "invalid effect");
		intention.m_inflictEffect = EffectDefinition::GetEffectDefinition(inflictEffect);
		intention.m_inflictEffectStack = ParseXmlAttribute(*intentionElement, "inflictEffectStack", 0);

		std::string gainEffect = ParseXmlAttribute(*intentionElement, "gainEffect", "invalid effect");
		intention.m_gainEffect = EffectDefinition::GetEffectDefinition(gainEffect);
		intention.m_gainEffectStack = ParseXmlAttribute(*intentionElement, "gainEffectStack", 0);
		
		intention.m_preparing = ParseXmlAttribute(*intentionElement, "preparing", false);

		m_intentions.emplace_back(intention);

		intentionElement = intentionElement->NextSiblingElement();
	}
}


//
//static functions
//
void EnemyDefinition::InitializeEnemyDefs()
{
	XmlDocument enemyDefsXml;
	char const* filePath = "Data/Definitions/EnemyDefinitions.xml";
	XmlError result = enemyDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "Failed to open enemy definitions xml file!");

	XmlElement* rootElement = enemyDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement != nullptr, "Failed to read enemy definitions root element!");

	XmlElement* enemyDefElement = rootElement->FirstChildElement();
	while (enemyDefElement != nullptr)
	{
		std::string elementName = enemyDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "EnemyDefinition", "Child element names in enemy definitions xml file must be <EnemyDefinition>!");
		EnemyDefinition newEnemyDef = EnemyDefinition(*enemyDefElement);
		s_enemyDefs.emplace_back(newEnemyDef);
		enemyDefElement = enemyDefElement->NextSiblingElement();
	}
}


EnemyDefinition const* EnemyDefinition::GetEnemyDefinition(std::string name)
{
	for (int defIndex = 0; defIndex < s_enemyDefs.size(); defIndex++)
	{
		if (s_enemyDefs[defIndex].m_name == name)
		{
			return &s_enemyDefs[defIndex];
		}
	}

	//return null if it wasn't found
	return nullptr;
}
