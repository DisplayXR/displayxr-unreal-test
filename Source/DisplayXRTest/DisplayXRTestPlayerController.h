#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DisplayXRTestPlayerController.generated.h"

/**
 * Player controller that switches between DisplayXR camera rigs with TAB.
 *
 * Place multiple Pawns in the level and tag them "DisplayXRRig".
 * TAB cycles possession through them. Each pawn keeps its own
 * position/rotation when unpossessed.
 */
UCLASS()
class ADisplayXRTestPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void SetupInputComponent() override;

private:
	void CycleRig();

	// All rigs found at BeginPlay, in discovery order
	UPROPERTY()
	TArray<TObjectPtr<APawn>> Rigs;

	// Per-rig saved controller rotation (preserved across TAB switches)
	TMap<APawn*, FRotator> SavedRotations;

	int32 CurrentRigIndex = 0;
};
