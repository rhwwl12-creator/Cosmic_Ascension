// Cosmic Ascension - UI layer (Presentation).

#include "UI/CosmicStabilityTrend.h"

#include "UI/CosmicNextRung.h"

int32 CosmicUI::StabilityPointsOf(TConstArrayView<float> Samples, float Now)
{
	if (Samples.Num() < StabilityTrendTicks)
	{
		return 0;
	}
	// The last window only, summed in double: the mean of up to StabilityTrendTicks values of 0..1.
	double Sum = 0.0;
	for (int32 At = Samples.Num() - StabilityTrendTicks; At < Samples.Num(); ++At)
	{
		Sum += Samples[At];
	}
	const float Mean = static_cast<float>(Sum / StabilityTrendTicks);
	return CosmicNextRung::WholePercent(Now) - CosmicNextRung::WholePercent(Mean);
}

ECosmicTrend CosmicUI::StabilityTrendOf(TConstArrayView<float> Samples, float Now)
{
	if (Samples.Num() < StabilityTrendTicks)
	{
		return ECosmicTrend::Unknown;
	}
	const int32 Points = StabilityPointsOf(Samples, Now);
	return Points >= 1 ? ECosmicTrend::Up : (Points <= -1 ? ECosmicTrend::Down : ECosmicTrend::Flat);
}
