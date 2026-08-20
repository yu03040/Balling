#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KeyIndicatorWidget.generated.h"

class ABallPlayerController;

UCLASS()
class BALLING_API UKeyIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	// Blueprint でキー押下状態を参照できる
	UPROPERTY(BlueprintReadOnly, Category = "KeyIndicator")
	bool bIsKeyPressed = false;

	// Blueprint のバインド関数。押下中は明るい金色、離しているときは暗いグレー
	UFUNCTION(BlueprintPure, Category = "KeyIndicator")
	FLinearColor GetKeyColor() const;

private:
	UPROPERTY()
	TWeakObjectPtr<ABallPlayerController> OwnerController;
};
