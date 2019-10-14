#include"pch.h"
#include"SQLServerDatabaseEngine.h"

bool DrvFTSQLHdaItem::SQLServerDatabaseEngine::OpenConnection()
{
	if (!m_ptrEnvironment) {
		m_ptrEnvironment = std::make_shared<SQLServerEnvironment>();
	}

	m_ptrConnection = std::make_shared<SQLServerConnection>(m_ptrEnvironment);
	bool isOpen = m_ptrConnection->IsValidConnection();
	return isOpen;
}

bool DrvFTSQLHdaItem::SQLServerDatabaseEngine::CreateDirectConnectionToDatabase(const ConnectionAttributes& attributes)
{
	if (!m_ptrEnvironment) {
		m_ptrEnvironment = std::make_shared<SQLServerEnvironment>();
	}
	m_ptrConnection = SQLServerConnection::CreateConnectionToDatabase(m_ptrEnvironment, attributes);
	if (m_ptrConnection) {
		return true;
	}
	else {
		return false;
	}
}

bool DrvFTSQLHdaItem::SQLServerDatabaseEngine::OpenConnection(const ConnectionAttributes& attributes)
{
	if (!m_ptrEnvironment) {
		m_ptrEnvironment = std::make_shared<SQLServerEnvironment>();
	}

	m_ptrConnection = std::make_shared<SQLServerConnection>(m_ptrEnvironment, attributes);
	if (m_ptrConnection->IsValidConnection() == false) {
		return false;
	}
	ConnectionAttributes attr = m_ptrConnection->GetConnectionAttributes();
	if (attr != attributes) {
		return false;
	}
	return true;
}

bool DrvFTSQLHdaItem::SQLServerDatabaseEngine::loadServerInstances(std::string driverName)
{
	bool isConnect = m_ptrConnection->ConnectToServerInstances(driverName);
	return isConnect;
}

bool DrvFTSQLHdaItem::SQLServerDatabaseEngine::loadDatabaseInstances(std::string serverName, AuthenticationType type, std::string user, std::string password)
{
	bool isConnect = m_ptrConnection->ConnectToDatabaseInstances(serverName, user, password, type);
	return isConnect;
}

bool DrvFTSQLHdaItem::SQLServerDatabaseEngine::ChooseDatabase(std::string databaseName)
{
	if (m_ptrConnection->ConnectToDatabase(databaseName)) {
		return true;
	}
	return false;
}

void DrvFTSQLHdaItem::SQLServerDatabaseEngine::CloseConnection()
{
	m_ptrConnection.reset();
}

bool DrvFTSQLHdaItem::SQLServerDatabaseEngine::IsValidConnection() const
{
	if (m_ptrConnection) {
		return m_ptrConnection->IsValidConnection();
	}
	else {
		return false;
	}
}

DrvFTSQLHdaItem::ConnectionAttributes DrvFTSQLHdaItem::SQLServerDatabaseEngine::GetConnectionAttributes() const
{
	if (m_ptrConnection) {
		return m_ptrConnection->GetConnectionAttributes();
	}
	else {
		return ConnectionAttributes();
	}
}

DrvFTSQLHdaItem::ConnectionStatus DrvFTSQLHdaItem::SQLServerDatabaseEngine::GetConnectionStatus() const {
	if (m_ptrConnection) {
		return m_ptrConnection->GetConnectionStatus();
	}
	else {
		return ConnectionStatus::NoConnect;
	}
}

std::vector<std::string> DrvFTSQLHdaItem::SQLServerDatabaseEngine::GetServersList() const
{
	return m_ptrConnection->GetServerList();
}
std::vector<std::string> DrvFTSQLHdaItem::SQLServerDatabaseEngine::GetDatabasesList() const
{
	return m_ptrConnection->GetDatabaseList();
}


bool DrvFTSQLHdaItem::SQLServerDatabaseEngine::OpenConnectionIfNeeded(const ConnectionAttributes& attributes)
{
	bool isValid = false;
	if (m_ptrConnection) {
		ConnectionAttributes attr = m_ptrConnection->GetConnectionAttributes();
		ConnectionStatus status = m_ptrConnection->GetConnectionStatus();
		switch (status)
		{
		case ConnectionStatus::ConnectToDatabase:
			if (attributes == attr) {
				return true;
			}
			else {
				CloseConnection();
				return OpenConnection(attributes);
			}
			break;
		case ConnectionStatus::ConnectToServer:
			if (attributes.loginName != attr.loginName || attributes.password != attr.password || attributes.isServerAuthentication != attr.isServerAuthentication || attributes.serverName != attr.serverName) {
				CloseConnection();
				return OpenConnection(attributes);
			}
			else {
				if (!attributes.databaseName.empty()) {
					return ChooseDatabase(attributes.databaseName);
				}
				else {
					return true;
				}
			}

			break;
		case ConnectionStatus::ConnectToDriver:
			if (attributes.driver == attr.driver) {
				if (!attributes.serverName.empty()) {
					AuthenticationType authtype = AuthenticationType::System;
					if (attributes.isServerAuthentication) {
						authtype = AuthenticationType::Server;
					}
					isValid = loadDatabaseInstances(attributes.serverName, authtype, attributes.loginName, attributes.password);
					if (isValid) {
						if (attributes.databaseName.empty()) {
							return isValid;
						}
						else {
							return ChooseDatabase(attributes.databaseName);
						}
					}
					else {
						return false;
					}
				}
			}
			else {
				CloseConnection();
				return OpenConnection(attributes);
			}
		default:
			CloseConnection();
			return OpenConnection(attributes);
			break;
		}
	}
	else {
		CloseConnection();
		return OpenConnection(attributes);
	}
}

std::vector<DrvFTSQLHdaItem::Record> DrvFTSQLHdaItem::SQLServerDatabaseEngine::ExecuteStatement(const std::string& query, const std::vector<std::string>& parameters)
{
	std::unique_ptr<SQLServerStatement> stm = std::make_unique<SQLServerStatement>(m_ptrConnection, query, parameters);
	std::vector<Record> vec = stm->Execute();
	return vec;
}

DrvFTSQLHdaItem::SQLServerDatabaseEngine::~SQLServerDatabaseEngine()
{
	CloseConnection();
	m_ptrEnvironment.reset();
}