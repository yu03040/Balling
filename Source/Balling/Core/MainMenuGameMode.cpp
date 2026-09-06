#include "MainMenuGameMode.h"
#include "Balling/UI/MainMenuWidget.h"
#include "Blueprint/UserWidget.h"

AMainMenuGameMode::AMainMenuGameMode()
{
	DefaultPawnClass = nullptr;
}

void AMainMenuGameMode::SetInitialFocus()
{
	if (MainMenuWidgetInstance)
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			FInputModeUIOnly UIMode;
			UIMode.SetWidgetToFocus(MainMenuWidgetInstance->TakeWidget());
			UIMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(UIMode);
		}
		MainMenuWidgetInstance->FocusFirstButton();
	}
}

void AMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (MainMenuWidgetClass)
	{
		MainMenuWidgetInstance = CreateWidget<UMainMenuWidget>(GetWorld(), MainMenuWidgetClass);
		if (MainMenuWidgetInstance)
		{
			MainMenuWidgetInstance->AddToViewport();

			if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
			{
				FInputModeUIOnly UIMode;
				UIMode.SetWidgetToFocus(MainMenuWidgetInstance->TakeWidget());
				PC->SetInputMode(UIMode);
				PC->bShowMouseCursor = true;
			}

			GetWorld()->GetTimerManager().SetTimer(
				FocusTimerHandle,
				this, &AMainMenuGameMode::SetInitialFocus,
				0.3f, false);
		}
	}
}
