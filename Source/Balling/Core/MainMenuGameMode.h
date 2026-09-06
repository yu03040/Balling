#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuGameMode.generated.h"

class UMainMenuWidget;

UCLASS()
class BALLING_API AMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMainMenuGameMode();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UMainMenuWidget> MainMenuWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UMainMenuWidget> MainMenuWidgetInstance;

	UFUNCTION()
	void SetInitialFocus();

	FTimerHandle FocusTimerHandle;
};
