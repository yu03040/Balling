#include "StageSelectWidget.h"
#include "Balling/Stage/StageManagerSubsystem.h"

void UStageSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = GetGameInstance())
	{
		StageManager = GI->GetSubsystem<UStageManagerSubsystem>();
	}
}

void UStageSelectWidget::OnStageButtonClicked(int32 StageIndex)
{
	if (StageManager.IsValid())
	{
		StageManager->OpenStage(StageIndex);
	}
}

bool UStageSelectWidget::IsStageUnlocked(int32 StageIndex) const
{
	return StageManager.IsValid() && StageManager->IsStageUnlocked(StageIndex);
}

bool UStageSelectWidget::IsStageCleared(int32 StageIndex) const
{
	return StageManager.IsValid() && StageManager->IsStageCleared(StageIndex);
}

int32 UStageSelectWidget::GetTotalStages() const
{
	return UStageManagerSubsystem::TotalStages;
}
