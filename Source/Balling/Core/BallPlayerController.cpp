#include "BallPlayerController.h"
#include "Balling/Puzzle/PuzzleActor.h"
#include "Balling/UI/HourglassWidget.h"
#include "Balling/UI/KeyIndicatorWidget.h"
#include "Balling/UI/ResetWidget.h"
#include "Balling/UI/ClearWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraActor.h"
#include "UObject/ConstructorHelpers.h"
#include "Balling/Puzzle/PuzzleBall.h"

ABallPlayerController::ABallPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMCFinder(
		TEXT("/Game/Input/IMC_Hourglass.IMC_Hourglass"));
	if (IMCFinder.Succeeded())
	{
		InputMappingContext = IMCFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IAFinder(
		TEXT("/Game/Input/IA_Hourglass.IA_Hourglass"));
	if (IAFinder.Succeeded())
	{
		IA_Hourglass = IAFinder.Object;
	}

	static ConstructorHelpers::FClassFinder<UHourglassWidget> WidgetFinder(
		TEXT("/Game/Blueprints/UI/WBP_HourglassWidget"));
	if (WidgetFinder.Succeeded())
	{
		HourglassWidgetClass = WidgetFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UKeyIndicatorWidget> KeyIndicatorFinder(
		TEXT("/Game/Blueprints/UI/WBP_KeyIndicatorWidget"));
	if (KeyIndicatorFinder.Succeeded())
	{
		KeyIndicatorWidgetClass = KeyIndicatorFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UResetWidget> ResetWidgetFinder(
		TEXT("/Game/Blueprints/UI/WBP_ResetWidget"));
	if (ResetWidgetFinder.Succeeded())
	{
		ResetWidgetClass = ResetWidgetFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UClearWidget> ClearWidgetFinder(
		TEXT("/Game/Blueprints/UI/WBP_StageClear"));
	if (ClearWidgetFinder.Succeeded())
	{
		ClearWidgetClass = ClearWidgetFinder.Class;
	}

}

void ABallPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (InputMappingContext)
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}

	AActor* Found = UGameplayStatics::GetActorOfClass(GetWorld(), APuzzleActor::StaticClass());
	PuzzleActor = Cast<APuzzleActor>(Found);

	AActor* FoundBall = UGameplayStatics::GetActorOfClass(GetWorld(), APuzzleBall::StaticClass());
	PuzzleBall = Cast<APuzzleBall>(FoundBall);

	// レベルに置いた CameraActor を視点として使用する
	if (ACameraActor* Cam = Cast<ACameraActor>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ACameraActor::StaticClass())))
	{
		SetViewTarget(Cam);
	}

	// キーボード入力をゲームに渡す（UIフォーカスやエディタへの横取りを防ぐ）
	SetInputMode(FInputModeGameOnly());

	if (HourglassWidgetClass)
	{
		UHourglassWidget* Widget = CreateWidget<UHourglassWidget>(this, HourglassWidgetClass);
		if (Widget)
		{
			Widget->AddToViewport();
			Widget->SetVisibility(ESlateVisibility::HitTestInvisible);
			Widget->SetRenderOpacity(0.f); // 初期は透明。NativeTick が表示を制御する
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("KeyIndicator: WidgetClass=%s"),
		KeyIndicatorWidgetClass ? *KeyIndicatorWidgetClass->GetName() : TEXT("NULL"));

	if (KeyIndicatorWidgetClass)
	{
		UKeyIndicatorWidget* KeyWidget = CreateWidget<UKeyIndicatorWidget>(this, KeyIndicatorWidgetClass);
		UE_LOG(LogTemp, Warning, TEXT("KeyIndicator: Widget=%s"),
			KeyWidget ? TEXT("Created") : TEXT("NULL"));
		if (KeyWidget)
		{
			KeyWidget->AddToViewport();
			KeyWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}

	if (ResetWidgetClass)
	{
		ResetWidgetInstance = CreateWidget<UResetWidget>(this, ResetWidgetClass);
		if (ResetWidgetInstance)
		{
			ResetWidgetInstance->AddToViewport();
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("BallPC: ClearWidgetClass=%s"),
		ClearWidgetClass ? *ClearWidgetClass->GetName() : TEXT("NULL"));

	if (ClearWidgetClass)
	{
		ClearWidgetInstance = CreateWidget<UClearWidget>(this, ClearWidgetClass);
		if (ClearWidgetInstance)
		{
			ClearWidgetInstance->AddToViewport(10); // ResetWidget より前面
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("BallPC: PuzzleBall=%s"),
		PuzzleBall.IsValid() ? *PuzzleBall->GetName() : TEXT("NULL"));

	if (PuzzleBall.IsValid())
	{
		PuzzleBall->OnBallExited.AddDynamic(this, &ABallPlayerController::OnGameClear);
		UE_LOG(LogTemp, Warning, TEXT("BallPC: OnBallExited bound"));
	}
}

void ABallPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (IA_Hourglass)
		{
			EIC->BindAction(IA_Hourglass, ETriggerEvent::Triggered, this,
				&ABallPlayerController::OnHourglassTriggered);
			EIC->BindAction(IA_Hourglass, ETriggerEvent::Completed, this,
				&ABallPlayerController::OnHourglassCompleted);
		}
	}

	InputComponent->BindKey(EKeys::R, IE_Pressed, this, &ABallPlayerController::OnResetPressed);
}

void ABallPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bGameCleared)
	{
		if (PuzzleActor.IsValid())
		{
			PuzzleActor->SetRotationSpeed(0.f);
		}
		return;
	}

	float NewSpeed = 0.f;

	if (bIsHoldingA)
	{
		if (HourglassGauge > 0.f)
		{
			// 新規プレス開始（借金ゼロの状態から押した）ときに基準角を記録する
			if (RotationDebt < KINDA_SMALL_NUMBER && PuzzleActor.IsValid())
			{
				RotationAtPressStart = PuzzleActor->GetCurrentAngle();
			}
			RotationDebt += RotationSpeed * DeltaTime;
			NewSpeed      = RotationSpeed; // CCW
		}
	}
	else
	{
		if (RotationDebt > 0.f)
		{
			const float PayBack = RotationSpeed * DeltaTime;
			if (PayBack >= RotationDebt)
			{
				// 最終フレーム: 基準角へ正確にスナップして誤差を消す
				RotationDebt = 0.f;
				if (PuzzleActor.IsValid())
				{
					PuzzleActor->SnapToAngle(RotationAtPressStart);
				}
			}
			else
			{
				RotationDebt -= PayBack;
				NewSpeed      = -RotationSpeed; // CW
			}
		}
	}

	// ゲージはデット量から直接計算。回転と常に同期する
	// MaxDebt = RotationSpeed / GaugeDrainRate 度のとき Gauge = 0.0
	const float MaxDebt = RotationSpeed / GaugeDrainRate;
	HourglassGauge = FMath::Max(0.f, 1.0f - RotationDebt / MaxDebt);

	if (PuzzleActor.IsValid())
	{
		PuzzleActor->SetRotationSpeed(NewSpeed);
	}
}

void ABallPlayerController::OnHourglassTriggered(const FInputActionValue& Value)
{
	bIsHoldingA = true;
}

void ABallPlayerController::OnHourglassCompleted(const FInputActionValue& Value)
{
	bIsHoldingA = false;
}

void ABallPlayerController::OnResetPressed()
{
	if (bGameCleared) { return; }

	if (ResetWidgetInstance)
	{
		// ウィジェット経由: フェード → リセット → フェード復帰
		ResetWidgetInstance->TriggerReset();
	}
	else
	{
		// ウィジェット未設定時のフォールバック（フェードなし即時リセット）
		ResetStage();
	}
}

void ABallPlayerController::OnGameClear()
{
	UE_LOG(LogTemp, Warning, TEXT("BallPC: OnGameClear called"));
	if (bGameCleared) { return; }
	bGameCleared = true;

	// 入力状態をリセットして回転を止める
	bIsHoldingA  = false;
	RotationDebt = 0.f;
	if (PuzzleActor.IsValid())
	{
		PuzzleActor->SetRotationSpeed(0.f);
	}

	// 入力モードを UI 専用に切り替え、UClearWidget にキーボードフォーカスを渡す
	if (ClearWidgetInstance)
	{
		FInputModeUIOnly UIMode;
		UIMode.SetWidgetToFocus(ClearWidgetInstance->TakeWidget());
		SetInputMode(UIMode);
		ClearWidgetInstance->ShowClear();
	}
}

void ABallPlayerController::ResetStage()
{
	if (PuzzleActor.IsValid())
	{
		PuzzleActor->ResetRotation();
	}
	if (PuzzleBall.IsValid())
	{
		PuzzleBall->ResetToInitial();
	}
	HourglassGauge       = 1.f;
	bIsHoldingA          = false;
	RotationDebt         = 0.f;
	RotationAtPressStart = 0.f;
}
