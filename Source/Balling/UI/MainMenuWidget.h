#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UImage;

UCLASS()
class BALLING_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMainMenuWidget(const FObjectInitializer& ObjectInitializer);
	void FocusFirstButton();

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> StartButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> ExitButton;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage> StartButtonFrame;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage> StartButtonText;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage> ExitButtonFrame;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage> ExitButtonText;

private:
	bool bStartButtonFocused = true;

	void UpdateButtonVisuals();

	UFUNCTION() void OnStartPressed();
	UFUNCTION() void OnExitPressed();
	UFUNCTION() void OnStartHovered();
	UFUNCTION() void OnExitHovered();
};
