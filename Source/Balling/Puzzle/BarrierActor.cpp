#include "BarrierActor.h"
#include "PuzzleActor.h"
#include "ProceduralMeshComponent.h"

ABarrierActor::ABarrierActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
	SetRootComponent(ProcMesh);
	ProcMesh->bUseComplexAsSimpleCollision = true;
	ProcMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ProcMesh->SetCollisionObjectType(ECC_WorldDynamic);
	ProcMesh->SetCollisionResponseToAllChannels(ECR_Block);
}

void ABarrierActor::BeginPlay()
{
	Super::BeginPlay();

	OwnerPuzzle = Cast<APuzzleActor>(GetOwner());

	if (BarrierType != EBarrierType::Fixed)
	{
		SetActorTickEnabled(true);
	}
}

void ABarrierActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	BuildMesh();
}

void ABarrierActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (BarrierType)
	{
	case EBarrierType::Rotating:
		if (OwnerPuzzle.IsValid())
		{
			const float PuzzleAngle = OwnerPuzzle->GetCurrentAngle();
			SetActorRelativeRotation(FRotator(0.f, PuzzleAngle * RotationMultiplier, 0.f));
		}
		break;

	case EBarrierType::Moving:
		{
			MoveElapsed += DeltaTime * MoveSpeed;
			const float Alpha = FMath::Sin(MoveElapsed);
			const FVector NewPos = FMath::Lerp(MovePointA, MovePointB, (Alpha + 1.f) * 0.5f);
			SetActorRelativeLocation(NewPos);
		}
		break;

	default:
		break;
	}
}

void ABarrierActor::BuildMesh()
{
	ProcMesh->ClearAllMeshSections();

	const float HalfL = Length * 0.5f;
	const float HalfT = Thickness * 0.5f;

	// ローカル原点中心のボックス。Z は 0 〜 Height（パズル床面に接地）
	const FVector P[8] = {
		{-HalfL, -HalfT, 0.f     },  // 0: 前・左・下
		{ HalfL, -HalfT, 0.f     },  // 1: 前・右・下
		{ HalfL,  HalfT, 0.f     },  // 2: 後・右・下
		{-HalfL,  HalfT, 0.f     },  // 3: 後・左・下
		{-HalfL, -HalfT, Height  },  // 4: 前・左・上
		{ HalfL, -HalfT, Height  },  // 5: 前・右・上
		{ HalfL,  HalfT, Height  },  // 6: 後・右・上
		{-HalfL,  HalfT, Height  },  // 7: 後・左・上
	};

	TArray<FVector> Verts;
	TArray<int32>   Tris;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;

	// 面を追加するヘルパー。A→B→C→D の順で頂点を渡す（外向き法線、外側から見て CW）
	auto AddFace = [&](const FVector& A, const FVector& B, const FVector& C, const FVector& D, const FVector& Normal)
	{
		const int32 Base = Verts.Num();
		Verts.Add(A); Verts.Add(B); Verts.Add(C); Verts.Add(D);
		for (int32 i = 0; i < 4; ++i) { Normals.Add(Normal); }
		UVs.Add({0.f, 0.f}); UVs.Add({1.f, 0.f});
		UVs.Add({1.f, 1.f}); UVs.Add({0.f, 1.f});
		Tris.Add(Base + 0); Tris.Add(Base + 2); Tris.Add(Base + 1);
		Tris.Add(Base + 0); Tris.Add(Base + 3); Tris.Add(Base + 2);
	};

	AddFace(P[4], P[5], P[6], P[7],  FVector::UpVector);     // 上面  (+Z)
	AddFace(P[3], P[2], P[1], P[0], -FVector::UpVector);     // 下面  (-Z)
	AddFace(P[0], P[1], P[5], P[4],  FVector(0.f,-1.f,0.f)); // 前面  (-Y)
	AddFace(P[2], P[3], P[7], P[6],  FVector(0.f, 1.f,0.f)); // 後面  (+Y)
	AddFace(P[3], P[0], P[4], P[7],  FVector(-1.f,0.f,0.f)); // 左端面(-X)
	AddFace(P[1], P[2], P[6], P[5],  FVector( 1.f,0.f,0.f)); // 右端面(+X)

	ProcMesh->CreateMeshSection(0, Verts, Tris, Normals, UVs,
		TArray<FColor>(), TArray<FProcMeshTangent>(), /*bCreateCollision=*/true);

	ProcMesh->SetMaterial(0, BarrierMaterial);
}
