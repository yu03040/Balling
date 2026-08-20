#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ResetWidget.generated.h"

class ABallPlayerController;
class UBorder;
class UButton;

UCLASS()
class BALLING_API UResetWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// R キーまたはボタンクリック共通エントリ（フェード進行中は無視）
	UFUNCTION(BlueprintCallable, Category = "Reset")
	void TriggerReset();

	// フェード所要時間（秒）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reset")
	float FadeDuration = 0.3f;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// WBP 内に "FadeOverlay" という名前の Border ウィジェットが必要
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UBorder> FadeOverlay;

	// WBP 内に "ResetButton" という名前の Button ウィジェットが必要
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> ResetButton;

private:
	UFUNCTION()
	void OnResetButtonClicked();

	enum class EFadeState : uint8 { Idle, FadingOut, FadingIn };

	EFadeState FadeState   = EFadeState::Idle;
	float      FadeElapsed = 0.f;
	float      FadeAlpha   = 0.f;

	UPROPERTY()
	TWeakObjectPtr<ABallPlayerController> OwnerController;
};
