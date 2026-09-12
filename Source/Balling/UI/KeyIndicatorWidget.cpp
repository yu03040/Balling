#include "KeyIndicatorWidget.h"
#include "Balling/Core/BallPlayerController.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Styling/SlateBrush.h"

void UKeyIndicatorWidget::NativeConstruct()
{
	Super::NativeConstruct();
	OwnerController = Cast<ABallPlayerController>(GetOwningPlayer());
	bWasKeyPressed = false;
	UpdateAButtonImage();
}

void UKeyIndicatorWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!OwnerController.IsValid())
	{
		return;
	}

	bIsKeyPressed = OwnerController->bIsHoldingA;

	if (bIsKeyPressed != bWasKeyPressed)
	{
		bWasKeyPressed = bIsKeyPressed;
		UpdateAButtonImage();
	}
}

FLinearColor UKeyIndicatorWidget::GetKeyColor() const
{
	return bIsKeyPressed
		? FLinearColor(1.0f, 0.85f, 0.3f, 1.0f)
		: FLinearColor(0.55f, 0.55f, 0.55f, 1.0f);
}

void UKeyIndicatorWidget::UpdateAButtonImage()
{
	if (!AButtonImage)
	{
		return;
	}

	const TCHAR* Path = bIsKeyPressed
		? TEXT("/Game/Assets/Textures/UI/T_A_Selected_Button.T_A_Selected_Button")
		: TEXT("/Game/Assets/Textures/UI/T_A_Unselected_Button.T_A_Unselected_Button");

	UTexture2D* Tex = LoadObject<UTexture2D>(nullptr, Path);
	if (!Tex)
	{
		return;
	}

	FSlateBrush Brush;
	Brush.SetResourceObject(Tex);
	Brush.ImageSize = FVector2D(300.f, 300.f);
	Brush.DrawAs = ESlateBrushDrawType::Image;
	Brush.TintColor = FSlateColor(FLinearColor::White);
	AButtonImage->SetBrush(Brush);
}
