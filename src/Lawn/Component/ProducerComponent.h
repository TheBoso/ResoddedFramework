#pragma once

#include "../../ConstEnums.h"
#include "Component.h"

/// @brief Periodically drops sun or coins (Sunflower, Twin Sunflower, Marigold...).
/// Works on any entity: give a zombie one and it drops sun for you as it walks.
///
///     plant->AddComponent<ProducerComponent>(CoinType::COIN_SUN, 2, 2500); // a Twin Sunflower
class ProducerComponent : public Component
{
	COMPONENT_TYPE(ProducerComponent)

	ProducerComponent() = default;
	ProducerComponent(CoinType theCoinType, int theCoinsPerDrop, int theRate)
	    : mCoinType(theCoinType), mCoinsPerDrop(theCoinsPerDrop), mRate(theRate)
	{
	}

	void OnAttach() override;
	void OnTick() override;
	void Serialize(ComponentArchive &theArchive) override;
	void DrawInspector() override;

	/// @brief Does it produce sun (Board::CountSunFlowers, sun-related achievements and challenges)?
	bool MakesSun() const;

	CoinType mCoinType = CoinType::COIN_SUN;
	int32_t mCoinsPerDrop = 1;
	/// Ticks between drops (randomized down by up to 150)
	int32_t mRate = 2500;
	CoinMotion mCoinMotion = CoinMotion::COIN_MOTION_FROM_PLANT;
	/// Chance in percent to drop mRareCoinType instead (Marigold: 10% gold)
	CoinType mRareCoinType = CoinType::COIN_NONE;
	int32_t mRareChance = 0;
	/// Extra coin per drop in the Big Time mini-game
	CoinType mBigTimeBonusCoin = CoinType::COIN_NONE;
	/// Stop producing a minute after the final wave starts (Marigold)
	bool mStopsAfterFinalWave = false;

	/// Ticks until the next drop. -1: pick a random start on attach.
	int32_t mCountdown = -1;
	/// Final wave grace period; -1 until the final wave starts
	int32_t mFinalWaveCountdown = -1;

  protected:
	/// @brief Is the owner able to produce right now? Override to add conditions.
	virtual bool CanProduce();
	/// @brief Which coin to drop. Override for state-dependent drops.
	virtual CoinType PickCoinType();
};

/// @brief Sun-shroom: small sun while small, nothing while growing, normal sun once grown.
/// (Growing itself still happens in Plant::UpdateSunShroom.)
class SunShroomProducerComponent : public ProducerComponent
{
	COMPONENT_SUBTYPE(SunShroomProducerComponent, ProducerComponent)

	SunShroomProducerComponent() = default;
	explicit SunShroomProducerComponent(int theRate) : ProducerComponent(CoinType::COIN_SUN, 1, theRate)
	{
	}

  protected:
	bool CanProduce() override;
	CoinType PickCoinType() override;
};
