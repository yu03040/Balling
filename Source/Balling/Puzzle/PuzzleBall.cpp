#include "PuzzleBall.h"
#include "Balling/Stage/StageManagerSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

APuzzleBall::APuzzleBall()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	MeshComponent->SetCollisionObjectType(ECC_PhysicsBody);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	SetRootComponent(MeshComponent);
}

void APuzzleBall::BeginPlay()
{
	Super::BeginPlay();

	InitialTransform = GetActorTransform();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UStageManagerSubsystem* SM = GI->GetSubsystem<UStageManagerSubsystem>())
		{
			SM->RegisterBall(this);
		}
	}

	UPhysicalMaterial* PhysMat = NewObject<UPhysicalMaterial>(GetTransientPackage());
	PhysMat->Restitution      = 0.f;
	PhysMat->Friction         = 0.6f;
	PhysMat->RaiseMassToPower = 0.75f;
	MeshComponent->SetPhysMaterialOverride(PhysMat);

	MeshComponent->SetLinearDamping(LinearDamping);
	MeshComponent->SetAngularDamping(AngularDamping);
	MeshComponent->SetSimulatePhysics(true);
}

void APuzzleBall::ResetToInitial()
{
	MeshComponent->SetSimulatePhysics(false);
	SetActorTransform(InitialTransform);
	MeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
	MeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	MeshComponent->SetSimulatePhysics(true);
}
