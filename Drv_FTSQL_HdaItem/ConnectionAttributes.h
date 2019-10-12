#pragma once
#include<string>

namespace DrvFTSQLHdaItem
{
	struct ConnectionAttributes {
		std::string driver;
		std::string serverName;
		std::string databaseName;
		std::string loginName;
		std::string password;
		bool isServerAuthentication;
		ConnectionAttributes();
		ConnectionAttributes(const ConnectionAttributes& src) = default;
		ConnectionAttributes& operator=(const ConnectionAttributes& rhs) = default;
		~ConnectionAttributes();
	};

	bool operator==(const ConnectionAttributes& lhs, const ConnectionAttributes& rhs);
	bool operator!=(const ConnectionAttributes& lhs, const ConnectionAttributes& rhs);
}