#include"pch.h"
#include"ParamUtils.h"

#include <HdaFunctionParam.h>
#include <AddressComponent.h>
#include <AddressHelper.h>
#include <Address.h>
#include <AddressComponentHelper.h>
#include <OdsErr.h>


	namespace ParamUtils
	{

		void GetAddress(ODS::OdsString& rAddress, ODS::OdsString& rFullAddress, const ODS::HdaFunctionParam* pParam)
		{
			ODS::HdaFunctionParamItemAddress* pIap = (ODS::HdaFunctionParamItemAddress*)pParam;

			ODS::ItemAddress ia;
			pIap->GetItemAddress(&ia);
			ODS::AddressComponent* addrComp = nullptr;
			int nCount = 0;
			int nIndex = 0;
			ODS::AddressHelper ah(&ia);

			rFullAddress = ah.GetPlainAddress();

			std::vector<ODS::AddressComponent> al;
			ia.GetAddress(&addrComp, &nCount, &nIndex);
			for (int index = 0; index < nCount; index++) {
				al.push_back(addrComp[index]);
			}

			if (al.size() > 0)
			{
				ODS::AddressComponentHelper ach1(&al[al.size() - 1]);
				rAddress = ach1.GetValue();
			}
		}

		void GetAddress(const ODS::OdsString& rPlainAddress, ODS::OdsString& rAddress)
		{
			ODS::ItemAddress ia;
			ia.SetPlainAddress((LPCTSTR)rPlainAddress);
			ODS::AddressHelper ah(&ia);

			std::vector<ODS::AddressComponent> al;
			ah.GetAddress(&al, 0);

			if (al.size() > 0)
			{
				ODS::AddressComponentHelper ach1(&al[al.size() - 1]);
				rAddress = ach1.GetValue();
			}
		}

		ODS::OdsString GetSql(const ODS::HdaFunctionParam* pParam)
		{
			ODS::HdaFunctionParamSql* pSql = (ODS::HdaFunctionParamSql*)pParam;

			ODS::OdsString text(_T(""));

			TCHAR* szText = 0;
			int rc = pSql->GetText(&szText);
			if (ODS::ERR::OK == rc)
			{
				text = szText;
				pSql->DestroyString(szText);
			}
			return text;
		}
	}