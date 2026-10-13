#include "EnumNames.h"
#include <deque>
#include <string>
#include <unordered_map>

namespace
{
struct EnumTable
{
	const char *mEnumName = nullptr;
	std::unordered_map<int64_t, const char *> mNamesByValue;
	std::unordered_map<std::string, int64_t> mValuesByName;
};

std::unordered_map<const void *, EnumTable> &Tables()
{
	static std::unordered_map<const void *, EnumTable> sTables;
	return sTables;
}
} // namespace

void EnumNames::Register(const void *theEnumKey, const char *theEnumName, const EnumNameEntry *theEntries,
                         size_t theCount)
{
	EnumTable &aTable = Tables()[theEnumKey];
	aTable.mEnumName = theEnumName;
	for (size_t i = 0; i < theCount; i++)
	{
		// Several names can share a value (NUM_ markers, aliases): the first one is the value's name
		aTable.mNamesByValue.emplace(theEntries[i].mValue, theEntries[i].mName);
		aTable.mValuesByName.emplace(theEntries[i].mName, theEntries[i].mValue);
	}
}

void EnumNames::Add(const void *theEnumKey, const std::string &theName, int64_t theValue)
{
	// Names added at runtime need storage that never moves
	static std::deque<std::string> sNames;

	EnumTable &aTable = Tables()[theEnumKey];
	auto anOld = aTable.mValuesByName.find(theName);
	if (anOld != aTable.mValuesByName.end())
	{
		if (anOld->second == theValue)
			return;
		auto anOldName = aTable.mNamesByValue.find(anOld->second);
		if (anOldName != aTable.mNamesByValue.end() && theName == anOldName->second)
		{
			aTable.mNamesByValue.erase(anOldName);
		}
	}

	sNames.push_back(theName);
	aTable.mValuesByName[theName] = theValue;
	aTable.mNamesByValue[theValue] = sNames.back().c_str();
}

const char *EnumNames::NameOf(const void *theEnumKey, int64_t theValue)
{
	auto aTable = Tables().find(theEnumKey);
	if (aTable == Tables().end())
		return nullptr;

	auto aName = aTable->second.mNamesByValue.find(theValue);
	return aName == aTable->second.mNamesByValue.end() ? nullptr : aName->second;
}

bool EnumNames::ValueOf(const void *theEnumKey, const char *theName, int64_t &theValue)
{
	auto aTable = Tables().find(theEnumKey);
	if (aTable == Tables().end())
		return false;

	auto aValue = aTable->second.mValuesByName.find(theName);
	if (aValue == aTable->second.mValuesByName.end())
		return false;

	theValue = aValue->second;
	return true;
}
