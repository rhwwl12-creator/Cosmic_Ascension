// Cosmic Ascension - Player layer. Design: guideline.md section 3.3, 4.4, 5.9, 5.10, 7.5.
// Player/ is the command API used by UI; it may read state but must not include UUserWidget or AActor headers either.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/CosmicTypes.h"
#include "Events/CosmicPetitions.h"
#include "PlayerRoleSubsystem.generated.h"

/**
 * Ruler / Wanderer role switching, travel, encounter flow, the ruler command API, and the death, deposition and
 * succession pipeline. Holds no player or life dependent member: all of it lives in FGalaxyState (section 6.1).
 * So far: Install, the deposition pipeline in its Phase 2 form with BecomeWanderer, the ruler's megaproject command
 * (settlement included) and the answer to a petition card (MVP item 2).
 * The rest of the interface in section 7.5 arrives with Phase 4 and 4b.
 */
UCLASS()
class COSMIC_ASCENSION_API UPlayerRoleSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Starts a life on a new vessel (section 4.4, 7.5). The first life (NewGalaxy, Ruler) and every succession go through
	 * here. Resets the vessel, opens a FLifeRecord and, for a ruler, opens a reign in the three reign arrays.
	 * Writes the birth system into Knowing.KnownSystems and the birth civilization into Knowing.MetCivs, each once and
	 * without a line (section 8.1): a life knows where it was born. It never writes Knowing.FirstContactCivID: the
	 * people one is born into is not a contact. A life that starts as a wanderer (succession, Phase 4b) also knows the
	 * lane neighbours of its birth system in its galaxy row (NoteKnown, Lanes order, no line - section 4.4: the new eye).
	 * One draw from the sim stream per call, for the name of the vessel, made before the name bank is looked at.
	 * Refused with an error line when called from a handler of a Core Logic delegate (a handler may only read the state),
	 * and for the sanctuary (CosmicState::IsSanctuary - no vessel stands there, section 4.5 (1), 16 #81): a refused call
	 * changes nothing and draws nothing.
	 */
	void Install(FName SystemID, FName CivID, EPlayerRole Role);

	/**
	 * The deposition pipeline (section 5.10), run by the role-effect stage for Effect.DeposeRuler, in its Phase 2 form:
	 * tells CosmicRows::LogFallenMonarch (Severity 2), draws the regent profile (one number from the sim stream,
	 * CosmicState::PickAIProfile), BecomeWanderer with Reign.Deposed, then every people's ReverenceToPlayer times
	 * DT_Rules.ReverenceDecay (as after a death, section 4.4: no people crowns the fallen at once), and for the people it
	 * ruled Stability = min(Stability, PostRebellionStability) and ThroneLockedUntilTick = Tick + ThroneLockTicks. The
	 * vessel stays where it stands. The three screens, the digest and the exile encounter are Phase 4. Does nothing unless
	 * the vessel rules; refused with an error line when called from a handler of a Core Logic delegate.
	 */
	void DeposeRuler();

	/**
	 * Ruler to wanderer (section 5.10): Role = Wanderer, FormerCivID = the ruled people, RuledCivID cleared, the people
	 * AI-run with RegentProfile, StatusTags from ReignEndReason (Reign.Abdicated: Status.Abdicated, Reign.Deposed:
	 * Status.FallenMonarch, Reign.Died: none), the open reign closed with ReignEndReason, Knowing.PendingContactCivID
	 * cleared, and the system the vessel stands in and its lane neighbours in its galaxy row known (NoteKnown, Lanes
	 * order, no line -
	 * section 4.4: the new eye of a wanderer; meeting their owners is the next sight stage's). The regent record (notable
	 * hook 1) arrives with the abdication command of Phase 4. Does nothing unless the vessel rules; refused with an error
	 * line when called from a handler of a Core Logic delegate, and when the vessel stands in the sanctuary (a state no
	 * rule makes - Install refuses it).
	 */
	void BecomeWanderer(FGameplayTag RegentProfile, FGameplayTag ReignEndReason);

	/**
	 * The ruler's command (section 5.7, 7.5): starts RowName on SystemID for the people the vessel rules, through
	 * CosmicMegaprojects::Start - the same rules as the AI. Only while Embodied and Ruler, and only for a system of the
	 * ruled people - except project_settle, whose SystemID is the wild system to settle (CosmicMegaprojects::Settle, the
	 * rules of the AI's settlement; the system it starts to settle becomes known at once, through FollowUpKnowing).
	 * Between ticks: the project first progresses in the next tick's crisis stage. False
	 * and nothing changed otherwise; inside a handler of a Core Logic delegate an error line and nothing. No draw.
	 */
	bool StartMegaproject(FName SystemID, FName RowName);

	// The prototype's project key (Phase 3 replaces it with the panel): starts the first of CosmicMegaprojects::Choices
	// for the people the vessel rules (SinceTick = Calling.BornTick), or with none a settlement of the first of
	// CosmicMegaprojects::SettleTargets, under the rules of StartMegaproject. False when there is neither.
	bool StartNextMegaproject();

	/**
	 * The ruler's answer to the petition card that waits (section 5.9, 5.10; Knowing.PendingPetitionRow), between ticks:
	 * player input, of the same standing as a draw of the sim stream (section 9.2).
	 * Refused - nothing drawn, nothing changed - while no embodied ruler has a card, and with an error line from a
	 * handler of a Core Logic delegate, for a card whose row DT_Encounters lacks, for a choice out of range and for a
	 * cost the ruled people's stockpile cannot pay in full (the card greys such a choice).
	 * An answer taken draws one number 0..5 from the sim stream first, before the choice is read any further. Then: a
	 * card that no longer stands (CosmicPetitions::StillStands) is taken away (Withdrawn - no cost, no effect, no line);
	 * otherwise the cost is paid from the stockpile, the choice is checked (CosmicPetitions::Passes), and its OnSuccess
	 * or OnFailure effects run in place on the ruled people and the card's system (FollowUpKnowing after them), its
	 * SuccessText or FailureText is told (Log.Civ, Severity 1, no draw of the tell stream), the lines that follow are told
	 * (FollowUpNotes), the row is remembered in the life (LastChoiceRows) and the card is taken away. A choice with a
	 * NextEncounter puts that row as the next card on the same subject when its conditions hold (no draw). A DeposeRuler
	 * of an effect is recorded and resolved by the next tick's role-effect stage. The stream is synced at the end.
	 */
	FCosmicPetitionAnswer AnswerPetition(int32 ChoiceIndex);
};
