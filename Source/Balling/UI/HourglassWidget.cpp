#include "HourglassWidget.h"
#include "Balling/Core/BallPlayerController.h"

void UHourglassWidget::NativeConstruct()
{
	Super::NativeConstruct();
	OwnerController = Cast<ABallPlayerController>(GetOwningPlayer());
}

void UHourglassWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!OwnerController.IsValid())
	{
		return;
	}

	GaugeValue = OwnerController->HourglassGauge;

	// Visibility を変えると Tick が止まるため、透明度で表示を制御する
	const bool bVisible = OwnerController->bIsHoldingA || GaugeValue < 1.f;
	SetRenderOpacity(bVisible ? 1.f : 0.f);
}
