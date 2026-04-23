#include "DisplayXRTestPlayerController.h"
#include "GameFramework/Pawn.h"
#include "EngineUtils.h"

void ADisplayXRTestPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Bind TAB to cycle rigs
	InputComponent->BindAction("CycleRig", IE_Pressed, this, &ADisplayXRTestPlayerController::CycleRig);

	// Also bind directly by key in case no action mapping exists
	InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &ADisplayXRTestPlayerController::CycleRig);

	// Shift+F1 toggles mouse capture in standalone -game (the editor's own
	// Shift+F1 capture-toggle only applies to the PIE viewport chain).
	FInputKeyBinding& Binding = InputComponent->BindKey(EKeys::F1, IE_Pressed,
		this, &ADisplayXRTestPlayerController::ToggleMouseCapture);
	Binding.Chord.bShift = true;
	Binding.bExecuteWhenPaused = true;
}

void ADisplayXRTestPlayerController::CycleRig()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Lazy-discover rigs on first TAB press
	if (Rigs.Num() == 0)
	{
		for (TActorIterator<APawn> It(World); It; ++It)
		{
			if (It->ActorHasTag(FName("DisplayXRRig")))
			{
				Rigs.Add(*It);
			}
		}
		// Find which rig we currently possess
		APawn* Current = GetPawn();
		for (int32 i = 0; i < Rigs.Num(); i++)
		{
			if (Rigs[i] == Current)
			{
				CurrentRigIndex = i;
				break;
			}
		}

		UE_LOG(LogTemp, Log, TEXT("DisplayXRTest: Found %d rigs tagged DisplayXRRig"), Rigs.Num());
	}

	if (Rigs.Num() < 2) return;

	// Cycle to next rig
	int32 NextIndex = (CurrentRigIndex + 1) % Rigs.Num();
	APawn* NextRig = Rigs[NextIndex];
	if (!NextRig) return;

	// Save current rig's controller rotation before switching
	APawn* CurrentRig = GetPawn();
	if (CurrentRig)
	{
		SavedRotations.Add(CurrentRig, GetControlRotation());
	}

	// Possess the next rig
	Possess(NextRig);
	CurrentRigIndex = NextIndex;

	// Restore saved rotation for the new rig (or keep zero if first time)
	if (FRotator* Saved = SavedRotations.Find(NextRig))
	{
		SetControlRotation(*Saved);
	}

	UE_LOG(LogTemp, Log, TEXT("DisplayXRTest: Switched to rig %d (%s)"),
		NextIndex, *NextRig->GetName());
}

void ADisplayXRTestPlayerController::ToggleMouseCapture()
{
	bMouseReleased = !bMouseReleased;

	if (bMouseReleased)
	{
		FInputModeGameAndUI Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		Mode.SetHideCursorDuringCapture(false);
		SetInputMode(Mode);
		bShowMouseCursor = true;
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
	}

	UE_LOG(LogTemp, Log, TEXT("DisplayXRTest: Mouse %s"),
		bMouseReleased ? TEXT("RELEASED") : TEXT("CAPTURED"));
}
