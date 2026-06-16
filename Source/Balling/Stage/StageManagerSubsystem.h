#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "StageManagerSubsystem.generated.h"

class APuzzleBall;

UCLASS()
class BALLING_API UStageManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// APuzzleBall::BeginPlay から呼ぶ。OnBallExited デリゲートをバインドする
	void RegisterBall(APuzzleBall* Ball);

	// ステージマップを開く（解放済みのステージのみ遷移可）
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void OpenStage(int32 StageIndex);

	UFUNCTION(BlueprintCallable, Category = "Stage")
	void OpenMainMenu();

	// 現在のマップ名からステージ番号を返す（ステージマップ以外は 0）
	UFUNCTION(BlueprintPure, Category = "Stage")
	int32 GetCurrentStageIndex() const;

	// Stage 1 は常に解放。Stage N は Stage N-1 クリア済みで解放
	UFUNCTION(BlueprintPure, Category = "Stage")
	bool IsStageUnlocked(int32 StageIndex) const;

	UFUNCTION(BlueprintPure, Category = "Stage")
	bool IsStageCleared(int32 StageIndex) const;

	static constexpr int32 TotalStages = 5;

private:
	UFUNCTION()
	void OnBallExited();

	void SaveProgress();
	void LoadProgress();

	// 0-based: ClearedStages[0] = Stage 1 クリア済み
	TArray<bool> ClearedStages;
};
