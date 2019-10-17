#pragma once
#include<string>

namespace DrvFTSQLHdaItem {
	
	class ParamValue {
	public:
		ParamValue() = delete;
		ParamValue(std::string&& address, std::string&& fullAddress, std::string&& sql, bool prevPoint, bool postPoint);
		virtual ~ParamValue();
		ParamValue(const ParamValue& src) = default;
		ParamValue(ParamValue&& src) = default;
		ParamValue& operator=(const ParamValue& rhs) = default;
		ParamValue& operator=(ParamValue&& rhs) = default;
		std::string GetAddress() const;
		std::string GetFullAddress() const;
		std::string GetSqc() const;
		bool HasPrevPoint() const;
		bool HasPostPoint() const;
		bool HasSql() const;
	private:
		std::string m_Address;
		std::string m_FullAddress;
		std::string m_Sql;
		bool m_bPrevPoint;
		bool m_bPostPoint;
	};
}