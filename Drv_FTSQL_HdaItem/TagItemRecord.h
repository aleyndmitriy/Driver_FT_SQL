#pragma once
#include<string>

namespace DrvFTSQLHdaItem
{
	class TagItemRecord {
	public:
		TagItemRecord();
		TagItemRecord(const std::string& tagName, short tagId, short tagType, short tagDataType);
		~TagItemRecord();
		TagItemRecord(const TagItemRecord& src) = default;
		TagItemRecord(TagItemRecord&& src) = default;
		TagItemRecord& operator=(const TagItemRecord& src) = default;
		TagItemRecord& operator=(TagItemRecord && src) = default;
		std::string GetTegName() const;
		short GetTagId() const;
		short GetTagType() const;
		short GetTagDataType() const;
	private:
		std::string m_tagName;
		short m_tagId;
		short m_tagType;
		short m_tagDataType;
	};

	bool operator==(const TagItemRecord& lhs, const TagItemRecord& rhs);
	bool operator!=(const TagItemRecord& lhs, const TagItemRecord& rhs);
	bool operator<(const TagItemRecord& lhs, const TagItemRecord& rhs);
	bool operator<=(const TagItemRecord& lhs, const TagItemRecord& rhs);
	bool operator>(const TagItemRecord& lhs, const TagItemRecord& rhs);
	bool operator>=(const TagItemRecord& lhs, const TagItemRecord& rhs);
}