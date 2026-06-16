#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "BallingSaveGame.generated.h"

UCLASS()
class BALLING_API UBallingSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// ClearedStages[0] = Stage 1 クリア済み、[1] = Stage 2 クリア済み、…
	UPROPERTY()
	TArray<bool> ClearedStages;
};
