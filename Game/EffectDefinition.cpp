#include "Game/EffectDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"


//static variable declaration
std::vector<EffectDefinition> EffectDefinition::s_effectDefs;


//
//constructor
//
EffectDefinition::EffectDefinition(XmlElement const& element)
{
	m_name = ParseXmlAttribute(element, "name", m_name);

	std::string textureFilePath = ParseXmlAttribute(element, "sprite", "invalid file path");
	m_sprite = g_theRenderer->CreateOrGetTextureFromFile(textureFilePath.c_str());

	std::string typeString = ParseXmlAttribute(element, "type", "Invalid");
	if (typeString == "Buff")
	{
		m_type = EffectType::BUFF;
	}
	else if (typeString == "Debuff")
	{
		m_type = EffectType::DEBUFF;
	}
	else
	{
		m_type = EffectType::INVALID;
	}

	std::string stackTypeString = ParseXmlAttribute(element, "stackType", "None");
	if (stackTypeString == "Duration")
	{
		m_stackType = StackType::DURATION;
	}
	else if (stackTypeString == "Intensity")
	{
		m_stackType = StackType::INTENSITY;
	}
	else if (stackTypeString == "Counter")
	{
		m_stackType = StackType::COUNTER;
	}
	else
	{
		m_stackType = StackType::NONE;
	}

	m_modDealtDamage = ParseXmlAttribute(element, "modDealtDamage", m_modDealtDamage);
	m_modReceivedDamage = ParseXmlAttribute(element, "modReceivedDamage", m_modReceivedDamage);
	m_modBlock = ParseXmlAttribute(element, "modBlock", m_modBlock);

	m_usePercentage = ParseXmlAttribute(element, "usePercentage", m_usePercentage);
	m_percentModifier = ParseXmlAttribute(element, "percentModifier", m_percentModifier);

	m_blockDebuff = ParseXmlAttribute(element, "blockDebuff", m_blockDebuff);
}


//
//static functions
//
void EffectDefinition::InitializeEffectDefs()
{
	XmlDocument effectDefsXml;
	char const* filePath = "Data/Definitions/EffectDefinitions.xml";
	XmlError result = effectDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "Failed to open effect definitions xml file!");

	XmlElement* rootElement = effectDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement != nullptr, "Failed to read effect definitions root element!");

	XmlElement* effectDefElement = rootElement->FirstChildElement();
	while (effectDefElement != nullptr)
	{
		std::string elementName = effectDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "EffectDefinition", "Child element names in effect definitions xml file must be <EffectDefinition>!");
		EffectDefinition newEffectDef = EffectDefinition(*effectDefElement);
		s_effectDefs.emplace_back(newEffectDef);
		effectDefElement = effectDefElement->NextSiblingElement();
	}
}


EffectDefinition const* EffectDefinition::GetEffectDefinition(std::string name)
{
	for (int defIndex = 0; defIndex < s_effectDefs.size(); defIndex++)
	{
		if (s_effectDefs[defIndex].m_name == name)
		{
			return &s_effectDefs[defIndex];
		}
	}

	//return null if it wasn't found
	return nullptr;
}
