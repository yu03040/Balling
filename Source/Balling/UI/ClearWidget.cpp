#include "ClearWidget.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Balling/Stage/StageManagerSubsystem.h"
#include "Framework/Application/SlateApplication.h"

void UClearWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (FadeOverlay)
	{
		FadeOverlay->SetRenderOpacity(0.f);
		FadeOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (NextStageButton)
	{
		NextStageButton->OnClicked.AddDynamic(this, &UClearWidget::OnNextStagePressed);
		NextStageButton->OnHovered.AddDynamic(this, &UClearWidget::OnNextStageHovered);
		NextStageButton->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.AddDynamic(this, &UClearWidget::OnMainMenuPressed);
		MainMenuButton->OnHovered.AddDynamic(this, &UClearWidget::OnMainMenuHovered);
		MainMenuButton->SetVisibility(ESlateVisibility::Collapsed);
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UClearWidget::ShowClear()
{
	bFading     = true;
	FadeElapsed = 0.f;
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UClearWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// ---- フェードアニメーション ----
	if (bFading)
	{
		FadeElapsed += InDeltaTime;
		const float Alpha   = FMath::Clamp(FadeElapsed / FadeDuration, 0.f, 1.f);
		const float Opacity = FMath::Lerp(0.f, 0.8f, Alpha);

		if (FadeOverlay)
		{
			FadeOverlay->SetRenderOpacity(Opacity);
		}

		if (Alpha >= 1.f)
		{
			bFading = false;

			// ボタンを表示（次ステージの有無を確認）
			if (UGameInstance* GI = GetGameInstance())
			{
				UStageManagerSubsystem* SM = GI->GetSubsystem<UStageManagerSubsystem>();
				if (SM)
				{
					const int32 Current  = SM->GetCurrentStageIndex();
					const bool  bHasNext = (Current > 0 && Current < UStageManagerSubsystem::TotalStages);

					if (NextStageButton)
					{
						NextStageButton->SetVisibility(
							bHasNext ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
					}
				}
			}

			if (MainMenuButton)
			{
				MainMenuButton->SetVisibility(ESlateVisibility::Visible);
			}

			// キーボードフォーカス時にホバーと同じ見た目にするスタイルを一度だけ生成する
			if (!bStylesSaved)
			{
				if (NextStageButton)
				{
					NextStageStyleNormal         = NextStageButton->GetStyle();
					NextStageStyleFocused        = NextStageStyleNormal;
					NextStageStyleFocused.Normal = NextStageStyleNormal.Hovered;
				}
				if (MainMenuButton)
				{
					MainMenuStyleNormal         = MainMenuButton->GetStyle();
					MainMenuStyleFocused        = MainMenuStyleNormal;
					MainMenuStyleFocused.Normal = MainMenuStyleNormal.Hovered;
				}
				bStylesSaved = true;
			}

			// 初期選択ボタンを設定し、スタイルを即時反映
			if (NextStageButton && NextStageButton->GetVisibility() == ESlateVisibility::Visible)
			{
				SelectedButton = NextStageButton;
			}
			else
			{
				SelectedButton = MainMenuButton;
			}
			if (NextStageButton) { NextStageButton->SetStyle(SelectedButton == NextStageButton ? NextStageStyleFocused : NextStageStyleNormal); }
			if (MainMenuButton)  { MainMenuButton->SetStyle(SelectedButton  == MainMenuButton  ? MainMenuStyleFocused  : MainMenuStyleNormal); }
			bNextFocusedLast = (SelectedButton == NextStageButton);
			bMainFocusedLast = (SelectedButton == MainMenuButton);

			// UClearWidget 自身にフォーカスを設定してキーボード入力を受け取る
			SetFocus();
		}
		return;
	}

	// ---- フェード完了後: SelectedButton に応じてスタイルを更新（変化時のみ）----
	if (!bStylesSaved) { return; }

	if (NextStageButton && NextStageButton->IsVisible())
	{
		const bool bFocused = (SelectedButton == NextStageButton);
		if (bFocused != bNextFocusedLast)
		{
			NextStageButton->SetStyle(bFocused ? NextStageStyleFocused : NextStageStyleNormal);
			bNextFocusedLast = bFocused;
		}
	}
	if (MainMenuButton && MainMenuButton->IsVisible())
	{
		const bool bFocused = (SelectedButton == MainMenuButton);
		if (bFocused != bMainFocusedLast)
		{
			MainMenuButton->SetStyle(bFocused ? MainMenuStyleFocused : MainMenuStyleNormal);
			bMainFocusedLast = bFocused;
		}
	}
}

FReply UClearWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (!bStylesSaved) { return Super::NativeOnKeyDown(InGeometry, InKeyEvent); }

	const FKey Key = InKeyEvent.GetKey();
	const bool bUp   = (Key == EKeys::Up   || Key == EKeys::Gamepad_DPad_Up);
	const bool bDown = (Key == EKeys::Down  || Key == EKeys::Gamepad_DPad_Down);
	const bool bOK   = (Key == EKeys::Enter || Key == EKeys::SpaceBar || Key == EKeys::Gamepad_FaceButton_Bottom);

	if (bUp || bDown)
	{
		const bool bNextVisible = NextStageButton && NextStageButton->IsVisible();
		if (bNextVisible)
		{
			if (bDown && SelectedButton == NextStageButton)   { SelectedButton = MainMenuButton; }
			else if (bUp && SelectedButton == MainMenuButton) { SelectedButton = NextStageButton; }
		}
		return FReply::Handled();
	}

	if (bOK)
	{
		if      (SelectedButton == NextStageButton) { OnNextStagePressed(); }
		else if (SelectedButton == MainMenuButton)  { OnMainMenuPressed(); }
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UClearWidget::OnNextStageHovered()
{
	if (bStylesSaved) { SelectedButton = NextStageButton; }
}

void UClearWidget::OnMainMenuHovered()
{
	if (bStylesSaved) { SelectedButton = MainMenuButton; }
}

void UClearWidget::OnNextStagePressed()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		UStageManagerSubsystem* SM = GI->GetSubsystem<UStageManagerSubsystem>();
		if (SM)
		{
			SM->OpenStage(SM->GetCurrentStageIndex() + 1);
		}
	}
}

void UClearWidget::OnMainMenuPressed()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		UStageManagerSubsystem* SM = GI->GetSubsystem<UStageManagerSubsystem>();
		if (SM)
		{
			SM->OpenMainMenu();
		}
	}
}
