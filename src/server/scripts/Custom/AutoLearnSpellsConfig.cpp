#include "Config.h"

#include "AutoLearnSpells.h"

void LearnSpells::OnConfigLoad(bool /*reload*/)
{
    EnableGamemasters = 1;
    EnableClassSpells = 1;
    EnableTalentRanks = 0;
    EnableProficiencies = 1;
    EnableFromQuests = 1;
    EnableApprenticeRiding = 0;
    EnableJourneymanRiding = 0;
    EnableExpertRiding = 0;
    EnableArtisanRiding = 0;
    EnableColdWeatherFlying = 0;
}

void AddSC_AutoLeanrSpellsScripts()
{
    new LearnSpells();
}
