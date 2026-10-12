#pragma once

#include "Components.h"

class Plant;
class Zombie;
class Projectile;

/// @brief Team queries. Everything that used to compare `mMindControlled` flags should ask these instead.
/// Every query falls back to the vanilla rule when an entity has no TeamComponent (e.g. off-board previews).
namespace Teams
{
inline bool AreHostile(int theTeamA, int theTeamB)
{
	return theTeamA != theTeamB && theTeamA != TEAM_NONE && theTeamB != TEAM_NONE;
}

int Of(Plant *thePlant);
int Of(Zombie *theZombie);
int Of(Projectile *theProjectile);

void Set(Plant *thePlant, int theTeam);
void Set(Zombie *theZombie, int theTeam);
void Set(Projectile *theProjectile, int theTeam);

/// @brief +1 = facing right, -1 = facing left
int FacingOf(Plant *thePlant);
int FacingOf(Zombie *theZombie);

/// @brief The plant faces the other way than it spawned (hypnotized): its attack area and shots are mirrored
bool IsMirrored(Plant *thePlant);
/// @brief The plant fights for another team than it spawned on
bool IsConverted(Plant *thePlant);

/// @brief Can an attack from theAttackerTeam with these DamageRangeFlags hit the zombie?
/// Replaces Zombie::EffectedByDamage, whose DAMAGES_MINDCONTROLLED bit hardcoded the two vanilla teams.
bool CanAttack(int theAttackerTeam, Zombie *theZombie, unsigned int theDamageRangeFlags);
/// @brief Can an attack from theAttackerTeam hit the plant?
bool CanAttack(int theAttackerTeam, Plant *thePlant);
} // namespace Teams

namespace Hypnosis
{
/// @brief Does biting thePlant hypnotize theEater? (HypnotizeOnEaten on the plant, Hypnotizable on the zombie)
bool HypnotizesEater(Plant *thePlant, Zombie *theEater);

/// @brief Update components when a zombie gets hypnotized. Called from Zombie::StartMindControlled.
void OnZombieHypnotized(Zombie *theZombie);

/// @brief Hypnotize a plant: it joins its Hypnotizable team and (by default) turns around.
/// Zombies stop eating it, straight shooters fire the other way and their shots hit plants.
/// @return False if the plant is not Hypnotizable or already converted
bool HypnotizePlant(Plant *thePlant);
} // namespace Hypnosis

namespace Combat
{
/// @brief Damage any entity: zombies through their armour (Zombie::TakeDamage), plants through DamagePlant,
/// anything else with a HealthComponent directly. Every path tells the entity's components (OnDamaged).
/// @param theDamageFlags DamageFlags bits (DAMAGE_DOESNT_CAUSE_FLASH, DAMAGE_BYPASSES_SHIELD, ...)
void Damage(GameObject *theTarget, int theDamage, unsigned int theDamageFlags = 0U);

/// @brief Deal damage to a plant and tell its components (OnDamaged).
/// Plants die from Plant::Update / the eating zombie when their health runs out.
/// @param theFlash Flash the plant white like a projectile hit (bites flash through the chew animation instead)
void DamagePlant(Plant *thePlant, int theDamage, bool theFlash = true);
} // namespace Combat
