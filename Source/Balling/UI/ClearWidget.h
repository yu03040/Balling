#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateTypes.h"
#include "ClearWidget.generated.h"

class UButton;
class UBorder;

UCLASS()
class BALLING_API UClearWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// クリア演出を開始する（BallPlayerController から呼ぶ）
	UFUNCTION(BlueprintCallable, Category = "Clear")
	void ShowClear();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual bool NativeSupportsKeyboardFocus() const override { return true; }

	// BP 側に同名ウィジェットが必要
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> FadeOverlay;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NextStageButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MainMenuButton;

private:
	UFUNCTION()
	void OnNextStagePressed();

	UFUNCTION()
	void OnMainMenuPressed();

	float FadeElapsed = 0.f;
	static constexpr float FadeDuration = 0.8f;
	bool bFading = false;

	// キーボードフォーカス時にホバーと同じ見た目を適用するためのスタイルキャッシュ
	FButtonStyle NextStageStyleNormal;
	FButtonStyle NextStageStyleFocused;
	FButtonStyle MainMenuStyleNormal;
	FButtonStyle MainMenuStyleFocused;
	bool bStylesSaved     = false;
	bool bNextFocusedLast = false;
	bool bMainFocusedLast = false;

	// 現在キーボードで選択中のボタン（nullptr = 未選択）
	UPROPERTY()
	TObjectPtr<UButton> SelectedButton;

	UFUNCTION()
	void OnNextStageHovered();
	UFUNCTION()
	void OnMainMenuHovered();
};
