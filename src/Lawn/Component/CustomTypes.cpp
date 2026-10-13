#include "CustomTypes.h"
#include <array>
#include "EnumNames.h"
#include "../../SexyAppFramework/SexyAppBase.h"

namespace
{
constexpr int kZombieSlots = MAX_ZOMBIE_TYPES - FIRST_CUSTOM_ZOMBIE_TYPE;
constexpr int kPlantSlots = MAX_SEED_TYPES - FIRST_CUSTOM_SEED_TYPE;

struct Registry
{
	// Fixed slots: the definitions' name pointers stay valid for the whole run
	std::array<CustomTypes::ZombieInfo, kZombieSlots> mZombies;
	std::array<CustomTypes::PlantInfo, kPlantSlots> mPlants;
	std::vector<ZombieType> mAllZombies, mCustomZombies;
	std::vector<SeedType> mAllPlants, mCustomPlants;
	bool mListsBuilt = false;
};

Registry &GetRegistry()
{
	static Registry sRegistry;
	return sRegistry;
}

void BuildLists()
{
	Registry &aRegistry = GetRegistry();
	aRegistry.mAllZombies.clear();
	aRegistry.mCustomZombies.clear();
	aRegistry.mAllPlants.clear();
	aRegistry.mCustomPlants.clear();

	for (int i = 0; i < (int)ZombieType::NUM_ZOMBIE_TYPES; i++)
	{
		aRegistry.mAllZombies.push_back((ZombieType)i);
	}
	for (const CustomTypes::ZombieInfo &aZombie : aRegistry.mZombies)
	{
		if (aZombie.mActive)
		{
			aRegistry.mAllZombies.push_back(aZombie.mType);
			aRegistry.mCustomZombies.push_back(aZombie.mType);
		}
	}

	for (int i = 0; i < (int)SeedType::NUM_SEED_TYPES; i++)
	{
		aRegistry.mAllPlants.push_back((SeedType)i);
	}
	for (const CustomTypes::PlantInfo &aPlant : aRegistry.mPlants)
	{
		if (aPlant.mActive)
		{
			aRegistry.mAllPlants.push_back(aPlant.mType);
			aRegistry.mCustomPlants.push_back(aPlant.mType);
		}
	}
	aRegistry.mListsBuilt = true;
}

Registry &GetListedRegistry()
{
	Registry &aRegistry = GetRegistry();
	if (!aRegistry.mListsBuilt)
	{
		BuildLists();
	}
	return aRegistry;
}

/// @brief The slot for a type name: the one it had, a requested one, or the first free one; -1 if none
template <typename TInfo, size_t N>
int PickSlot(std::array<TInfo, N> &theSlots, const std::string &theTypeName, int theRequestedSlot,
             std::string &theError)
{
	int aSlotOfName = -1;
	for (size_t i = 0; i < N; i++)
	{
		if (theSlots[i].mTypeName == theTypeName)
		{
			aSlotOfName = (int)i;
		}
	}

	if (theRequestedSlot >= 0)
	{
		if (theRequestedSlot >= (int)N)
		{
			theError = "\"id\" is out of range";
			return -1;
		}
		const std::string &anOwner = theSlots[theRequestedSlot].mTypeName;
		if (!anOwner.empty() && anOwner != theTypeName)
		{
			theError = "\"id\" is already used by " + anOwner;
			return -1;
		}
		if (aSlotOfName >= 0 && aSlotOfName != theRequestedSlot)
		{
			// Moved to a new ID: free the old one
			theSlots[aSlotOfName] = TInfo();
		}
		return theRequestedSlot;
	}

	if (aSlotOfName >= 0)
		return aSlotOfName;

	for (size_t i = 0; i < N; i++)
	{
		if (theSlots[i].mTypeName.empty())
			return (int)i;
	}
	theError = "too many custom types";
	return -1;
}
} // namespace

bool CustomTypes::IsCustom(ZombieType theType)
{
	return theType >= FIRST_CUSTOM_ZOMBIE_TYPE && theType < MAX_ZOMBIE_TYPES;
}

bool CustomTypes::IsCustom(SeedType theType)
{
	return theType >= FIRST_CUSTOM_SEED_TYPE && theType < MAX_SEED_TYPES;
}

bool CustomTypes::Exists(ZombieType theType)
{
	if (theType >= 0 && theType < ZombieType::NUM_ZOMBIE_TYPES)
		return true;
	return GetZombie(theType) != nullptr;
}

bool CustomTypes::Exists(SeedType theType)
{
	if (theType >= 0 && theType < SeedType::NUM_SEED_TYPES)
		return true;
	return GetPlant(theType) != nullptr;
}

ZombieType CustomTypes::PlaysAs(ZombieType theType)
{
	if (ZombieInfo *anInfo = GetZombie(theType))
		return anInfo->mBase;
	return theType;
}

SeedType CustomTypes::PlaysAs(SeedType theType)
{
	if (PlantInfo *anInfo = GetPlant(theType))
		return anInfo->mBase;
	return theType;
}

const std::vector<ZombieType> &CustomTypes::AllZombieTypes()
{
	return GetListedRegistry().mAllZombies;
}

const std::vector<SeedType> &CustomTypes::AllPlantTypes()
{
	return GetListedRegistry().mAllPlants;
}

const std::vector<ZombieType> &CustomTypes::CustomZombieTypes()
{
	return GetListedRegistry().mCustomZombies;
}

const std::vector<SeedType> &CustomTypes::CustomPlantTypes()
{
	return GetListedRegistry().mCustomPlants;
}

CustomTypes::ZombieInfo *CustomTypes::GetZombie(ZombieType theType)
{
	if (!IsCustom(theType))
		return nullptr;
	ZombieInfo &anInfo = GetRegistry().mZombies[theType - FIRST_CUSTOM_ZOMBIE_TYPE];
	return anInfo.mTypeName.empty() ? nullptr : &anInfo;
}

CustomTypes::PlantInfo *CustomTypes::GetPlant(SeedType theType)
{
	if (!IsCustom(theType))
		return nullptr;
	PlantInfo &anInfo = GetRegistry().mPlants[theType - FIRST_CUSTOM_SEED_TYPE];
	return anInfo.mTypeName.empty() ? nullptr : &anInfo;
}

void CustomTypes::BeginReload()
{
	Registry &aRegistry = GetRegistry();
	for (ZombieInfo &aZombie : aRegistry.mZombies)
	{
		aZombie.mActive = false;
	}
	for (PlantInfo &aPlant : aRegistry.mPlants)
	{
		aPlant.mActive = false;
	}
}

CustomTypes::ZombieInfo *CustomTypes::RegisterZombie(const std::string &theTypeName, ZombieType theBase,
                                                     int theRequestedId, std::string &theError)
{
	Registry &aRegistry = GetRegistry();
	int aRequestedSlot = theRequestedId < 0 ? -1 : theRequestedId - FIRST_CUSTOM_ZOMBIE_TYPE;
	if (theRequestedId >= 0 && aRequestedSlot < 0)
	{
		theError = "\"id\" must be at least " + std::to_string(FIRST_CUSTOM_ZOMBIE_TYPE);
		return nullptr;
	}
	int aSlot = PickSlot(aRegistry.mZombies, theTypeName, aRequestedSlot, theError);
	if (aSlot < 0)
		return nullptr;

	ZombieInfo &anInfo = aRegistry.mZombies[aSlot];
	anInfo.mTypeName = theTypeName;
	anInfo.mType = (ZombieType)(FIRST_CUSTOM_ZOMBIE_TYPE + aSlot);
	anInfo.mBase = theBase;
	anInfo.mActive = true;
	EnumNames::Add(EnumKey<ZombieType>(), anInfo.mTypeName, anInfo.mType);
	return &anInfo;
}

CustomTypes::PlantInfo *CustomTypes::RegisterPlant(const std::string &theTypeName, SeedType theBase,
                                                   int theRequestedId, std::string &theError)
{
	Registry &aRegistry = GetRegistry();
	int aRequestedSlot = theRequestedId < 0 ? -1 : theRequestedId - FIRST_CUSTOM_SEED_TYPE;
	if (theRequestedId >= 0 && aRequestedSlot < 0)
	{
		theError = "\"id\" must be at least " + std::to_string(FIRST_CUSTOM_SEED_TYPE);
		return nullptr;
	}
	int aSlot = PickSlot(aRegistry.mPlants, theTypeName, aRequestedSlot, theError);
	if (aSlot < 0)
		return nullptr;

	PlantInfo &anInfo = aRegistry.mPlants[aSlot];
	anInfo.mTypeName = theTypeName;
	anInfo.mType = (SeedType)(FIRST_CUSTOM_SEED_TYPE + aSlot);
	anInfo.mBase = theBase;
	anInfo.mActive = true;
	EnumNames::Add(EnumKey<SeedType>(), anInfo.mTypeName, anInfo.mType);
	return &anInfo;
}

void CustomTypes::EndReload()
{
	BuildLists();
}

void CustomTypes::SetString(const std::string &theKey, const std::string &theValue)
{
	if (Sexy::gSexyAppBase)
	{
		Sexy::gSexyAppBase->mStringProperties[theKey] = theValue;
	}
}

std::string CustomTypes::GetString(const std::string &theKey)
{
	if (Sexy::gSexyAppBase == nullptr)
		return std::string();
	auto anIt = Sexy::gSexyAppBase->mStringProperties.find(theKey);
	return anIt == Sexy::gSexyAppBase->mStringProperties.end() ? std::string() : anIt->second;
}
