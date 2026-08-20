#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PuzzleActor.generated.h"

class UProceduralMeshComponent;
class UBoxComponent;
class UMaterialInterface;
class APuzzleBall;

// パズル外壁に開けるゴール出口の定義
USTRUCT(BlueprintType)
struct FExitGap
{
	GENERATED_BODY()

	// 出口を開けるエッジのインデックス（0 = 頂点0→頂点1 のエッジ）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExitGap")
	int32 EdgeIndex = 0;

	// エッジ上での穴の中心位置（0.0 = A端, 0.5 = 中央, 1.0 = B端）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExitGap",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CenterT = 0.5f;

	// 穴の幅（UU 単位）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExitGap",
		meta = (ClampMin = "1.0"))
	float GapWidth = 100.f;
};

UCLASS()
class BALLING_API APuzzleActor : public AActor
{
	GENERATED_BODY()

public:
	APuzzleActor();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	virtual void Tick(float DeltaTime) override;

	// 多角形の頂点リスト（ローカル XY 平面・CCW 順）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Shape")
	TArray<FVector2D> Vertices;

	// 外壁の高さ（Z 方向の押し出し量、UU 単位）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Shape", meta = (ClampMin = "1.0"))
	float WallHeight = 50.f;

	// 壁コリジョンボックスの厚み
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Shape", meta = (ClampMin = "1.0"))
	float WallThickness = 20.f;

	// 床面（Section 0）に適用するマテリアル
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Materials")
	TObjectPtr<UMaterialInterface> FloorMaterial;

	// 外壁外側・上面（Section 1）に適用するマテリアル
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Materials")
	TObjectPtr<UMaterialInterface> WallMaterial;

	// 外壁内側（Section 2）に適用するマテリアル
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Materials")
	TObjectPtr<UMaterialInterface> WallInnerMaterial;

	// true にすると床メッシュを非表示にして背景を透過させる
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Materials")
	bool bFloorTransparent = false;

	// ゴール出口の定義リスト（エッジ番号・中心位置・幅を指定）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Exit")
	TArray<FExitGap> ExitGaps;

	// ゲーム開始時のスピン角（度）。配置回転とは独立に出口の向きを調整できる
	// 正値 = CCW 方向、負値 = CW 方向
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Rotation")
	float InitialSpinAngleDeg = 0.f;

	// 頂点リストから ProceduralMesh とコリジョンを再生成する
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void BuildMesh();

	// 現在の Yaw 角度を返す（ABarrierActor が参照）
	UFUNCTION(BlueprintPure, Category = "Puzzle")
	float GetCurrentAngle() const;

	// 毎フレームの回転速度を設定する（PlayerController から呼ぶ）
	// 正値 = 反時計回り（CCW）、負値 = 時計回り（CW）
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void SetRotationSpeed(float DegreesPerSecond);

	// パズルをゲーム開始時の回転角度に戻す
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void ResetRotation();

	// 指定したスピン角へ即座にスナップする
	void SnapToAngle(float TargetSpinAngle);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puzzle")
	TObjectPtr<UProceduralMeshComponent> ProcMesh;

private:
	void BuildFloor();
	void BuildWalls();
	void BuildWallColliders();

	FVector2D ComputeCentroid() const;

	// ポリゴン内外判定（レイキャスト法）
	static bool IsPointInsidePolygon(const TArray<FVector2D>& Poly, const FVector2D& Point);

	float RotationSpeed = 0.f;
	bool bWasBallInside = true;

	FQuat  InitialQuat;       // BeginPlay 時点の回転（クォータニオン）
	float  SpinAngleDeg = 0.f; // 初期姿勢からの累積スピン角（度）

	UPROPERTY()
	TWeakObjectPtr<APuzzleBall> PuzzleBall;

	// 壁ごとに生成する BoxComponent（GC 対策で UPROPERTY 必須）
	UPROPERTY()
	TArray<TObjectPtr<UBoxComponent>> WallColliders;
};
