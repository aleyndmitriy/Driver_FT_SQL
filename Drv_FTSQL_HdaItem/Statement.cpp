#include"pch.h"
#include"Statement.h"

DrvFTSQLHdaItem::Statement::Statement(const std::shared_ptr<Connection>& connection, const std::string& query, std::initializer_list<std::string> list) :m_ptrConnection(connection), strQuery(query), binders(list) {

}

DrvFTSQLHdaItem::Statement::Statement(const std::shared_ptr<Connection>& connection, const std::string& query) : Statement(connection, query, std::initializer_list<std::string>{}) {

}

DrvFTSQLHdaItem::Statement::Statement(std::shared_ptr<Connection>&& connection, std::string&& query, std::initializer_list<std::string> list) : m_ptrConnection(std::move(connection)), strQuery(std::move(query)), binders(list) {

}

DrvFTSQLHdaItem::Statement::Statement(std::shared_ptr<Connection>&& connection, std::string&& query) : Statement(std::move(connection), std::move(query), std::initializer_list<std::string>{}) {

}


DrvFTSQLHdaItem::Statement::Statement(const std::shared_ptr<Connection>& connection, const std::string& query, const std::vector<std::string>& parameters) : m_ptrConnection(connection), strQuery(query), binders(parameters)
{

}

DrvFTSQLHdaItem::Statement::Statement(const std::shared_ptr<Connection>& connection, const std::string& query, std::vector<std::string>&& parameters) : m_ptrConnection(connection), strQuery(query), binders(std::move(parameters))
{

}

DrvFTSQLHdaItem::Statement::Statement(std::shared_ptr<Connection>&& connection, std::string&& query, std::vector<std::string>&& parameters) : m_ptrConnection(std::move(connection)), strQuery(std::move(query)), binders(std::move(parameters))
{

}

DrvFTSQLHdaItem::Statement::~Statement() {
	m_ptrConnection.reset();
	strQuery.clear();
	binders.clear();
}

DrvFTSQLHdaItem::Statement::Statement(Statement&& src) noexcept {
	m_ptrConnection = src.m_ptrConnection;
	src.m_ptrConnection.reset();
	strQuery = std::move(src.strQuery);
	binders = std::move(src.binders);
}

DrvFTSQLHdaItem::Statement& DrvFTSQLHdaItem::Statement::operator=(Statement&& rhs) noexcept
{
	if (this == &rhs) {
		return *this;
	}
	else {
		m_ptrConnection = rhs.m_ptrConnection;
		rhs.m_ptrConnection.reset();
		strQuery = std::move(rhs.strQuery);
		binders = std::move(rhs.binders);
		return *this;
	}
}

std::string DrvFTSQLHdaItem::Statement::getQueryString() const {
	return strQuery;
}
