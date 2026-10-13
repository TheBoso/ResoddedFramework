#ifndef __ALMANACDIALOG_H__
#define __ALMANACDIALOG_H__

#include <vector>
#include "LawnDialog.h"

#define NUM_ALMANAC_SEEDS 49
#define NUM_ALMANAC_ZOMBIES 26

constexpr const float ALMANAC_PLANT_POSITION_X = 578.0f;
constexpr const float ALMANAC_PLANT_POSITION_Y = 140.0f;
constexpr const float ALMANAC_ZOMBIE_POSITION_X = 559.0f;
constexpr const float ALMANAC_ZOMBIE_POSITION_Y = 175.0f;
constexpr const int ALMANAC_INDEXPLANT_POSITION_X = 167;
constexpr const int ALMANAC_INDEXPLANT_POSITION_Y = 225;
constexpr const float ALMANAC_INDEXZOMBIE_POSITION_X = 535.0f;
constexpr const float ALMANAC_INDEXZOMBIE_POSITION_Y = 215.0f;
constexpr const float ALMANAC_PLANT_MAX_SPACE = 230.0f;
constexpr const float ALMANAC_ZOMBIE_MAX_SPACE = 162.0f;

class Plant;
class Zombie;
class LawnApp;
class GameButton;
class Reanimation;
class LawnScrollbar;
class AlmanacDialog : public LawnDialog
{
  private:
	enum
	{
		ALMANAC_BUTTON_CLOSE = 0,
		ALMANAC_BUTTON_PLANT = 1,
		ALMANAC_BUTTON_ZOMBIE = 2,
		ALMANAC_BUTTON_INDEX = 3,
		ALMANAC_BUTTON_MORE = 4
	};

  public:
	LawnApp *mApp;
	GameButton *mCloseButton;
	GameButton *mIndexButton;
	GameButton *mPlantButton;
	GameButton *mZombieButton;
	/// Switches between the game's plants or zombies and the custom ones (see Component/CustomTypes.h)
	GameButton *mMoreButton;
	/// The page of the plant and zombie grids on show: 0 the game's, 1 and up the custom ones
	int mPlantPage;
	int mZombiePage;
	AlmanacPage mOpenPage;
	Reanimation *mReanim[4];
	SeedType mSelectedSeed;
	ZombieType mSelectedZombie;
	Plant *mPlant;
	Zombie *mZombie;
	Zombie *mZombiePerfTest[400];

	LawnScrollbar *mDescriptionScrollbar;

  public:
	AlmanacDialog(LawnApp *theApp);
	virtual ~AlmanacDialog();

	void ClearPlantsAndZombies();
	virtual void RemovedFromManager(WidgetManager *theWidgetManager);
	void SetupPlant();
	void SetupZombie();
	void SetPage(AlmanacPage thePage);
	virtual void Update();
	void DrawIndex(Graphics *g);
	void DrawPlants(Graphics *g);
	void DrawZombies(Graphics *g);
	virtual void Draw(Graphics *g);
	void GetSeedPosition(SeedType theSeedType, int &x, int &y);
	/// @brief The plants in the grid on show
	std::vector<SeedType> PlantsOnPage();
	int NumPlantPages();
	int NumZombiePages();
	/// @brief Show or hide the button for the next page, with the right label
	void UpdateMoreButton();
	SeedType SeedHitTest(int x, int y);
	bool ZombieHasSilhouette(ZombieType theZombieType);
	bool ZombieIsShown(ZombieType theZombieType);
	bool ZombieHasDescription(ZombieType theZombieType);
	void GetZombiePosition(ZombieType theZombieType, int &x, int &y);
	ZombieType ZombieHitTest(int x, int y);
	virtual void MouseUp(int x, int y, int theClickCount);
	virtual void MouseDown(int x, int y, int theClickCount);
	virtual void KeyChar(SexyChar theChar)
	{
	}

	virtual void MouseWheel(int theDelta);
	/// @brief The zombie in a window of the grid on show
	ZombieType GetZombieType(int theIndex);
	void ShowPlant(SeedType theSeedType);
	void ShowZombie(ZombieType theZombieType);
};
extern bool gZombieDefeated[MAX_ZOMBIE_TYPES];

void AlmanacInitForPlayer();
void AlmanacPlayerDefeatedZombie(ZombieType theZombieType);

#endif
