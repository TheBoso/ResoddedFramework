// ImGui inspectors for the debugger window's Components section.
// Each component draws and edits its own fields; a new component only has to override DrawInspector.

#include "Components.h"

#if LAWN_DEBUG_TOOLS

#include <algorithm>
#include <imgui/imgui.h>
#include "../Board.h"
#include "../../LawnApp.h"

static const char *kProjectileNames[] = { "Pea",        "Snow Pea", "Cabbage", "Melon",  "Puff",       "Winter Melon",
                                          "Fireball",   "Star",     "Spike",   "Basketball", "Kernel", "Cob",
                                          "Butter",     "Zombie Pea" };
static_assert(sizeof(kProjectileNames) / sizeof(kProjectileNames[0]) == ProjectileType::NUM_PROJECTILES);

struct CoinChoice
{
	CoinType mType;
	const char *mName;
};
static const CoinChoice kCoinChoices[] = {
	{ CoinType::COIN_NONE, "None" },          { CoinType::COIN_SUN, "Sun" },       { CoinType::COIN_SMALLSUN, "Small Sun" },
	{ CoinType::COIN_LARGESUN, "Large Sun" }, { CoinType::COIN_SILVER, "Silver" }, { CoinType::COIN_GOLD, "Gold" },
	{ CoinType::COIN_DIAMOND, "Diamond" },
};

static bool CoinCombo(const char *theLabel, CoinType &theCoinType)
{
	const char *aPreview = "Other";
	for (const CoinChoice &aChoice : kCoinChoices)
	{
		if (aChoice.mType == theCoinType)
			aPreview = aChoice.mName;
	}

	bool aChanged = false;
	if (ImGui::BeginCombo(theLabel, aPreview))
	{
		for (const CoinChoice &aChoice : kCoinChoices)
		{
			if (ImGui::Selectable(aChoice.mName, aChoice.mType == theCoinType))
			{
				theCoinType = aChoice.mType;
				aChanged = true;
			}
		}
		ImGui::EndCombo();
	}
	return aChanged;
}

static bool TeamInput(const char *theLabel, int32_t &theTeam)
{
	const char *aName = theTeam == TEAM_PLANTS ? "Plants" : theTeam == TEAM_ZOMBIES ? "Zombies" : theTeam == TEAM_NONE ? "Neutral" : "Custom";
	ImGui::SetNextItemWidth(100);
	bool aChanged = ImGui::InputInt(theLabel, &theTeam);
	ImGui::SameLine();
	ImGui::TextDisabled("(%s)", aName);
	return aChanged;
}

static bool SeedCombo(const char *theLabel, SeedType &theSeedType)
{
	bool aChanged = false;
	const char *aPreview = theSeedType >= 0 && theSeedType < SeedType::NUM_SEED_TYPES ? gPlantDefs[theSeedType].mPlantName : "None";
	if (ImGui::BeginCombo(theLabel, aPreview))
	{
		for (int i = 0; i < (int)SeedType::NUM_SEED_TYPES; i++)
		{
			if (ImGui::Selectable(gPlantDefs[i].mPlantName, i == theSeedType))
			{
				theSeedType = (SeedType)i;
				aChanged = true;
			}
		}
		ImGui::EndCombo();
	}
	return aChanged;
}

void Component::DrawInspector()
{
	ImGui::TextDisabled("No editable fields");
}

static void HealthLayerInput(const char *theLabel, int32_t &theHealth, int32_t &theMaxHealth)
{
	ImGui::PushID(theLabel);
	ImGui::SetNextItemWidth(90);
	ImGui::InputInt(theLabel, &theHealth);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(90);
	ImGui::InputInt("max", &theMaxHealth);
	if (theMaxHealth > 0)
	{
		ImGui::SameLine();
		ImGui::ProgressBar(std::clamp((float)theHealth / theMaxHealth, 0.0f, 1.0f), ImVec2(80, 0));
		ImGui::SameLine();
		if (ImGui::SmallButton("Full"))
			theHealth = theMaxHealth;
	}
	ImGui::PopID();
}

void HealthComponent::DrawInspector()
{
	HealthLayerInput("Health", mHealth, mMaxHealth);
}

void ArmorComponent::DrawInspector()
{
	HealthLayerInput("Armour", mHealth, mMaxHealth);
	if (ImGui::Button("Break"))
	{
		Break(0U);
	}
}

void HelmComponent::DrawInspector()
{
	static const char *kHelmNames[] = { "None", "Traffic cone", "Bucket", "Football helmet", "Hard hat", "Red eyes",
	                                    "Headband", "Bobsled", "Wall-nut", "Tall-nut" };
	ImGui::Text("Type: %s", mType >= 0 && mType < (int)(sizeof(kHelmNames) / sizeof(kHelmNames[0])) ? kHelmNames[mType] : "?");
	ArmorComponent::DrawInspector();
}

void ShieldComponent::DrawInspector()
{
	static const char *kShieldNames[] = { "None", "Screen door", "Newspaper", "Ladder" };
	ImGui::Text("Type: %s", mType >= 0 && mType < (int)(sizeof(kShieldNames) / sizeof(kShieldNames[0])) ? kShieldNames[mType] : "?");
	ArmorComponent::DrawInspector();
}

void StatusEffectComponent::DrawInspector()
{
	ImGui::SetNextItemWidth(100);
	ImGui::InputInt("Ticks left", &mTicksLeft);
	ImGui::SameLine();
	ImGui::TextDisabled("(%.1fs)", mTicksLeft / 100.0f);
	if (ImGui::Button("Expire now"))
	{
		Expire();
	}
}

void TeamComponent::DrawInspector()
{
	TeamInput("Team", mTeam);
	ImGui::Text("Native team: %d%s", mNativeTeam, mTeam != mNativeTeam ? "  (converted)" : "");
	ImGui::TextDisabled("0 = plants, 1 = zombies, -1 = neutral, anything else = a custom team");
}

void FacingComponent::DrawInspector()
{
	ImGui::RadioButton("Left", &mDirection, -1);
	ImGui::SameLine();
	ImGui::RadioButton("Right", &mDirection, 1);
	ImGui::Text("Native: %s", mNativeDirection < 0 ? "left" : "right");
}

void HypnotizableComponent::DrawInspector()
{
	TeamInput("Hypnotized team", mHypnotizedTeam);
	ImGui::Checkbox("Turn around when hypnotized", &mFlipFacing);
}

void HypnotizeOnEatenComponent::DrawInspector()
{
	ImGui::Checkbox("Only while awake", &mRequiresAwake);
}

void ProducerComponent::DrawInspector()
{
	CoinCombo("Coin", mCoinType);
	ImGui::InputInt("Coins per drop", &mCoinsPerDrop);
	ImGui::InputInt("Rate (ticks)", &mRate);
	ImGui::InputInt("Countdown", &mCountdown);
	CoinCombo("Rare coin", mRareCoinType);
	ImGui::SliderInt("Rare chance %", &mRareChance, 0, 100);
	CoinCombo("Big Time bonus", mBigTimeBonusCoin);
	ImGui::Checkbox("Stops after final wave", &mStopsAfterFinalWave);
	if (ImGui::Button("Produce now"))
	{
		mCountdown = 1;
	}
}

void ShooterComponent::DrawInspector()
{
	int aProjectile = (int)mProjectileType;
	if (ImGui::Combo("Projectile", &aProjectile, kProjectileNames, (int)ProjectileType::NUM_PROJECTILES))
	{
		mProjectileType = (ProjectileType)aProjectile;
	}
	int anArc = (int)mArc;
	ImGui::RadioButton("Straight", &anArc, (int)ShotArc::Straight);
	ImGui::SameLine();
	ImGui::RadioButton("Lobbed", &anArc, (int)ShotArc::Lobbed);
	mArc = (ShotArc)anArc;

	ImGui::InputInt("Interval", &mInterval);
	ImGui::InputInt("Shots per volley", &mShotsPerVolley);
	ImGui::InputInt("Shot spacing", &mShotSpacing);
	ImGui::InputInt("Windup", &mWindup);
	ImGui::InputInt("Range (0 = row)", &mRange);
	ImGui::Checkbox("Needs a target", &mRequiresTarget);
	ImGui::DragFloat("Muzzle X", &mMuzzleX, 1.0f);
	ImGui::DragFloat("Muzzle Y", &mMuzzleY, 1.0f);
	ImGui::Text("Countdown %d  shots left %d", mCountdown, mShotsLeft);
	if (ImGui::Button("Fire now"))
	{
		mShotsLeft = std::max(mShotsPerVolley, 1);
		mShotTimer = 1;
	}
}

void PlantHeadComponent::DrawInspector()
{
	// Edit a copy: the head animation is built on attach, so changes apply by re-attaching
	static SeedType sSeedType = SeedType::SEED_NONE;
	static float sOffsetX, sOffsetY, sScale;
	static const PlantHeadComponent *sEditing = nullptr;
	if (sEditing != this)
	{
		sEditing = this;
		sSeedType = mSeedType;
		sOffsetX = mOffsetX;
		sOffsetY = mOffsetY;
		sScale = mScale;
	}

	SeedCombo("Plant", sSeedType);
	ImGui::DragFloat("Offset X", &sOffsetX, 0.5f);
	ImGui::DragFloat("Offset Y", &sOffsetY, 0.5f);
	ImGui::DragFloat("Scale", &sScale, 0.01f, 0.1f, 3.0f);
	if (ImGui::Button("Apply"))
	{
		// Replaces this component (deleted at the end of the tick, so returning from here is safe)
		if (GameObject *anOwner = GetOwnerObject())
		{
			sEditing = nullptr;
			anOwner->AddComponent(new PlantHeadComponent(sSeedType, sOffsetX, sOffsetY, sScale));
		}
	}
}

#else

void Component::DrawInspector()
{
}
void HealthComponent::DrawInspector()
{
}
void StatusEffectComponent::DrawInspector()
{
}
void ArmorComponent::DrawInspector()
{
}
void HelmComponent::DrawInspector()
{
}
void ShieldComponent::DrawInspector()
{
}
void TeamComponent::DrawInspector()
{
}
void FacingComponent::DrawInspector()
{
}
void HypnotizableComponent::DrawInspector()
{
}
void HypnotizeOnEatenComponent::DrawInspector()
{
}
void ProducerComponent::DrawInspector()
{
}
void ShooterComponent::DrawInspector()
{
}
void PlantHeadComponent::DrawInspector()
{
}

#endif
