#include "pch.h"
#include"ConnectionAttributes.h"

DrvFTSQLHdaItem::ConnectionAttributes::ConnectionAttributes() :driver(), serverName(), databaseName(), loginName(), password(), isServerAuthentication(false), daysBack()
{

}

DrvFTSQLHdaItem::ConnectionAttributes::~ConnectionAttributes()
{

}

bool DrvFTSQLHdaItem::operator==(const ConnectionAttributes& lhs, const ConnectionAttributes& rhs)
{
	return (lhs.driver == rhs.driver && lhs.serverName == rhs.serverName && lhs.loginName == rhs.loginName &&
		lhs.password == rhs.password && lhs.databaseName == rhs.databaseName && lhs.isServerAuthentication == rhs.isServerAuthentication);
}

bool DrvFTSQLHdaItem::operator!=(const ConnectionAttributes& lhs, const ConnectionAttributes& rhs)
{
	return (lhs.driver != rhs.driver || lhs.serverName != rhs.serverName || lhs.loginName != rhs.loginName ||
		lhs.password != rhs.password || lhs.databaseName != rhs.databaseName || lhs.isServerAuthentication != rhs.isServerAuthentication);
}