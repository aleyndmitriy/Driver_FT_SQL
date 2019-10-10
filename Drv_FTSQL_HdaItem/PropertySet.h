#pragma once
#include "IPropertySet.h"
#include <map>

namespace DrvFTSQLHdaItem
{
	class PropertySet final : public ODS::IPropertySet {
	public:
		PropertySet();
		PropertySet(const PropertySet& src) = default;
		PropertySet(PropertySet&& src) noexcept = default;
		PropertySet& operator=(const PropertySet& src) = default;
		PropertySet& operator=(PropertySet&& src) = default;
		~PropertySet();
		void GetPropertyInfoList(ODS::PropertyInfo** ppPropInfoList, ULONG* pulCount) const override;
		void DestroyPropertyInfoList(ODS::PropertyInfo* pPropInfoList, ULONG ulCount) const override;
		void GetPropertyList(ODS::Property** ppPropList, ULONG* pulCount) const override;
		void DestroyPropertyList(ODS::Property* pPropList, ULONG ulCount) const override;
		INT GetProperty(ULONG ulPropId, ODS::Property* pProperty) const override;
		INT GetProperty(const TCHAR* pPropName, ODS::Property* pProperty) const override;
		INT SetProperty(const ODS::Property* pProperty) override;
		INT AddProperty(const ULONG ulId, const TCHAR* szName, const TCHAR* szValue);
		INT AddProperty(const ULONG ulId, const TCHAR* szName, const VARIANT* pValue);
	private:
		std::map<ULONG, ODS::Property> propertyList;
	};
}