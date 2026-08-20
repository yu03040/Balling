#include "PuzzleActor.h"
#include "PuzzleBall.h"
#include "ProceduralMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

APuzzleActor::APuzzleActor()
{
	PrimaryActorTick.bCanEverTick = true;

	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
	SetRootComponent(ProcMesh);
	// 視覚専用。コリジョンは BuildWallColliders() の BoxComponent が担う
	ProcMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProcMesh->SetCastShadow(false);
	ProcMesh->bAffectDistanceFieldLighting = false;
	ProcMesh->bVisibleInRayTracing = false;

	// デフォルト形状: 正六角形（Stage 1 と同形）、半径 300 UU
	const int32 Sides = 6;
	const float Radius = 300.f;
	Vertices.Reserve(Sides);
	for (int32 i = 0; i < Sides; ++i)
	{
		// 90° 始まりで CCW 順に並べる（頂点が真上から始まる）
		const float Angle = FMath::DegreesToRadians(60.f * i + 90.f);
		Vertices.Add(FVector2D(Radius * FMath::Cos(Angle), Radius * FMath::Sin(Angle)));
	}
}

void APuzzleActor::BeginPlay()
{
	Super::BeginPlay();
	BuildWallColliders();

	InitialQuat  = GetActorTransform().GetRotation();
	SpinAngleDeg = InitialSpinAngleDeg;

	// 初期スピン角を視覚に反映する
	if (!FMath::IsNearlyZero(SpinAngleDeg))
	{
		const FQuat SpinQuat(FVector::UpVector, FMath::DegreesToRadians(SpinAngleDeg));
		SetActorRotation((InitialQuat * SpinQuat).Rotator());
	}

	AActor* Found = UGameplayStatics::GetActorOfClass(GetWorld(), APuzzleBall::StaticClass());
	PuzzleBall = Cast<APuzzleBall>(Found);
	bWasBallInside = true;
}

void APuzzleActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	BuildMesh();
}

void APuzzleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!FMath::IsNearlyZero(RotationSpeed))
	{
		SpinAngleDeg += RotationSpeed * DeltaTime;
		const FQuat SpinQuat(FVector::UpVector, FMath::DegreesToRadians(SpinAngleDeg));
		SetActorRotation((InitialQuat * SpinQuat).Rotator());
	}

	// ボールがパズル外枠を出たか毎フレーム判定する
	if (PuzzleBall.IsValid() && Vertices.Num() >= 3)
	{
		// ボール位置をパズルのローカル空間に変換して XY 平面で判定
		const FVector LocalPos = GetActorTransform().InverseTransformPosition(
			PuzzleBall->GetActorLocation());
		const bool bIsInside = IsPointInsidePolygon(Vertices, FVector2D(LocalPos.X, LocalPos.Y));

		if (bWasBallInside && !bIsInside)
		{
			PuzzleBall->OnBallExited.Broadcast();
		}
		bWasBallInside = bIsInside;
	}
}

bool APuzzleActor::IsPointInsidePolygon(const TArray<FVector2D>& Poly, const FVector2D& Point)
{
	bool bInside = false;
	const int32 N = Poly.Num();
	for (int32 i = 0, j = N - 1; i < N; j = i++)
	{
		const FVector2D& Vi = Poly[i];
		const FVector2D& Vj = Poly[j];
		if (((Vi.Y > Point.Y) != (Vj.Y > Point.Y)) &&
			(Point.X < (Vj.X - Vi.X) * (Point.Y - Vi.Y) / (Vj.Y - Vi.Y) + Vi.X))
		{
			bInside = !bInside;
		}
	}
	return bInside;
}

void APuzzleActor::BuildMesh()
{
	if (Vertices.Num() < 3)
	{
		return;
	}

	ProcMesh->ClearAllMeshSections();
	BuildFloor();
	BuildWalls();

	ProcMesh->SetMaterial(0, FloorMaterial);
	ProcMesh->SetMaterial(1, WallMaterial);
	ProcMesh->SetMaterial(2, WallInnerMaterial);
	ProcMesh->SetMeshSectionVisible(0, !bFloorTransparent);
}

float APuzzleActor::GetCurrentAngle() const
{
	return SpinAngleDeg;
}

void APuzzleActor::SetRotationSpeed(float DegreesPerSecond)
{
	RotationSpeed = DegreesPerSecond;
}

void APuzzleActor::SnapToAngle(float TargetSpinAngle)
{
	RotationSpeed = 0.f;
	SpinAngleDeg  = TargetSpinAngle;
	const FQuat SpinQuat(FVector::UpVector, FMath::DegreesToRadians(SpinAngleDeg));
	SetActorRotation((InitialQuat * SpinQuat).Rotator());
}

void APuzzleActor::ResetRotation()
{
	RotationSpeed = 0.f;
	SpinAngleDeg  = InitialSpinAngleDeg;
	const FQuat SpinQuat(FVector::UpVector, FMath::DegreesToRadians(SpinAngleDeg));
	SetActorRotation((InitialQuat * SpinQuat).Rotator());
	bWasBallInside = true;
}

FVector2D APuzzleActor::ComputeCentroid() const
{
	FVector2D Sum = FVector2D::ZeroVector;
	for (const FVector2D& V : Vertices)
	{
		Sum += V;
	}
	return Sum / static_cast<float>(Vertices.Num());
}

void APuzzleActor::BuildFloor()
{
	const int32 N = Vertices.Num();
	const FVector2D Centroid = ComputeCentroid();

	// UV 正規化用のバウンディングボックスを計算する
	FBox2D Bounds(ForceInit);
	for (const FVector2D& V : Vertices)
	{
		Bounds += V;
	}
	const FVector2D BoundsSize = Bounds.GetSize();
	const float UVScale = FMath::Max(FMath::Max(BoundsSize.X, BoundsSize.Y), 1.f);

	TArray<FVector> Verts;
	TArray<int32> Tris;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	Verts.Reserve(N + 1);
	Tris.Reserve(N * 3);
	Normals.Reserve(N + 1);
	UVs.Reserve(N + 1);

	// Index 0: 重心
	Verts.Add(FVector(Centroid.X, Centroid.Y, 0.f));
	Normals.Add(FVector::UpVector);
	UVs.Add((Centroid - Bounds.Min) / UVScale);

	// Index 1〜N: 外周頂点
	for (int32 i = 0; i < N; ++i)
	{
		Verts.Add(FVector(Vertices[i].X, Vertices[i].Y, 0.f));
		Normals.Add(FVector::UpVector);
		UVs.Add((Vertices[i] - Bounds.Min) / UVScale);
	}

	// ファン三角形（+Z から見て CW = UE のフロントフェイス）
	for (int32 i = 0; i < N; ++i)
	{
		const int32 Next = (i + 1) % N;
		Tris.Add(0);
		Tris.Add(Next + 1);
		Tris.Add(i + 1);
	}

	// 床面はコリジョンなし（視覚のみ）
	// 床コリジョンは別途フラットな BoxComponent で代替する
	ProcMesh->CreateMeshSection(0, Verts, Tris, Normals, UVs,
		TArray<FColor>(), TArray<FProcMeshTangent>(), /*bCreateCollision=*/false);
}

void APuzzleActor::BuildWalls()
{
	const int32 N = Vertices.Num();

	// Section 1: 外側面 + 上面（WallMaterial）
	TArray<FVector>  OuterVerts,  InnerVerts;
	TArray<int32>    OuterTris,   InnerTris;
	TArray<FVector>  OuterNormals, InnerNormals;
	TArray<FVector2D> OuterUVs,   InnerUVs;

	auto AddFaceTo = [](
		TArray<FVector>& Verts, TArray<int32>& Tris,
		TArray<FVector>& Normals, TArray<FVector2D>& UVs,
		const FVector& A, const FVector& B, const FVector& C, const FVector& D, const FVector& Normal)
	{
		const int32 Base = Verts.Num();
		Verts.Add(A); Verts.Add(B); Verts.Add(C); Verts.Add(D);
		for (int32 k = 0; k < 4; ++k) { Normals.Add(Normal); }
		UVs.Add({0.f,0.f}); UVs.Add({1.f,0.f}); UVs.Add({1.f,1.f}); UVs.Add({0.f,1.f});
		Tris.Add(Base+0); Tris.Add(Base+2); Tris.Add(Base+1);
		Tris.Add(Base+0); Tris.Add(Base+3); Tris.Add(Base+2);
	};

	for (int32 i = 0; i < N; ++i)
	{
		const int32 Next = (i + 1) % N;
		const FVector2D A = Vertices[i];
		const FVector2D B = Vertices[Next];

		const FVector2D Edge      = B - A;
		const float     EdgeLength = Edge.Size();
		const FVector2D EdgeDir   = Edge / EdgeLength;
		const FVector2D InwardNormal2D (-EdgeDir.Y,  EdgeDir.X);
		const FVector2D OutwardNormal2D( EdgeDir.Y, -EdgeDir.X);
		const FVector InN ( InwardNormal2D.X,  InwardNormal2D.Y, 0.f);
		const FVector OutN(OutwardNormal2D.X, OutwardNormal2D.Y, 0.f);

		TArray<TPair<float,float>> Segs;
		const FExitGap* Gap = ExitGaps.FindByPredicate(
			[i](const FExitGap& G){ return G.EdgeIndex == i; });
		if (Gap)
		{
			const float GapCenter = Gap->CenterT * EdgeLength;
			const float HalfGap  = Gap->GapWidth * 0.5f;
			const float Seg1End  = FMath::Clamp(GapCenter - HalfGap, 0.f, EdgeLength);
			const float Seg2Start= FMath::Clamp(GapCenter + HalfGap, 0.f, EdgeLength);
			if (Seg1End   > 1.f)              Segs.Add({0.f,       Seg1End  });
			if (Seg2Start < EdgeLength - 1.f) Segs.Add({Seg2Start, EdgeLength});
		}
		else
		{
			Segs.Add({0.f, EdgeLength});
		}

		for (const TPair<float,float>& Seg : Segs)
		{
			const FVector2D SA  = A + EdgeDir * Seg.Key;
			const FVector2D SB  = A + EdgeDir * Seg.Value;
			const FVector2D SAI = SA + InwardNormal2D * WallThickness;
			const FVector2D SBI = SB + InwardNormal2D * WallThickness;

			const FVector Ao (SA.X,  SA.Y,  0.f       );
			const FVector Bo (SB.X,  SB.Y,  0.f       );
			const FVector AoT(SA.X,  SA.Y,  WallHeight);
			const FVector BoT(SB.X,  SB.Y,  WallHeight);
			const FVector Ai (SAI.X, SAI.Y, 0.f       );
			const FVector Bi (SBI.X, SBI.Y, 0.f       );
			const FVector AiT(SAI.X, SAI.Y, WallHeight);
			const FVector BiT(SBI.X, SBI.Y, WallHeight);

			// Section 2: 内面（WallInnerMaterial）
			AddFaceTo(InnerVerts, InnerTris, InnerNormals, InnerUVs,
				AiT, BiT, Bi, Ai, InN);

			// Section 1: 外面 + 上面（WallMaterial）
			AddFaceTo(OuterVerts, OuterTris, OuterNormals, OuterUVs,
				Bo,  Ao,  AoT, BoT, OutN);
			AddFaceTo(OuterVerts, OuterTris, OuterNormals, OuterUVs,
				AoT, BoT, BiT, AiT, FVector::UpVector);
		}
	}

	ProcMesh->CreateMeshSection(1, OuterVerts, OuterTris, OuterNormals, OuterUVs,
		TArray<FColor>(), TArray<FProcMeshTangent>(), /*bCreateCollision=*/false);
	ProcMesh->CreateMeshSection(2, InnerVerts, InnerTris, InnerNormals, InnerUVs,
		TArray<FColor>(), TArray<FProcMeshTangent>(), /*bCreateCollision=*/false);
}

void APuzzleActor::BuildWallColliders()
{
	// 既存コライダーを破棄してから再生成する
	for (TObjectPtr<UBoxComponent>& Col : WallColliders)
	{
		if (IsValid(Col))
		{
			Col->UnregisterComponent();
			Col->DestroyComponent();
		}
	}
	WallColliders.Empty();

	// BoxComponent を1本生成するヘルパー
	auto MakeWallBox = [&](const FVector2D& SegA, const FVector2D& SegB,
	                       const FVector2D& InwardNormal2D, float Yaw)
	{
		const float SegLen = (SegB - SegA).Size();
		if (SegLen < 1.f) return;

		const FVector2D Center2D = (SegA + SegB) * 0.5f + InwardNormal2D * (WallThickness * 0.5f);

		UBoxComponent* Box = NewObject<UBoxComponent>(this);
		Box->SetupAttachment(RootComponent);
		Box->SetBoxExtent(FVector(SegLen * 0.5f, WallThickness * 0.5f, WallHeight * 0.5f));
		Box->SetRelativeLocation(FVector(Center2D.X, Center2D.Y, WallHeight * 0.5f));
		Box->SetRelativeRotation(FRotator(0.f, Yaw, 0.f));
		Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Box->SetCollisionObjectType(ECC_WorldDynamic);
		Box->SetCollisionResponseToAllChannels(ECR_Block);
		Box->SetCastShadow(false);
		//Box->bAffectDistanceFieldLighting = false;
		//Box->bVisibleInRayTracing = false;
		Box->RegisterComponent();
		AddInstanceComponent(Box);
		WallColliders.Add(Box);
	};

	const int32 N = Vertices.Num();
	for (int32 i = 0; i < N; ++i)
	{
		const int32 Next = (i + 1) % N;
		const FVector2D A = Vertices[i];
		const FVector2D B = Vertices[Next];

		const FVector2D Edge      = B - A;
		const float     EdgeLength = Edge.Size();
		const FVector2D EdgeDir   = Edge / EdgeLength;
		const FVector2D InwardNormal2D(-EdgeDir.Y, EdgeDir.X);
		const float Yaw = FMath::RadiansToDegrees(FMath::Atan2(EdgeDir.Y, EdgeDir.X));

		const FExitGap* Gap = ExitGaps.FindByPredicate(
			[i](const FExitGap& G){ return G.EdgeIndex == i; });
		if (Gap)
		{
			const float GapCenter = Gap->CenterT * EdgeLength;
			const float HalfGap  = Gap->GapWidth * 0.5f;
			const float Seg1End  = FMath::Clamp(GapCenter - HalfGap, 0.f, EdgeLength);
			const float Seg2Start= FMath::Clamp(GapCenter + HalfGap, 0.f, EdgeLength);

			// 穴より手前のセグメント
			if (Seg1End > 1.f)
				MakeWallBox(A, A + EdgeDir * Seg1End, InwardNormal2D, Yaw);
			// 穴より奥のセグメント
			if (Seg2Start < EdgeLength - 1.f)
				MakeWallBox(A + EdgeDir * Seg2Start, B, InwardNormal2D, Yaw);
		}
		else
		{
			MakeWallBox(A, B, InwardNormal2D, Yaw);
		}
	}

	// コーナーピラー: 隣接する壁ボックス間の隙間を埋める
	for (const FVector2D& V : Vertices)
	{
		UBoxComponent* Corner = NewObject<UBoxComponent>(this);
		Corner->SetupAttachment(RootComponent);
		Corner->SetBoxExtent(FVector(WallThickness * 0.5f, WallThickness * 0.5f, WallHeight * 0.5f));
		Corner->SetRelativeLocation(FVector(V.X, V.Y, WallHeight * 0.5f));
		Corner->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Corner->SetCollisionObjectType(ECC_WorldDynamic);
		Corner->SetCollisionResponseToAllChannels(ECR_Block);
		Corner->RegisterComponent();
		AddInstanceComponent(Corner);

		WallColliders.Add(Corner);
	}

	// 前後キャップ: パズルの奥行き方向（ローカル Z 軸）を両側で塞ぐ
	// 出口（ExitGap）は周壁の辺方向に開くため、これらのキャップとは干渉しない
	FBox2D Bounds(ForceInit);
	for (const FVector2D& V : Vertices) { Bounds += V; }
	const FVector2D BoundsSize   = Bounds.GetSize();
	const FVector2D BoundsCenter = Bounds.GetCenter();

	// 背面キャップ（ローカル Z <= 0 側）
	UBoxComponent* BackBox = NewObject<UBoxComponent>(this);
	BackBox->SetupAttachment(RootComponent);
	BackBox->SetBoxExtent(FVector(BoundsSize.X * 0.5f, BoundsSize.Y * 0.5f, 1.f));
	BackBox->SetRelativeLocation(FVector(BoundsCenter.X, BoundsCenter.Y, -1.f));
	BackBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BackBox->SetCollisionObjectType(ECC_WorldDynamic);
	BackBox->SetCollisionResponseToAllChannels(ECR_Block);
	BackBox->SetCastShadow(false);
	BackBox->RegisterComponent();
	AddInstanceComponent(BackBox);
	WallColliders.Add(BackBox);

	// 前面キャップ（ローカル Z >= WallHeight 側 = カメラ向き面）
	UBoxComponent* FrontBox = NewObject<UBoxComponent>(this);
	FrontBox->SetupAttachment(RootComponent);
	FrontBox->SetBoxExtent(FVector(BoundsSize.X * 0.5f, BoundsSize.Y * 0.5f, 1.f));
	FrontBox->SetRelativeLocation(FVector(BoundsCenter.X, BoundsCenter.Y, WallHeight + 1.f));
	FrontBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FrontBox->SetCollisionObjectType(ECC_WorldDynamic);
	FrontBox->SetCollisionResponseToAllChannels(ECR_Block);
	FrontBox->SetCastShadow(false);
	FrontBox->RegisterComponent();
	AddInstanceComponent(FrontBox);
	WallColliders.Add(FrontBox);
}
