#include "MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "InputCoreTypes.h"
#include "Engine/Texture2D.h"
#include "Styling/SlateBrush.h"

UMainMenuWidget::UMainMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

static UTexture2D* LoadTex(const TCHAR* Path)
{
	return LoadObject<UTexture2D>(nullptr, Path);
}

static void SetImageBrush(UImage* Img, UTexture2D* Tex, float W, float H)
{
	if (!Img || !Tex) return;
	FSlateBrush Brush;
	Brush.SetResourceObject(Tex);
	Brush.ImageSize = FVector2D(W, H);
	Brush.DrawAs = ESlateBrushDrawType::Image;
	Brush.TintColor = FSlateColor(FLinearColor::White);
	Img->SetBrush(Brush);
}

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnStartPressed);
		StartButton->OnHovered.AddDynamic(this, &UMainMenuWidget::OnStartHovered);
	}
	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnExitPressed);
		ExitButton->OnHovered.AddDynamic(this, &UMainMenuWidget::OnExitHovered);
	}

	bStartButtonFocused = true;
	UpdateButtonVisuals();
}

void UMainMenuWidget::FocusFirstButton()
{
	SetKeyboardFocus();
}

void UMainMenuWidget::UpdateButtonVisuals()
{
	UTexture2D* TexSelFrame    = LoadTex(TEXT("/Game/Assets/Textures/ui_frame/T_Selected_UI.T_Selected_UI"));
	UTexture2D* TexUnselFrame  = LoadTex(TEXT("/Game/Assets/Textures/ui_frame/T_Unselected_UI.T_Unselected_UI"));
	UTexture2D* TexStartSel    = LoadTex(TEXT("/Game/Assets/Textures/character/T_selected_text_new_game.T_selected_text_new_game"));
	UTexture2D* TexStartUnsel  = LoadTex(TEXT("/Game/Assets/Textures/character/T_unselected_text_new_game.T_unselected_text_new_game"));
	UTexture2D* TexExitSel     = LoadTex(TEXT("/Game/Assets/Textures/character/T_selected_text_exit.T_selected_text_exit"));
	UTexture2D* TexExitUnsel   = LoadTex(TEXT("/Game/Assets/Textures/character/T_unselected_text_exit.T_unselected_text_exit"));

	// Frame: texture aspect ratio ≈ 2.97:1 → display at 480×162
	SetImageBrush(StartButtonFrame, bStartButtonFocused ? TexSelFrame   : TexUnselFrame,  480.f, 162.f);
	SetImageBrush(ExitButtonFrame,  bStartButtonFocused ? TexUnselFrame : TexSelFrame,    480.f, 162.f);

	// NewGame text: aspect ratio ≈ 5.23:1 → display at 380×73
	SetImageBrush(StartButtonText,
		bStartButtonFocused ? TexStartSel   : TexStartUnsel,
		bStartButtonFocused ? 170.f         : 163.f,
		bStartButtonFocused ? 35.f          : 27.f);
	SetImageBrush(ExitButtonText,
		bStartButtonFocused ? TexExitUnsel  : TexExitSel,
		bStartButtonFocused ? 90.f          : 99.f,
		bStartButtonFocused ? 27.f          : 34.f);
}

FReply UMainMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (Key == EKeys::Up || Key == EKeys::Down || Key == EKeys::Tab)
	{
		bStartButtonFocused = !bStartButtonFocused;
		UpdateButtonVisuals();
		return FReply::Handled();
	}

	if (Key == EKeys::Enter || Key == EKeys::SpaceBar)
	{
		if (bStartButtonFocused) { OnStartPressed(); return FReply::Handled(); }
		else                     { OnExitPressed();  return FReply::Handled(); }
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UMainMenuWidget::OnStartHovered()
{
	bStartButtonFocused = true;
	UpdateButtonVisuals();
}

void UMainMenuWidget::OnExitHovered()
{
	bStartButtonFocused = false;
	UpdateButtonVisuals();
}

void UMainMenuWidget::OnStartPressed()
{
	UGameplayStatics::OpenLevel(this, TEXT("Stage_01"));
}

void UMainMenuWidget::OnExitPressed()
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}
