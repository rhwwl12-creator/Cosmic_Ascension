// Cosmic Ascension - Sim layer (Core Logic).

#include "Sim/CosmicResearch.h"

bool CosmicResearch::Focus(TMap<FName, float>& ResearchFocus, FName Field, float Share)
{
	// The field takes the share, the others share the rest evenly (section 5.4): the sum stays 1.
	if (!ResearchFocus.Contains(Field) || !(Share > 0.f && Share <= 1.f) || ResearchFocus.Num() < 2)
	{
		return false;
	}
	const float Rest = (1.f - Share) / static_cast<float>(ResearchFocus.Num() - 1);
	for (TPair<FName, float>& Each : ResearchFocus)
	{
		Each.Value = Each.Key == Field ? Share : Rest;
	}
	return true;
}

bool CosmicResearch::Even(TMap<FName, float>& ResearchFocus)
{
	if (ResearchFocus.IsEmpty())
	{
		return false;
	}
	const float Share = 1.f / static_cast<float>(ResearchFocus.Num());
	for (TPair<FName, float>& Each : ResearchFocus)
	{
		Each.Value = Share;
	}
	return true;
}

ECosmicFocusShape CosmicResearch::ShapeOf(const TMap<FName, float>& ResearchFocus, FName* OutField)
{
	if (OutField)
	{
		*OutField = NAME_None;
	}
	if (ResearchFocus.Num() < 2)
	{
		return ECosmicFocusShape::Mixed;
	}
	// A share that is not a number has no place in an order: no clean shape.
	float Highest = 0.f;
	bool bFirst = true;
	for (const TPair<FName, float>& Each : ResearchFocus)
	{
		if (FMath::IsNaN(Each.Value))
		{
			return ECosmicFocusShape::Mixed;
		}
		if (bFirst || Each.Value > Highest)
		{
			Highest = Each.Value;
			bFirst = false;
		}
	}
	// How many keys hold the highest share, and whether the others all hold one value.
	int32 AtHighest = 0;
	FName HighField = NAME_None;
	bool bRestSet = false;
	float Rest = 0.f;
	bool bRestEqual = true;
	for (const TPair<FName, float>& Each : ResearchFocus)
	{
		if (Each.Value == Highest)
		{
			++AtHighest;
			HighField = Each.Key;
		}
		else if (!bRestSet)
		{
			Rest = Each.Value;
			bRestSet = true;
		}
		else if (Each.Value != Rest)
		{
			bRestEqual = false;
		}
	}
	if (AtHighest == ResearchFocus.Num())
	{
		return ECosmicFocusShape::Even;
	}
	if (AtHighest == 1 && bRestEqual)
	{
		if (OutField)
		{
			*OutField = HighField;
		}
		return ECosmicFocusShape::Focused;
	}
	return ECosmicFocusShape::Mixed;
}
