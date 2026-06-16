#include "BallGameMode.h"
#include "BallPlayerController.h"

ABallGameMode::ABallGameMode()
{
	PlayerControllerClass = ABallPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
}
