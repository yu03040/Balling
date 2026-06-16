#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PuzzleBall.generated.h"

class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBallExited);

UCLASS()
class BALLING_API APuzzleBall : public AActor
{
	GENERATED_BODY()

public:
	APuzzleBall();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Physics", meta = (ClampMin = "0.0"))
	float LinearDamping = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Physics", meta = (ClampMin = "0.0"))
	float AngularDamping = 1.5f;

	UPROPERTY(BlueprintAssignable, Category = "Ball")
	FOnBallExited OnBallExited;

	UFUNCTION(BlueprintCallable, Category = "Ball")
	void ResetToInitial();

protected:
	// 物理シミュレーション・ビジュアル両用ルートコンポーネント
	// BP でスフィアメッシュを割り当て、カプセル簡略化コリジョンを追加すること
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

private:
	FTransform InitialTransform;
};
