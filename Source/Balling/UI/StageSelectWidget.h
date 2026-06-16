#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StageSelectWidget.generated.h"

class UStageManagerSubsystem;

UCLASS()
class BALLING_API UStageSelectWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	// ステージボタンがクリックされたときに Blueprint から呼ぶ
	UFUNCTION(BlueprintCallable, Category = "StageSelect")
	void OnStageButtonClicked(int32 StageIndex);

	// 各ボタンの表示状態を Blueprint から取得する
	UFUNCTION(BlueprintPure, Category = "StageSelect")
	bool IsStageUnlocked(int32 StageIndex) const;

	UFUNCTION(BlueprintPure, Category = "StageSelect")
	bool IsStageCleared(int32 StageIndex) const;

	// ステージ総数（ボタン生成のループ上限に使う）
	UFUNCTION(BlueprintPure, Category = "StageSelect")
	int32 GetTotalStages() const;

private:
	UPROPERTY()
	TWeakObjectPtr<UStageManagerSubsystem> StageManager;
};
