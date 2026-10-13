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
#include "../Plant.h"

using Json = nlohmann::ordered_json;
namespace fs = std::filesystem;

// ─── JSON fields ─────────────────────────────────────────────────────────────

namespace
{
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
} // namespace

void ComponentDefinitions::Load(const std::string &theFolder)
{
	Store &aStore = GetStore();
	aStore.mLoaded = true;
	aStore.mDefinitions.clear();
	aStore.mMessages.clear();

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
			int64_t aType;
			if (!EnumNames::ValueOf(aKind.mEnumKey, aTypeName.c_str(), aType))
			{
				Log(aWhere + ": unknown " + aKind.mKey + " type \"" + aTypeName + "\"");
				continue;
			}
			if (!aRoot.contains("components") || !aRoot["components"].is_array())
			{
				Log(aWhere + ": needs a \"components\" list");
				continue;
			}

			auto aKey = std::make_pair(aKindId, (int)aType);
			if (aStore.mDefinitions.count(aKey))
			{
				Log(aWhere + ": " + aTypeName + " is already defined by " + aStore.mDefinitions[aKey].mFile);
				continue;
			}
			aStore.mDefinitions[aKey] = { aWhere, aRoot["components"] };

			// Report unknown components and fields once, at load time
			ComponentList aCheck;
			std::vector<std::string> aMessages;
			for (const Json &aComponent : aRoot["components"])
			{
				if (Component *aBuilt = ComponentFromJson(aComponent, aWhere, aMessages))
				{
					aCheck.Add(aBuilt);
				}
			}
			for (const std::string &aMessage : aMessages)
			{
				Log(aMessage);
			}
		}
	}
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
	if (anIt == aStore.mDefinitions.end())
		return false;

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
