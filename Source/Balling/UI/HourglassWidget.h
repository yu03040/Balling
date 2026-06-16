#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HourglassWidget.generated.h"

class ABallPlayerController;

UCLASS()
class BALLING_API UHourglassWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	// UMG の ProgressBar にバインドする（0.0 = 空、1.0 = 満タン）
	UPROPERTY(BlueprintReadOnly, Category = "Hourglass")
	float GaugeValue = 1.f;

private:
	UPROPERTY()
	TWeakObjectPtr<ABallPlayerController> OwnerController;
};
