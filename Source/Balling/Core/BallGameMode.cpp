#include "BallGameMode.h"
#include "BallPlayerController.h"
#include "Blueprint/UserWidget.h"

ABallGameMode::ABallGameMode()
{
	PlayerControllerClass = ABallPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
}

void ABallGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (BackgroundWidgetClass)
	{
		BackgroundWidget = CreateWidget<UUserWidget>(GetWorld(), BackgroundWidgetClass);
		if (BackgroundWidget)
		{
			BackgroundWidget->AddToViewport(-1);
		}
	}
}
