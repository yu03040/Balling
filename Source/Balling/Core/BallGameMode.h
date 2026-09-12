#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BallGameMode.generated.h"

class UUserWidget;

UCLASS()
class BALLING_API ABallGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABallGameMode();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> BackgroundWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UUserWidget> BackgroundWidget;
};
