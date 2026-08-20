#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BallPlayerController.generated.h"

class APuzzleActor;
class APuzzleBall;
class UInputMappingContext;
class UInputAction;
class UHourglassWidget;
class UKeyIndicatorWidget;
class UResetWidget;
class UClearWidget;
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
	// 最大デット = RotationSpeed / GaugeDrainRate 度の CCW 回転でゲージが空になる
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Hourglass", meta = (ClampMin = "0.01"))
	float GaugeDrainRate = 0.5f;

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

	// WBP_KeyIndicatorWidget を割り当てる
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UKeyIndicatorWidget> KeyIndicatorWidgetClass;

	// WBP_ResetWidget を割り当てる
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UResetWidget> ResetWidgetClass;

	// WBP_ClearWidget を割り当てる
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UClearWidget> ClearWidgetClass;

public:
	// UResetWidget がフェード完了後に呼ぶ（直接呼ぶとフェードなしでリセット）
	void ResetStage();

private:
	void OnHourglassTriggered(const FInputActionValue& Value);
	void OnHourglassCompleted(const FInputActionValue& Value);
	void OnResetPressed();

	UFUNCTION()
	void OnGameClear();

	bool bGameCleared = false;

	UPROPERTY()
	TWeakObjectPtr<APuzzleActor> PuzzleActor;

	UPROPERTY()
	TWeakObjectPtr<APuzzleBall> PuzzleBall;

	UPROPERTY()
	TObjectPtr<UResetWidget> ResetWidgetInstance;

	UPROPERTY()
	TObjectPtr<UClearWidget> ClearWidgetInstance;

	// A 長押し中に積み上げた CCW 回転量（度）。離したときにこの分だけ CW で返す
	float RotationDebt = 0.f;

	// 新規プレスを開始した時点のパズル Yaw 角。返し終わりにここへスナップする
	float RotationAtPressStart = 0.f;
};
