#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KeyIndicatorWidget.generated.h"

class ABallPlayerController;
class UImage;

UCLASS()
class BALLING_API UKeyIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> AButtonImage;

public:
	UPROPERTY(BlueprintReadOnly, Category = "KeyIndicator")
	bool bIsKeyPressed = false;

	UFUNCTION(BlueprintPure, Category = "KeyIndicator")
	FLinearColor GetKeyColor() const;

private:
	bool bWasKeyPressed = false;

	void UpdateAButtonImage();

	UPROPERTY()
	TWeakObjectPtr<ABallPlayerController> OwnerController;
};
