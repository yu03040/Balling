#include "KeyIndicatorWidget.h"
#include "Balling/Core/BallPlayerController.h"

void UKeyIndicatorWidget::NativeConstruct()
{
	Super::NativeConstruct();
	OwnerController = Cast<ABallPlayerController>(GetOwningPlayer());
}

void UKeyIndicatorWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!OwnerController.IsValid())
	{
		return;
	}

	bIsKeyPressed = OwnerController->bIsHoldingA;
}

FLinearColor UKeyIndicatorWidget::GetKeyColor() const
{
	// 押下中: 明るい金色（ゲームテーマの青×金に合わせる）
	// 離し中: 暗いグレー
	return bIsKeyPressed
		? FLinearColor(1.0f, 0.85f, 0.3f, 1.0f)   // 押下: 明るい金色
		: FLinearColor(0.55f, 0.55f, 0.55f, 1.0f); // 離し: 中間グレー（暗背景でも視認できる）
}
