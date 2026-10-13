#include "ComponentDefinitions.h"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <json.hpp>
#include "Archetypes.h"
#include "EnumNames.h"
#include "CustomTypes.h"
#include "../Challenge.h"
#include "../../Sexy.TodLib/Reanimator.h"
#include "../Plant.h"

using Json = nlohmann::ordered_json;
namespace fs = std::filesystem;

// ─── JSON fields ─────────────────────────────────────────────────────────────

namespace
{
ReanimationType RegisterReanimFile(const std::string &theFile);

/// @brief ComponentFields over a JSON object (see ComponentArchive)
class JsonFields : public ComponentFields
{
  public:
	JsonFields(Json &theObject, bool theReading, std::vector<std::string> *theErrors)
	    : mReading(theReading), mErrors(theErrors)
	{
		mStack.push_back({ &theObject, 0 });
	}

	/// @brief The top-level keys Serialize read (to report the ones it didn't know)
	const std::set<std::string> &GetReadKeys() const
	{
		return mReadKeys;
	}

	bool Integer(const char *theName, int64_t &theValue) override
	{
		if (!mReading)
		{
			Slot(theName) = theValue;
			return true;
		}
		const Json *aValue = Find(theName);
		if (aValue == nullptr)
			return false;
		if (aValue->is_number_integer() || aValue->is_number_unsigned())
		{
			theValue = aValue->get<int64_t>();
			return true;
		}
		if (aValue->is_number_float())
		{
			theValue = (int64_t)aValue->get<double>();
			return true;
		}
		if (aValue->is_boolean())
		{
			theValue = aValue->get<bool>() ? 1 : 0;
			return true;
		}
		Error(theName, "expected a whole number");
		return false;
	}

	bool Number(const char *theName, double &theValue, bool theIsFloat) override
	{
		if (!mReading)
		{
			Slot(theName) = theIsFloat ? ShortestFloat(theValue) : theValue;
			return true;
		}
		const Json *aValue = Find(theName);
		if (aValue == nullptr)
			return false;
		if (!aValue->is_number())
		{
			Error(theName, "expected a number");
			return false;
		}
		theValue = aValue->get<double>();
		return true;
	}

	bool Boolean(const char *theName, bool &theValue) override
	{
		if (!mReading)
		{
			Slot(theName) = theValue;
			return true;
		}
		const Json *aValue = Find(theName);
		if (aValue == nullptr)
			return false;
		if (aValue->is_boolean())
		{
			theValue = aValue->get<bool>();
			return true;
		}
		if (aValue->is_number())
		{
			theValue = aValue->get<double>() != 0.0;
			return true;
		}
		Error(theName, "expected true or false");
		return false;
	}

	bool Text(const char *theName, std::string &theValue) override
	{
		if (!mReading)
		{
			Slot(theName) = theValue;
			return true;
		}
		const Json *aValue = Find(theName);
		if (aValue == nullptr)
			return false;
		if (!aValue->is_string())
		{
			Error(theName, "expected a string");
			return false;
		}
		theValue = aValue->get<std::string>();
		return true;
	}

	bool Enum(const char *theName, int64_t &theValue, const void *theEnumKey) override
	{
		if (!mReading)
		{
			const char *anEnumName = EnumNames::NameOf(theEnumKey, theValue);
			if (anEnumName)
			{
				Slot(theName) = anEnumName;
			}
			else
			{
				Slot(theName) = theValue;
			}
			return true;
		}
		const Json *aValue = Find(theName);
		if (aValue == nullptr)
			return false;
		if (aValue->is_number_integer() || aValue->is_number_unsigned())
		{
			theValue = aValue->get<int64_t>();
			return true;
		}
		if (aValue->is_string())
		{
			if (EnumNames::ValueOf(theEnumKey, aValue->get<std::string>().c_str(), theValue))
				return true;
			if (theEnumKey == EnumKey<ReanimationType>() && aValue->get<std::string>().find(".reanim") != std::string::npos)
			{
				ReanimationType aReanim = RegisterReanimFile(aValue->get<std::string>());
				if (aReanim != ReanimationType::REANIM_NONE)
				{
					theValue = aReanim;
					return true;
				}
			}
			Error(theName, "unknown name \"" + aValue->get<std::string>() + "\"");
			return false;
		}
		Error(theName, "expected a name or a number");
		return false;
	}

	bool Bytes(const char *theName, void *theData, size_t theSize) override
	{
		static const char kHex[] = "0123456789abcdef";
		uint8_t *aBytes = static_cast<uint8_t *>(theData);
		if (!mReading)
		{
			std::string aText;
			for (size_t i = 0; i < theSize; i++)
			{
				aText += kHex[aBytes[i] >> 4];
				aText += kHex[aBytes[i] & 15];
			}
			Slot(theName) = aText;
			return true;
		}
		const Json *aValue = Find(theName);
		if (aValue == nullptr)
			return false;
		if (!aValue->is_string() || aValue->get<std::string>().size() != theSize * 2)
		{
			Error(theName, "expected " + std::to_string(theSize * 2) + " hex digits");
			return false;
		}
		const std::string aText = aValue->get<std::string>();
		for (size_t i = 0; i < theSize; i++)
		{
			aBytes[i] = (uint8_t)std::stoi(aText.substr(i * 2, 2), nullptr, 16);
		}
		return true;
	}

	bool BeginList(const char *theName, size_t &theCount) override
	{
		if (!mReading)
		{
			Json &aList = Slot(theName);
			aList = Json::array();
			mStack.push_back({ &aList, 0 });
			return true;
		}
		const Json *aValue = Find(theName);
		if (aValue == nullptr)
			return false;
		if (!aValue->is_array())
		{
			Error(theName, "expected a list");
			return false;
		}
		theCount = aValue->size();
		mStack.push_back({ const_cast<Json *>(aValue), 0 });
		return true;
	}

	void EndList() override
	{
		mStack.pop_back();
	}

	bool BeginObject(const char *theName) override
	{
		if (!mReading)
		{
			Json &anObject = Slot(theName);
			anObject = Json::object();
			mStack.push_back({ &anObject, 0 });
			return true;
		}
		const Json *aValue = Find(theName);
		if (aValue == nullptr)
			return false;
		if (!aValue->is_object())
		{
			Error(theName, "expected an object");
			return false;
		}
		mStack.push_back({ const_cast<Json *>(aValue), 0 });
		return true;
	}

	void EndObject() override
	{
		mStack.pop_back();
	}

  private:
	struct Frame
	{
		Json *mNode;
		size_t mNextIndex; // the next element of a list
	};

	/// @brief Reading: the value of a field (theName null: the next list element), or nullptr
	const Json *Find(const char *theName)
	{
		Frame &aFrame = mStack.back();
		if (theName == nullptr)
		{
			if (aFrame.mNode->is_array() && aFrame.mNextIndex < aFrame.mNode->size())
				return &(*aFrame.mNode)[aFrame.mNextIndex++];
			return nullptr;
		}
		if (mStack.size() == 1)
		{
			mReadKeys.insert(theName);
		}
		if (!aFrame.mNode->is_object())
			return nullptr;
		auto anIt = aFrame.mNode->find(theName);
		return anIt == aFrame.mNode->end() ? nullptr : &*anIt;
	}

	/// @brief Writing: where a field goes (theName null: a new list element)
	Json &Slot(const char *theName)
	{
		Frame &aFrame = mStack.back();
		if (theName == nullptr)
		{
			aFrame.mNode->push_back(nullptr);
			return aFrame.mNode->back();
		}
		return (*aFrame.mNode)[theName];
	}

	void Error(const char *theName, const std::string &theProblem)
	{
		if (mErrors)
		{
			mErrors->push_back(std::string(theName ? theName : "(list element)") + ": " + theProblem);
		}
	}

	/// @brief The float as a double with the fewest digits that still reads back as the same float (0.66f, not
	/// 0.6600000262260437)
	static double ShortestFloat(double theValue)
	{
		float aFloat = (float)theValue;
		char aText[32];
		for (int aDigits = 6; aDigits <= 9; aDigits++)
		{
			snprintf(aText, sizeof(aText), "%.*g", aDigits, aFloat);
			if ((float)strtod(aText, nullptr) == aFloat)
				break;
		}
		return strtod(aText, nullptr);
	}

	std::vector<Frame> mStack;
	bool mReading;
	std::vector<std::string> *mErrors;
	std::set<std::string> mReadKeys;
};

// ─── Components <-> JSON ─────────────────────────────────────────────────────

/// @brief A component's fields as a JSON object
Json FieldsOf(Component *theComponent)
{
	Json aFields = Json::object();
	JsonFields aWriter(aFields, false, nullptr);
	ComponentArchive anArchive(aWriter, false);
	theComponent->Serialize(anArchive);
	return aFields;
}

/// @brief A component as written in a definition file: its type, then the fields that differ from a
/// default-constructed one
Json ComponentToJson(Component *theComponent)
{
	Json aJson = Json::object();
	aJson["type"] = theComponent->GetTypeName();

	Json aFields = FieldsOf(theComponent);
	if (aFields.contains("type"))
	{
		// "type" names the component in a definition file: a field can't use it
		fprintf(stderr, "[Definitions] %s has a field named \"type\": rename it in its Serialize\n",
		        theComponent->GetTypeName());
	}
	Component *aDefault = ComponentFactory::Create(theComponent->GetTypeName());
	Json aDefaultFields = aDefault ? FieldsOf(aDefault) : Json::object();
	delete aDefault;

	for (auto &[aKey, aValue] : aFields.items())
	{
		auto aDefaultValue = aDefaultFields.find(aKey);
		if (aDefaultValue == aDefaultFields.end() || *aDefaultValue != aValue)
		{
			aJson[aKey] = aValue;
		}
	}
	return aJson;
}

/// @brief Create a component from its definition-file object; nullptr (and a message) when its type is unknown
Component *ComponentFromJson(const Json &theJson, const std::string &theWhere, std::vector<std::string> &theMessages)
{
	if (!theJson.is_object() || !theJson.contains("type") || !theJson["type"].is_string())
	{
		theMessages.push_back(theWhere + ": a component needs a \"type\"");
		return nullptr;
	}

	std::string aType = theJson["type"].get<std::string>();
	Component *aComponent = ComponentFactory::Create(aType.c_str());
	if (aComponent == nullptr)
	{
		theMessages.push_back(theWhere + ": unknown component type \"" + aType + "\"");
		return nullptr;
	}

	std::vector<std::string> anErrors;
	Json aCopy = theJson;
	JsonFields aReader(aCopy, true, &anErrors);
	ComponentArchive anArchive(aReader, true);
	aComponent->Serialize(anArchive);

	for (const std::string &anError : anErrors)
	{
		theMessages.push_back(theWhere + ", " + aType + "." + anError);
	}
	for (auto &[aKey, aValue] : theJson.items())
	{
		if (aKey != "type" && !aReader.GetReadKeys().count(aKey))
		{
			theMessages.push_back(theWhere + ", " + aType + ": unknown field \"" + aKey + "\" (ignored)");
		}
	}
	return aComponent;
}

// ─── Kinds of definitions ────────────────────────────────────────────────────

struct KindInfo
{
	EntityKind mKind;
	const char *mFolder; // under the definitions folder
	const char *mKey;    // the key naming the type in a file
	const void *mEnumKey;
	const char *mPrefix; // stripped from the type name for the file name
	int mCount;          // the built-in types
};

const KindInfo &KindOf(EntityKind theKind)
{
	static const KindInfo kKinds[] = {
		{ EntityKind::Plant,      "plants",      "plant",      EnumKey<SeedType>(),       "SEED_",       SeedType::NUM_SEED_TYPES     },
		{ EntityKind::Zombie,     "zombies",     "zombie",     EnumKey<ZombieType>(),     "ZOMBIE_",     ZombieType::NUM_ZOMBIE_TYPES },
		{ EntityKind::Projectile, "projectiles", "projectile", EnumKey<ProjectileType>(), "PROJECTILE_",
         ProjectileType::NUM_PROJECTILES		                                                                                      },
	};
	for (const KindInfo &aKind : kKinds)
	{
		if (aKind.mKind == theKind)
			return aKind;
	}
	return kKinds[0];
}

const EntityKind kKinds[] = { EntityKind::Plant, EntityKind::Zombie, EntityKind::Projectile };

void BuildVanilla(EntityKind theKind, int theType, ComponentList &theOut)
{
	switch (theKind)
	{
	case EntityKind::Plant:
		Archetypes::BuildVanillaPlant((SeedType)theType, GetPlantDefinition((SeedType)theType).mLaunchRate, theOut);
		break;
	case EntityKind::Zombie:
		Archetypes::BuildVanillaZombie((ZombieType)theType, theOut);
		break;
	case EntityKind::Projectile:
		Archetypes::BuildVanillaProjectile((ProjectileType)theType, theOut);
		break;
	default:
		break;
	}
}

// ─── Loaded definitions ──────────────────────────────────────────────────────

struct Definition
{
	std::string mFile;
	/// null: a custom type that uses its base's components
	Json mComponents;
};

struct Store
{
	bool mLoaded = false;
	std::map<std::pair<EntityKind, int>, Definition> mDefinitions;
	std::vector<std::string> mMessages;
};

Store &GetStore()
{
	static Store sStore;
	return sStore;
}

void Log(const std::string &theMessage)
{
	GetStore().mMessages.push_back(theMessage);
	fprintf(stderr, "[Definitions] %s\n", theMessage.c_str());
}

// ─── Custom types ────────────────────────────────────────────────────────────

ReanimationType RegisterReanimFile(const std::string &theFile)
{
	ReanimationType aType = ReanimatorRegisterCustom(theFile.c_str());
	if (aType != ReanimationType::REANIM_NONE)
	{
		// Components can name it too ("charredReanim": "reanim/My_charred.reanim")
		EnumNames::Add(EnumKey<ReanimationType>(), theFile, aType);
	}
	return aType;
}

int ReadInt(const Json &theObject, const char *theKey, int theDefault, const std::string &theWhere)
{
	auto anIt = theObject.find(theKey);
	if (anIt == theObject.end())
		return theDefault;
	if (!anIt->is_number())
	{
		Log(theWhere + ": \"" + theKey + "\" should be a number");
		return theDefault;
	}
	return (int)anIt->get<double>();
}

/// @brief The "reanim" of an "info": a reanimation type ("REANIM_ZOMBIE_FOOTBALL") or a .reanim file
/// ("reanim/Zombie_blackfootball.reanim")
void ReadReanim(const Json &theInfo, ReanimationType &theReanim, const std::string &theWhere)
{
	auto anIt = theInfo.find("reanim");
	if (anIt == theInfo.end())
		return;
	if (!anIt->is_string())
	{
		Log(theWhere + ": \"reanim\" should be a reanimation name or a .reanim file");
		return;
	}

	std::string aName = anIt->get<std::string>();
	int64_t aValue;
	if (EnumNames::ValueOf(EnumKey<ReanimationType>(), aName.c_str(), aValue))
	{
		theReanim = (ReanimationType)aValue;
		return;
	}
	ReanimationType aCustom = RegisterReanimFile(aName);
	if (aCustom != ReanimationType::REANIM_NONE)
	{
		theReanim = aCustom;
	}
	else if (gReanimatorDefArray != nullptr) // the command-line tools run without animations
	{
		Log(theWhere + ": can't use reanim \"" + aName + "\" (unknown name, or too many custom reanims)");
	}
}

/// @brief Report the keys of an "info" the game doesn't know
void CheckInfoKeys(const Json &theInfo, std::initializer_list<const char *> theKnown, const std::string &theWhere)
{
	for (auto &[aKey, aValue] : theInfo.items())
	{
		bool isKnown = false;
		for (const char *aKnown : theKnown)
		{
			isKnown |= aKey == aKnown;
		}
		if (!isKnown)
		{
			Log(theWhere + ": unknown \"info\" field \"" + aKey + "\" (ignored)");
		}
	}
}

/// @brief The name, tooltip and Almanac texts of a custom type: the file's, else its base's
void SetTypeStrings(const std::string &theKey, const std::string &theBaseKey, const Json &theInfo, bool theIsPlant)
{
	auto aSet = [&](const char *theField, const char *theSuffix) {
		std::string aValue;
		auto anIt = theInfo.find(theField);
		if (anIt != theInfo.end() && anIt->is_string())
		{
			aValue = anIt->get<std::string>();
		}
		else
		{
			aValue = CustomTypes::GetString(theBaseKey + theSuffix);
		}
		CustomTypes::SetString(theKey + theSuffix, aValue);
	};
	aSet("name", "");
	aSet("description", "_DESCRIPTION");
	aSet("descriptionHeader", "_DESCRIPTION_HEADER");
	if (theIsPlant)
	{
		aSet("tooltip", "_TOOLTIP");
	}
}

/// @brief Read "basedOn" as a game type; -1 (and a message) when it isn't one
int ReadBase(const Json &theRoot, const KindInfo &theKind, const std::string &theWhere)
{
	const Json &aBasedOn = theRoot["basedOn"];
	int64_t aBase;
	if (!aBasedOn.is_string() || !EnumNames::ValueOf(theKind.mEnumKey, aBasedOn.get<std::string>().c_str(), aBase) ||
	    aBase < 0 || aBase >= theKind.mCount)
	{
		Log(theWhere + ": \"basedOn\" must name one of the game's " + theKind.mKey + " types");
		return -1;
	}
	return (int)aBase;
}

/// @brief Register a custom zombie type from its file; its type, or -1
int RegisterCustomZombie(const Json &theRoot, const std::string &theTypeName, const KindInfo &theKind,
                         const std::string &theWhere)
{
	int aBase = ReadBase(theRoot, theKind, theWhere);
	if (aBase < 0)
		return -1;

	std::string anError;
	CustomTypes::ZombieInfo *anInfo = CustomTypes::RegisterZombie(theTypeName, (ZombieType)aBase,
	                                                              ReadInt(theRoot, "id", -1, theWhere), anError);
	if (anInfo == nullptr)
	{
		Log(theWhere + ": " + anError);
		return -1;
	}
	anInfo->mFile = theWhere;

	// What the file doesn't say comes from the base
	const ZombieDefinition &aBaseDef = GetZombieDefinition((ZombieType)aBase);
	Json anInfoJson = theRoot.contains("info") ? theRoot["info"] : Json::object();
	CheckInfoKeys(anInfoJson,
	              { "name", "description", "descriptionHeader", "reanim", "value", "startingLevel", "firstAllowedWave",
	                "pickWeight", "allowedLevels" },
	              theWhere);

	ZombieDefinition aDef = aBaseDef;
	aDef.mZombieType = anInfo->mType;
	ReadReanim(anInfoJson, aDef.mReanimationType, theWhere);
	aDef.mZombieValue = ReadInt(anInfoJson, "value", aDef.mZombieValue, theWhere);
	aDef.mStartingLevel = ReadInt(anInfoJson, "startingLevel", aDef.mStartingLevel, theWhere);
	aDef.mFirstAllowedWave = ReadInt(anInfoJson, "firstAllowedWave", aDef.mFirstAllowedWave, theWhere);
	// Out of the levels until the file opts in with a pick weight
	aDef.mPickWeight = ReadInt(anInfoJson, "pickWeight", 0, theWhere);
	aDef.mZombieName = anInfo->mTypeName.c_str();
	anInfo->mDefinition = aDef;

	// The adventure levels it can appear on
	for (int i = 0; i < 50; i++)
	{
		anInfo->mAllowedOnLevel[i] = gZombieAllowedLevels[aBase].mAllowedOnLevel[i];
	}
	if (anInfoJson.contains("allowedLevels"))
	{
		const Json &aLevels = anInfoJson["allowedLevels"];
		if (aLevels.is_array())
		{
			std::fill(std::begin(anInfo->mAllowedOnLevel), std::end(anInfo->mAllowedOnLevel), false);
			for (const Json &aLevel : aLevels)
			{
				int aNumber = aLevel.is_number() ? (int)aLevel.get<double>() : 0;
				if (aNumber >= 1 && aNumber <= 50)
				{
					anInfo->mAllowedOnLevel[aNumber - 1] = true;
				}
				else
				{
					Log(theWhere + ": \"allowedLevels\" holds level numbers from 1 to 50");
				}
			}
		}
		else
		{
			Log(theWhere + ": \"allowedLevels\" should be a list of level numbers");
		}
	}

	SetTypeStrings(anInfo->mTypeName, aBaseDef.mZombieName, anInfoJson, false);
	return anInfo->mType;
}

/// @brief Register a custom plant type from its file; its type, or -1
int RegisterCustomPlant(const Json &theRoot, const std::string &theTypeName, const KindInfo &theKind,
                        const std::string &theWhere)
{
	int aBase = ReadBase(theRoot, theKind, theWhere);
	if (aBase < 0)
		return -1;

	std::string anError;
	CustomTypes::PlantInfo *anInfo = CustomTypes::RegisterPlant(theTypeName, (SeedType)aBase,
	                                                            ReadInt(theRoot, "id", -1, theWhere), anError);
	if (anInfo == nullptr)
	{
		Log(theWhere + ": " + anError);
		return -1;
	}
	anInfo->mFile = theWhere;

	const PlantDefinition &aBaseDef = GetPlantDefinition((SeedType)aBase);
	Json anInfoJson = theRoot.contains("info") ? theRoot["info"] : Json::object();
	CheckInfoKeys(anInfoJson,
	              { "name", "tooltip", "description", "descriptionHeader", "reanim", "cost", "refreshTime",
	                "launchRate", "subClass" },
	              theWhere);

	PlantDefinition aDef = aBaseDef;
	aDef.mSeedType = anInfo->mType;
	ReadReanim(anInfoJson, aDef.mReanimationType, theWhere);
	aDef.mSeedCost = ReadInt(anInfoJson, "cost", aDef.mSeedCost, theWhere);
	aDef.mRefreshTime = ReadInt(anInfoJson, "refreshTime", aDef.mRefreshTime, theWhere);
	aDef.mLaunchRate = ReadInt(anInfoJson, "launchRate", aDef.mLaunchRate, theWhere);
	if (anInfoJson.contains("subClass"))
	{
		int64_t aSubClass;
		const Json &aValue = anInfoJson["subClass"];
		if (aValue.is_string() && EnumNames::ValueOf(EnumKey<PlantSubClass>(), aValue.get<std::string>().c_str(), aSubClass))
		{
			aDef.mSubClass = (PlantSubClass)aSubClass;
		}
		else
		{
			Log(theWhere + ": \"subClass\" should be SUBCLASS_NORMAL or SUBCLASS_SHOOTER");
		}
	}
	aDef.mPlantName = anInfo->mTypeName.c_str();
	anInfo->mDefinition = aDef;

	SetTypeStrings(anInfo->mTypeName, aBaseDef.mPlantName, anInfoJson, true);
	return anInfo->mType;
}
} // namespace

void ComponentDefinitions::Load(const std::string &theFolder)
{
	Store &aStore = GetStore();
	aStore.mLoaded = true;
	aStore.mDefinitions.clear();
	aStore.mMessages.clear();
	CustomTypes::BeginReload();

	for (EntityKind aKindId : kKinds)
	{
		const KindInfo &aKind = KindOf(aKindId);
		fs::path aFolder = fs::path(theFolder) / aKind.mFolder;
		std::error_code anError;
		if (!fs::is_directory(aFolder, anError))
			continue;

		std::vector<fs::path> aFiles;
		for (const fs::directory_entry &anEntry : fs::directory_iterator(aFolder, anError))
		{
			if (anEntry.is_regular_file() && anEntry.path().extension() == ".json")
			{
				aFiles.push_back(anEntry.path());
			}
		}
		std::sort(aFiles.begin(), aFiles.end());

		for (const fs::path &aPath : aFiles)
		{
			std::string aWhere = aPath.generic_string();
			std::ifstream aStream(aPath);
			Json aRoot = Json::parse(aStream, nullptr, false, true);
			if (aRoot.is_discarded() || !aRoot.is_object())
			{
				Log(aWhere + ": not valid JSON");
				continue;
			}
			if (!aRoot.contains(aKind.mKey) || !aRoot[aKind.mKey].is_string())
			{
				Log(aWhere + ": needs \"" + std::string(aKind.mKey) + "\" naming its type");
				continue;
			}
			std::string aTypeName = aRoot[aKind.mKey].get<std::string>();

			// A new type names what it's based on; a game type is redefined in place
			bool isCustom = aRoot.contains("basedOn");
			int aType = -1;
			if (isCustom)
			{
				if (aKindId == EntityKind::Zombie)
				{
					aType = RegisterCustomZombie(aRoot, aTypeName, aKind, aWhere);
				}
				else if (aKindId == EntityKind::Plant)
				{
					aType = RegisterCustomPlant(aRoot, aTypeName, aKind, aWhere);
				}
				else
				{
					Log(aWhere + ": new " + aKind.mKey + " types aren't supported yet");
				}
				if (aType < 0)
					continue;
			}
			else
			{
				int64_t aValue;
				if (!EnumNames::ValueOf(aKind.mEnumKey, aTypeName.c_str(), aValue) || aValue < 0 ||
				    aValue >= aKind.mCount)
				{
					Log(aWhere + ": unknown " + aKind.mKey + " type \"" + aTypeName +
					    "\" (a new type needs \"basedOn\")");
					continue;
				}
				aType = (int)aValue;
			}

			Json aComponents; // null: the base's components
			if (aRoot.contains("components"))
			{
				if (!aRoot["components"].is_array())
				{
					Log(aWhere + ": \"components\" should be a list");
					continue;
				}
				aComponents = aRoot["components"];
			}
			else if (!isCustom)
			{
				Log(aWhere + ": needs a \"components\" list");
				continue;
			}

			auto aKey = std::make_pair(aKindId, aType);
			if (aStore.mDefinitions.count(aKey))
			{
				Log(aWhere + ": " + aTypeName + " is already defined by " + aStore.mDefinitions[aKey].mFile);
				continue;
			}
			aStore.mDefinitions[aKey] = { aWhere, aComponents };

			// Report unknown components and fields once, at load time
			ComponentList aCheck;
			std::vector<std::string> aMessages;
			if (aComponents.is_array())
			{
				for (const Json &aComponent : aComponents)
				{
					if (Component *aBuilt = ComponentFromJson(aComponent, aWhere, aMessages))
					{
						aCheck.Add(aBuilt);
					}
				}
			}
			for (const std::string &aMessage : aMessages)
			{
				Log(aMessage);
			}
		}
	}
	CustomTypes::EndReload();
}

void ComponentDefinitions::EnsureLoaded()
{
	if (!GetStore().mLoaded)
	{
		Load();
	}
}

bool ComponentDefinitions::Build(EntityKind theKind, int theType, ComponentList &theOut)
{
	EnsureLoaded();
	Store &aStore = GetStore();
	auto anIt = aStore.mDefinitions.find(std::make_pair(theKind, theType));
	if (anIt == aStore.mDefinitions.end() || anIt->second.mComponents.is_null())
	{
		// A custom type without components of its own has its base's
		int aBase = theType;
		if (theKind == EntityKind::Zombie)
		{
			aBase = CustomTypes::PlaysAs((ZombieType)theType);
		}
		else if (theKind == EntityKind::Plant)
		{
			aBase = CustomTypes::PlaysAs((SeedType)theType);
		}
		return aBase != theType && Build(theKind, aBase, theOut);
	}

	std::vector<std::string> anIgnored; // reported when loading
	for (const Json &aComponent : anIt->second.mComponents)
	{
		if (Component *aBuilt = ComponentFromJson(aComponent, anIt->second.mFile, anIgnored))
		{
			theOut.Add(aBuilt);
		}
	}
	return true;
}

size_t ComponentDefinitions::GetCount()
{
	return GetStore().mDefinitions.size();
}

const std::vector<std::string> &ComponentDefinitions::GetMessages()
{
	return GetStore().mMessages;
}

// ─── Command-line tools ──────────────────────────────────────────────────────

namespace
{
std::string FileNameFor(const KindInfo &theKind, const char *theTypeName)
{
	std::string aName = theTypeName;
	size_t aPrefixLength = strlen(theKind.mPrefix);
	if (aName.compare(0, aPrefixLength, theKind.mPrefix) == 0)
	{
		aName = aName.substr(aPrefixLength);
	}
	for (char &c : aName)
	{
		c = (char)tolower((unsigned char)c);
	}
	return aName + ".json";
}

/// @brief Write every built-in type as a definition file
int ExportDefinitions(const std::string &theFolder, std::ostream &theLog)
{
	int aWritten = 0;
	for (EntityKind aKindId : kKinds)
	{
		const KindInfo &aKind = KindOf(aKindId);
		fs::path aFolder = fs::path(theFolder) / aKind.mFolder;
		fs::create_directories(aFolder);
		for (int aType = 0; aType < aKind.mCount; aType++)
		{
			const char *aTypeName = EnumNames::NameOf(aKind.mEnumKey, aType);
			if (aTypeName == nullptr)
				continue;

			ComponentList aComponents;
			BuildVanilla(aKindId, aType, aComponents);

			Json aRoot = Json::object();
			aRoot[aKind.mKey] = aTypeName;
			Json aList = Json::array();
			for (Component *aComponent : aComponents.Get())
			{
				aList.push_back(ComponentToJson(aComponent));
			}
			aRoot["components"] = aList;

			std::ofstream aStream(aFolder / FileNameFor(aKind, aTypeName), std::ios::binary);
			aStream << aRoot.dump(2) << "\n";
			aWritten++;
		}
	}
	theLog << "Exported " << aWritten << " definitions to " << theFolder << "\n";
	return 0;
}

std::vector<uint8_t> SaveBytes(Component *theComponent)
{
	std::vector<uint8_t> aBytes;
	ComponentArchive anArchive(aBytes, false);
	theComponent->Serialize(anArchive);
	return aBytes;
}

/// @brief Verify that every definition in the folder builds the same components as the built-in tables
int CheckDefinitions(const std::string &theFolder, std::ostream &theLog)
{
	ComponentDefinitions::Load(theFolder);
	for (const std::string &aMessage : ComponentDefinitions::GetMessages())
	{
		theLog << "message: " << aMessage << "\n";
	}

	int aChecked = 0;
	int aDifferent = 0;
	for (EntityKind aKindId : kKinds)
	{
		const KindInfo &aKind = KindOf(aKindId);
		for (int aType = 0; aType < aKind.mCount; aType++)
		{
			ComponentList aFromFile;
			if (!ComponentDefinitions::Build(aKindId, aType, aFromFile))
				continue;

			ComponentList aBuiltIn;
			BuildVanilla(aKindId, aType, aBuiltIn);
			aChecked++;

			const char *aTypeName = EnumNames::NameOf(aKind.mEnumKey, aType);
			std::string aProblem;
			const auto &aFileList = aFromFile.Get();
			const auto &aBuiltInList = aBuiltIn.Get();
			if (aFileList.size() != aBuiltInList.size())
			{
				aProblem = std::to_string(aFileList.size()) + " components instead of " +
				           std::to_string(aBuiltInList.size());
			}
			for (size_t i = 0; aProblem.empty() && i < aFileList.size(); i++)
			{
				if (strcmp(aFileList[i]->GetTypeName(), aBuiltInList[i]->GetTypeName()) != 0)
				{
					aProblem = std::string("component ") + std::to_string(i) + " is " + aFileList[i]->GetTypeName() +
					           " instead of " + aBuiltInList[i]->GetTypeName();
				}
				else if (SaveBytes(aFileList[i]) != SaveBytes(aBuiltInList[i]))
				{
					aProblem = std::string(aFileList[i]->GetTypeName()) + " has different values";
				}
			}

			if (!aProblem.empty())
			{
				aDifferent++;
				theLog << "DIFFERENT " << (aTypeName ? aTypeName : "?") << ": " << aProblem << "\n";
			}
		}
	}
	// The custom types: what they are based on and what they build
	for (ZombieType aType : CustomTypes::CustomZombieTypes())
	{
		CustomTypes::ZombieInfo *anInfo = CustomTypes::GetZombie(aType);
		ComponentList aComponents;
		ComponentDefinitions::Build(EntityKind::Zombie, aType, aComponents);
		theLog << "custom zombie " << anInfo->mTypeName << " = " << (int)aType << ", based on "
		       << EnumNames::NameOf(anInfo->mBase) << ", " << aComponents.Get().size() << " components\n";
	}
	for (SeedType aType : CustomTypes::CustomPlantTypes())
	{
		CustomTypes::PlantInfo *anInfo = CustomTypes::GetPlant(aType);
		ComponentList aComponents;
		ComponentDefinitions::Build(EntityKind::Plant, aType, aComponents);
		theLog << "custom plant " << anInfo->mTypeName << " = " << (int)aType << ", based on "
		       << EnumNames::NameOf(anInfo->mBase) << ", cost " << anInfo->mDefinition.mSeedCost << ", "
		       << aComponents.Get().size() << " components\n";
	}

	theLog << "Checked " << aChecked << " definitions: " << aDifferent << " different, "
	       << ComponentDefinitions::GetMessages().size() << " messages\n";
	return aDifferent == 0 && ComponentDefinitions::GetMessages().empty() ? 0 : 1;
}
} // namespace

bool ComponentDefinitions::RunCommandLineTool(int argc, char **argv, int &theExitCode)
{
	for (int i = 1; i < argc; i++)
	{
		std::string anArg = argv[i];
		bool isExport = anArg.rfind("-exportdefinitions=", 0) == 0;
		bool isCheck = anArg.rfind("-checkdefinitions=", 0) == 0;
		if (!isExport && !isCheck)
			continue;

		std::string aFolder = anArg.substr(anArg.find('=') + 1);
		// The log goes next to the folder, as the game may have no console
		std::ofstream aLog(fs::path(aFolder).parent_path() / "definitions_tool.log");
		theExitCode = isExport ? ExportDefinitions(aFolder, aLog) : CheckDefinitions(aFolder, aLog);
		return true;
	}
	return false;
}
