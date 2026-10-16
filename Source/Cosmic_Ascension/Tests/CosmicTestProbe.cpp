// Cosmic Ascension - test probe.

#include "Tests/CosmicTestProbe.h"

void UCosmicTestProbe::HandleLogEntry(const FCosmicLogEntry& Entry)
{
	if (OnLog)
	{
		OnLog(Entry);
	}
}

void UCosmicTestProbe::HandleEventFired(const FCosmicEventNotice& Notice)
{
	if (OnEvent)
	{
		OnEvent(Notice);
	}
}

void UCosmicTestProbe::HandleTierAscended(EAscensionTier NewTier, FName FeatRow, EPlayerRole Path)
{
	if (OnTier)
	{
		OnTier(NewTier, FeatRow, Path);
	}
}
