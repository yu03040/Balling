#include "BallPlayerController.h"
#include "Balling/Puzzle/PuzzleActor.h"
#include "Balling/UI/HourglassWidget.h"
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

	InputComponent->BindKey(EKeys::R, IE_Pressed, this, &ABallPlayerController::ResetStage);
}

void ABallPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float NewSpeed = 0.f;

	if (bIsHoldingA)
	{
		if (HourglassGauge > 0.f)
		{
			const float Delta = RotationSpeed * DeltaTime;
			HourglassGauge  = FMath::Max(0.f, HourglassGauge - GaugeDrainRate * DeltaTime);
			RotationDebt   += Delta; // CCW 回転した分を借金として積む
			NewSpeed        = RotationSpeed; // CCW
		}
		// ゲージ枯渇中は回転しない（借金も増えない）
	}
	else
	{
		// ゲージ回復
		HourglassGauge = FMath::Min(1.f, HourglassGauge + GaugeRecoverRate * DeltaTime);

		// 借金が残っている間だけ CW で返す
		if (RotationDebt > 0.f)
		{
			const float PayBack = FMath::Min(RotationSpeed * DeltaTime, RotationDebt);
			RotationDebt -= PayBack;
			NewSpeed      = -RotationSpeed; // CW
		}
	}

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
	HourglassGauge = 1.f;
	bIsHoldingA    = false;
	RotationDebt   = 0.f;
}
