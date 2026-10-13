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
