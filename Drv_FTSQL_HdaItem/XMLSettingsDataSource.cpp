#include"pch.h"
#include"XMLSettingsDataSource.h"
#include<pugixml.hpp>

bool DrvFTSQLHdaItem::XMLSettingsDataSource::Save(const ConnectionAttributes& attributes, std::ostream& stream)
{
	pugi::xml_document doc;
	pugi::xml_node rootNode = doc.append_child("Settings");
	pugi::xml_node connectionNode = rootNode.append_child("Connection");
	connectionNode.append_attribute("Version").set_value(attributes.driver.c_str());
	connectionNode.append_attribute("Server").set_value(attributes.serverName.c_str());
	connectionNode.append_attribute("AuthenticationType").set_value(attributes.isServerAuthentication);
	connectionNode.append_attribute("ConfigDataBase").set_value(attributes.databaseName.c_str());
	connectionNode.append_attribute("User").set_value(attributes.loginName.c_str());
	std::string encryptPass = attributes.password;
	connectionNode.append_attribute("Password").set_value(encryptPass.c_str());
	doc.save(stream);
	return true;
}

bool DrvFTSQLHdaItem::XMLSettingsDataSource::Load(ConnectionAttributes& attributes, std::istream& stream)
{
	pugi::xml_document doc;
	pugi::xml_parse_result res = doc.load(stream);
	if (!res) {
		return false;
	}
	pugi::xml_node rootNode = doc.child("Settings");
	pugi::xml_node connectionNode = rootNode.child("Connection");
	attributes.driver = std::string(connectionNode.attribute("Version").as_string());
	attributes.serverName = std::string(connectionNode.attribute("Server").as_string());
	attributes.isServerAuthentication = connectionNode.attribute("AuthenticationType").as_bool();
	attributes.databaseName = std::string(connectionNode.attribute("ConfigDataBase").as_string());
	attributes.loginName = std::string(connectionNode.attribute("User").as_string());
	std::string pass = std::string(connectionNode.attribute("Password").as_string());
	attributes.password = pass;
	return true;
}

bool DrvFTSQLHdaItem::XMLSettingsDataSource::LoadAttributesString(const char* source, size_t length, ConnectionAttributes& attributes)
{
	pugi::xml_document doc;
	pugi::xml_parse_result res = doc.load_buffer(source, length);
	if (!res) {
		return false;
	}
	pugi::xml_node rootNode = doc.child("Settings");
	pugi::xml_node connectionNode = rootNode.child("Connection");
	attributes.driver = std::string(connectionNode.attribute("Version").as_string());
	attributes.serverName = std::string(connectionNode.attribute("Server").as_string());
	attributes.isServerAuthentication = connectionNode.attribute("AuthenticationType").as_bool();
	attributes.databaseName = std::string(connectionNode.attribute("ConfigDataBase").as_string());
	attributes.loginName = std::string(connectionNode.attribute("User").as_string());
	std::string pass = std::string(connectionNode.attribute("Password").as_string());
	attributes.password = pass;
	return true;
}

bool DrvFTSQLHdaItem::XMLSettingsDataSource::SaveAttributesString(const ConnectionAttributes& attributes, std::ostream& stream)
{
	pugi::xml_document doc;
	pugi::xml_node rootNode = doc.append_child("Settings");
	pugi::xml_node connectionNode = rootNode.append_child("Connection");
	connectionNode.append_attribute("Version").set_value(attributes.driver.c_str());
	connectionNode.append_attribute("Server").set_value(attributes.serverName.c_str());
	connectionNode.append_attribute("AuthenticationType").set_value(attributes.isServerAuthentication);
	connectionNode.append_attribute("ConfigDataBase").set_value(attributes.databaseName.c_str());
	connectionNode.append_attribute("User").set_value(attributes.loginName.c_str());
	std::string encryptPass = attributes.password;
	connectionNode.append_attribute("Password").set_value(encryptPass.c_str());
	doc.save(stream);
	return true;
}