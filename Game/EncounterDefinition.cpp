#include "Game/EncounterDefinition.hpp"
#include "Game/EnemyDefinition.hpp"


//static variable declaration
std::vector<EncounterDefinition> EncounterDefinition::s_encounterDefs;


//
//constructor
//
EncounterDefinition::EncounterDefinition(XmlElement const& element)
{
	m_difficultyLevel = ParseXmlAttribute(element, "difficulty", m_difficultyLevel);

	XmlElement const* enemiesRootElement = element.FirstChildElement();
	GUARANTEE_OR_DIE(enemiesRootElement != nullptr, "Failed to read enemies root element!");

	XmlElement const* enemyElement = enemiesRootElement->FirstChildElement();
	while (enemyElement != nullptr)
	{
		std::string elementName = enemyElement->Name();
		GUARANTEE_OR_DIE(elementName == "Enemy", "Enemy elements in encounter definitions xml must be <Enemy>!");

		std::string enemyName = ParseXmlAttribute(*enemyElement, "name", "invalid enemy name");
		EnemyDefinition const* enemyDef = EnemyDefinition::GetEnemyDefinition(enemyName);
		m_enemies.emplace_back(enemyDef);

		AABB2 enemyBounds = ParseXmlAttribute(*enemyElement, "renderBounds", AABB2(1100.0f, 325.0f, 1300.0f, 525.0f));
		m_enemyBounds.emplace_back(enemyBounds);

		enemyElement = enemyElement->NextSiblingElement();
	}
}


//
//static functions
//
void EncounterDefinition::InitializeEncounterDefs()
{
	XmlDocument encounterDefsXml;
	char const* filePath = "Data/Definitions/EncounterDefinitions.xml";
	XmlError result = encounterDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "Failed to open encounter definitions xml file!");

	XmlElement* rootElement = encounterDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement != nullptr, "Failed to read encounter definitions root element!");

	XmlElement* encounterDefElement = rootElement->FirstChildElement();
	uint8_t currentEncounterID = 0;
	while (encounterDefElement != nullptr)
	{
		std::string elementName = encounterDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "EncounterDefinition", "Child element names in encounter definitions xml file must be <EncounterDefinition>!");
		EncounterDefinition newEncounterDef = EncounterDefinition(*encounterDefElement);
		newEncounterDef.m_id = currentEncounterID;
		s_encounterDefs.emplace_back(newEncounterDef);
		currentEncounterID++;
		encounterDefElement = encounterDefElement->NextSiblingElement();
	}
}


EncounterDefinition const* EncounterDefinition::GetEncounterDefinition(int encounterID)
{
	if (encounterID <= s_encounterDefs.size())
	{
		return &s_encounterDefs[encounterID];
	}

	//return null if it wasn't found
	return nullptr;
}
