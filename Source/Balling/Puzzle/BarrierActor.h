#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BarrierActor.generated.h"

class APuzzleActor;
class UProceduralMeshComponent;
class UMaterialInterface;

UENUM(BlueprintType)
enum class EBarrierType : uint8
{
	Fixed    UMETA(DisplayName = "Fixed"),
	Rotating UMETA(DisplayName = "Rotating"),
	Moving   UMETA(DisplayName = "Moving"),
};

UCLASS()
class BALLING_API ABarrierActor : public AActor
{
	GENERATED_BODY()

public:
	ABarrierActor();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Barrier")
	EBarrierType BarrierType = EBarrierType::Fixed;

	// ローカル X 方向の長さ
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Barrier|Shape", meta = (ClampMin = "1.0"))
	float Length = 200.f;

	// ローカル Y 方向の厚み
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Barrier|Shape", meta = (ClampMin = "1.0"))
	float Thickness = 10.f;

	// Z 方向の高さ（APuzzleActor::WallHeight と合わせること）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Barrier|Shape", meta = (ClampMin = "1.0"))
	float Height = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Barrier|Shape")
	TObjectPtr<UMaterialInterface> BarrierMaterial;

	// Rotating: パズル回転角度に対する相対回転の倍率
	// 1.0 = パズル回転と同速で追加回転、-1.0 = ワールド空間で静止
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Barrier|Rotating",
		meta = (EditCondition = "BarrierType == EBarrierType::Rotating"))
	float RotationMultiplier = 1.f;

	// Moving: 往復の始点（ローカル座標）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Barrier|Moving",
		meta = (EditCondition = "BarrierType == EBarrierType::Moving", MakeEditWidget = true))
	FVector MovePointA = FVector::ZeroVector;

	// Moving: 往復の終点（ローカル座標）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Barrier|Moving",
		meta = (EditCondition = "BarrierType == EBarrierType::Moving", MakeEditWidget = true))
	FVector MovePointB = FVector(100.f, 0.f, 0.f);

	// Moving: 往復速度（rad/s 相当）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Barrier|Moving",
		meta = (EditCondition = "BarrierType == EBarrierType::Moving", ClampMin = "0.1"))
	float MoveSpeed = 1.f;

	// Length / Thickness / Height を変更したときにメッシュを再生成する
	UFUNCTION(BlueprintCallable, Category = "Barrier")
	void BuildMesh();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Barrier")
	TObjectPtr<UProceduralMeshComponent> ProcMesh;

private:
	UPROPERTY()
	TWeakObjectPtr<APuzzleActor> OwnerPuzzle;

	float MoveElapsed = 0.f;
};
