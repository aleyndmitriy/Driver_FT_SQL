#include"pch.h"
#include"Connection.h"

DrvFTSQLHdaItem::Connection::Connection(const std::shared_ptr<IEnvironment>& environment) : m_ptrEnvironment(environment) {

}

DrvFTSQLHdaItem::Connection::Connection(const std::shared_ptr<IEnvironment>& environment, const ConnectionAttributes& attributes) : m_ptrEnvironment(environment), m_connectionAttributes(attributes) {

}

DrvFTSQLHdaItem::Connection::Connection(std::shared_ptr<IEnvironment>&& environment, ConnectionAttributes&& attributes) : m_ptrEnvironment(std::move(environment)), m_connectionAttributes(std::move(attributes)) {

}

DrvFTSQLHdaItem::Connection::Connection(const std::shared_ptr<IEnvironment>& environment, ConnectionAttributes&& attributes): m_ptrEnvironment(environment), m_connectionAttributes(std::move(attributes)) {

}

DrvFTSQLHdaItem::Connection::Connection(std::shared_ptr<IEnvironment>&& environment) : m_ptrEnvironment(std::move(environment)) {

}

DrvFTSQLHdaItem::ConnectionAttributes DrvFTSQLHdaItem::Connection::GetConnectionAttributes() const
{
	return m_connectionAttributes;
}

DrvFTSQLHdaItem::Connection::~Connection() {
	m_ptrEnvironment.reset();
}