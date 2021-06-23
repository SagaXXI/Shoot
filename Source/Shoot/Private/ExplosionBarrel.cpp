// Fill out your copyright notice in the Description page of Project Settings.


#include "ExplosionBarrel.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "SHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AExplosionBarrel::AExplosionBarrel()
{
	HealthComp = CreateDefaultSubobject<USHealthComponent>("HealthComp");
	HealthComp->OnHealthChanged.AddDynamic(this, &AExplosionBarrel::WhenHealthChanged);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = MeshComp;

	RadialForce = CreateDefaultSubobject<URadialForceComponent>("RadialForce");
	RadialForce->SetupAttachment(MeshComp);
	RadialForce->Radius = 250;
	RadialForce->bImpulseVelChange = true;//makes impulse velocity more persistent
	RadialForce->bAutoActivate = false;//prevent from ticking
	RadialForce->bIgnoreOwningActor = true;

	ExplosionImpulse = 400;
	ExplosionDamage = 50.0f;
	ExplosionRadius = 300.f;

	SetReplicates(true);
	SetReplicateMovement(true);
}

void AExplosionBarrel::WhenHealthChanged(USHealthComponent* OwnHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	//Nothing to do. Already exploded
	if (bExploded)
	{
		return;
	}
	if (Health <= 0.0f)
	{
		bExploded = true;
		OnRep_PlayVisualEffects();

		FVector BoostIntensivity = FVector::UpVector * ExplosionImpulse;
		MeshComp->AddImpulse(BoostIntensivity, NAME_None, true);

		TArray<AActor*> IgnoredActors;
		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors,
			this, GetInstigatorController(), true);
		RadialForce->FireImpulse();
	}
}

void AExplosionBarrel::OnRep_PlayVisualEffects()
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	MeshComp->SetMaterial(0, ExplosionMaterial);
}
void AExplosionBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AExplosionBarrel, bExploded);

}
