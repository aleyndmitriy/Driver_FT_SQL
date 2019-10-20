#include"pch.h"
#include"TagItemRecord.h"

DrvFTSQLHdaItem::TagItemRecord::TagItemRecord(const std::string& tagName, short tagId, short tagType, short tagDataType):
	m_tagName(tagName),m_tagId(tagId),m_tagType(tagType),m_tagDataType(tagDataType)
{

}

DrvFTSQLHdaItem::TagItemRecord::TagItemRecord() :TagItemRecord(std::string(), -1, -1, -1)
{

}

DrvFTSQLHdaItem::TagItemRecord::~TagItemRecord()
{

}

std::string DrvFTSQLHdaItem::TagItemRecord::GetTegName() const
{
	return m_tagName;
}

short DrvFTSQLHdaItem::TagItemRecord::GetTagId() const
{
	return m_tagId;
}

short DrvFTSQLHdaItem::TagItemRecord::GetTagType() const
{
	return m_tagType;
}

short DrvFTSQLHdaItem::TagItemRecord::GetTagDataType() const
{
	return m_tagDataType;
}

bool DrvFTSQLHdaItem::operator==(const TagItemRecord& lhs, const TagItemRecord& rhs)
{
	return lhs.GetTagId() == rhs.GetTagId();
}

bool DrvFTSQLHdaItem::operator!=(const TagItemRecord& lhs, const TagItemRecord& rhs)
{
	return lhs.GetTagId() != rhs.GetTagId();
}

bool DrvFTSQLHdaItem::operator<(const TagItemRecord& lhs, const TagItemRecord& rhs)
{
	return lhs.GetTagId() < rhs.GetTagId();
}

bool DrvFTSQLHdaItem::operator<=(const TagItemRecord& lhs, const TagItemRecord& rhs)
{
	return lhs.GetTagId() <= rhs.GetTagId();
}

bool DrvFTSQLHdaItem::operator>(const TagItemRecord& lhs, const TagItemRecord& rhs)
{
	return lhs.GetTagId() > rhs.GetTagId();
}

bool DrvFTSQLHdaItem::operator>=(const TagItemRecord& lhs, const TagItemRecord& rhs)
{
	return lhs.GetTagId() >= rhs.GetTagId();
}