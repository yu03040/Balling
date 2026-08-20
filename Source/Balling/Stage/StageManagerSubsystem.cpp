#include "StageManagerSubsystem.h"
#include "BallingSaveGame.h"
#include "Balling/Puzzle/PuzzleBall.h"
#include "Kismet/GameplayStatics.h"

void UStageManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadProgress();
}

void UStageManagerSubsystem::RegisterBall(APuzzleBall* Ball)
{
	if (!Ball) { return; }
	Ball->OnBallExited.AddDynamic(this, &UStageManagerSubsystem::OnBallExited);
}

void UStageManagerSubsystem::OpenStage(int32 StageIndex)
{
	if (!IsStageUnlocked(StageIndex)) { return; }
	const FString MapName = FString::Printf(TEXT("Stage_%02d"), StageIndex);
	UGameplayStatics::OpenLevel(GetGameInstance(), FName(*MapName));
}

void UStageManagerSubsystem::OpenMainMenu()
{
	UGameplayStatics::OpenLevel(GetGameInstance(), FName(TEXT("MainMenu")));
}

int32 UStageManagerSubsystem::GetCurrentStageIndex() const
{
	const UWorld* World = GetGameInstance()->GetWorld();
	if (!World) { return 0; }

	// PIE プレフィックス（"UEDPIE_0_"）を除去してマップ名を取得
	FString MapName = World->GetMapName();
	const int32 UnderscoreIdx = MapName.Find(TEXT("_"), ESearchCase::IgnoreCase,
		ESearchDir::FromStart, MapName.Find(TEXT("UEDPIE")));
	if (MapName.StartsWith(TEXT("UEDPIE")))
	{
		// "UEDPIE_0_Stage_01" → 2番目のアンダースコア以降を取る
		int32 First = MapName.Find(TEXT("_"));
		int32 Second = MapName.Find(TEXT("_"), ESearchCase::IgnoreCase, ESearchDir::FromStart, First + 1);
		if (Second != INDEX_NONE)
		{
			MapName = MapName.Mid(Second + 1);
		}
	}

	for (int32 i = 1; i <= TotalStages; ++i)
	{
		if (MapName.Equals(FString::Printf(TEXT("Stage_%02d"), i), ESearchCase::IgnoreCase))
		{
			return i;
		}
	}
	return 0;
}

bool UStageManagerSubsystem::IsStageUnlocked(int32 StageIndex) const
{
	if (StageIndex == 1) { return true; }
	if (StageIndex < 1 || StageIndex > TotalStages) { return false; }
	return IsStageCleared(StageIndex - 1);
}

bool UStageManagerSubsystem::IsStageCleared(int32 StageIndex) const
{
	const int32 Idx = StageIndex - 1; // 0-based
	return ClearedStages.IsValidIndex(Idx) && ClearedStages[Idx];
}

void UStageManagerSubsystem::OnBallExited()
{
	const int32 Current = GetCurrentStageIndex();
	if (Current < 1 || Current > TotalStages) { return; }

	ClearedStages[Current - 1] = true;
	SaveProgress();
	// 遷移は UClearWidget のボタンが担う
}

void UStageManagerSubsystem::SaveProgress()
{
	UBallingSaveGame* Save = Cast<UBallingSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UBallingSaveGame::StaticClass()));
	Save->ClearedStages = ClearedStages;
	UGameplayStatics::SaveGameToSlot(Save, TEXT("BallingSave"), 0);
}

void UStageManagerSubsystem::LoadProgress()
{
	ClearedStages.Init(false, TotalStages);

	if (!UGameplayStatics::DoesSaveGameExist(TEXT("BallingSave"), 0)) { return; }

	UBallingSaveGame* Save = Cast<UBallingSaveGame>(
		UGameplayStatics::LoadGameFromSlot(TEXT("BallingSave"), 0));

	if (Save && Save->ClearedStages.Num() == TotalStages)
	{
		ClearedStages = Save->ClearedStages;
	}
}
