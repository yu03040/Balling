#include "ResetWidget.h"
#include "Balling/Core/BallPlayerController.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Styling/SlateBrush.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Texture2D.h"

UResetWidget::UResetWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> TexFinder(
		TEXT("/Game/Assets/Textures/UI/T_R_Button_UI"));
	if (TexFinder.Succeeded())
	{
		ResetButtonTexture = TexFinder.Object;
	}
}

void UResetWidget::NativeConstruct()
{
	Super::NativeConstruct();
	OwnerController = Cast<ABallPlayerController>(GetOwningPlayer());

	if (ResetButton)
	{
		ResetButton->OnClicked.AddDynamic(this, &UResetWidget::OnResetButtonClicked);
	}

	if (ResetButtonImage && ResetButtonTexture)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(ResetButtonTexture);
		Brush.DrawAs = ESlateBrushDrawType::Image;
		Brush.TintColor = FSlateColor(FLinearColor::White);
		ResetButtonImage->SetBrush(Brush);
	}

	if (FadeOverlay)
	{
		FadeOverlay->SetRenderOpacity(0.f);
	}
}

void UResetWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (FadeState == EFadeState::Idle)
	{
		return;
	}

	FadeElapsed += InDeltaTime;
	const float T = FMath::Clamp(FadeElapsed / FMath::Max(FadeDuration, KINDA_SMALL_NUMBER), 0.f, 1.f);

	if (FadeState == EFadeState::FadingOut)
	{
		// イーズアウト: 素早く暗転し始め、最後はゆっくり真っ黒に
		FadeAlpha = 1.f - FMath::Square(1.f - T);
		if (T >= 1.f)
		{
			FadeAlpha = 1.f;
			if (OwnerController.IsValid())
			{
				OwnerController->ResetStage();
			}
			FadeState   = EFadeState::FadingIn;
			FadeElapsed = 0.f;
		}
	}
	else // FadingIn
	{
		// イーズイン: ゆっくり明るくなり始め、最後はすっきり消える
		FadeAlpha = FMath::Square(1.f - T);
		if (T >= 1.f)
		{
			FadeAlpha = 0.f;
			FadeState = EFadeState::Idle;
		}
	}

	if (FadeOverlay)
	{
		FadeOverlay->SetRenderOpacity(FadeAlpha);
	}
}

void UResetWidget::TriggerReset()
{
	if (FadeState != EFadeState::Idle)
	{
		return; // 連打防止
	}
	FadeState   = EFadeState::FadingOut;
	FadeElapsed = 0.f;
}

void UResetWidget::OnResetButtonClicked()
{
	TriggerReset();
}
