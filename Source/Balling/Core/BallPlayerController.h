#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BallPlayerController.generated.h"

class APuzzleActor;
class APuzzleBall;
class UInputMappingContext;
class UInputAction;
class UHourglassWidget;
struct FInputActionValue;

UCLASS()
class BALLING_API ABallPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABallPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

public:
	virtual void Tick(float DeltaTime) override;

	// CCW / CW それぞれの回転速度（度/秒）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Rotation", meta = (ClampMin = "1.0"))
	float RotationSpeed = 90.f;

	// A 長押し中のゲージ消費速度（1.0 = 1秒でゼロ）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Hourglass", meta = (ClampMin = "0.01"))
	float GaugeDrainRate = 0.5f;

	// A を離している間のゲージ回復速度（1.0 = 1秒で全回復）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Hourglass", meta = (ClampMin = "0.01"))
	float GaugeRecoverRate = 0.25f;

	// 砂時計ゲージ [0.0〜1.0]（UHourglassWidget が参照する）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puzzle|Hourglass")
	float HourglassGauge = 1.f;

	// A キーを押しているか（UHourglassWidget の表示切替に使用）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puzzle|Hourglass")
	bool bIsHoldingA = false;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Hourglass;

	// BP_HourglassWidget を割り当てる
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UHourglassWidget> HourglassWidgetClass;

private:
	void OnHourglassTriggered(const FInputActionValue& Value);
	void OnHourglassCompleted(const FInputActionValue& Value);
	void ResetStage();

	UPROPERTY()
	TWeakObjectPtr<APuzzleActor> PuzzleActor;

	UPROPERTY()
	TWeakObjectPtr<APuzzleBall> PuzzleBall;

	// A 長押し中に積み上げた CCW 回転量（度）。離したときにこの分だけ CW で返す
	float RotationDebt = 0.f;
};
