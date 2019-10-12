#pragma once
#include"ConnectionAttributes.h"
#include<iostream>

namespace DrvFTSQLHdaItem
{
	class ISettingsDataSource {
	public:
		virtual bool Save(const ConnectionAttributes& attributes, std::ostream& stream) = 0;
		virtual bool Load(ConnectionAttributes& attributes, std::istream& stream) = 0;
		virtual ~ISettingsDataSource() {};
	};
}