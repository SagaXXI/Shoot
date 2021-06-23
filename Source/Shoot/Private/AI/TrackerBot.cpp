// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/TrackerBot.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "SHealthComponent.h"
#include "SCharacter.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"
#include "UnrealNetwork.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "AmmoDrop.h"
#include "Pickup.h"


int32 DebugTrackerBotDrawing = 0;
FAutoConsoleVariableRef CVARDebugTrackerBotDrawing(TEXT("COOP.DebugTrackerBot"),
	DebugTrackerBotDrawing,
	TEXT("Draw debug lines for tracker bot"),
	ECVF_Cheat);


// Sets default values
ATrackerBot::ATrackerBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCanEverAffectNavigation(false);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>("HealthComp");
	HealthComp->OnHealthChanged.AddDynamic(this, &ATrackerBot::HandleTakeDamage);

	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SphereComp->SetSphereRadius(200);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);

	RadialForce = CreateDefaultSubobject<URadialForceComponent>("RadialForce");
	RadialForce->SetupAttachment(MeshComp);
	RadialForce->Radius = 150;
	RadialForce->bImpulseVelChange = true;//makes impulse velocity more persistent
	RadialForce->bAutoActivate = false;//prevent from ticking
	RadialForce->bIgnoreOwningActor = true;

	bUseVelocityChange = false;
	MovementForce = 1000;
	RequiredDistanceToTarget = 100;

	ExplosionRadius = 200;
	ExplosionDamage = 40;
	
}

// Called when the game starts or when spawned
void ATrackerBot::BeginPlay()
{
	Super::BeginPlay();
	
	if (Role == ROLE_Authority)
	{
		//Find first path point
		NextPathPoint = GetPathNextPoint();

		//Check every sec
		FTimerHandle TimerHandle_CheckPowerLevel;
		GetWorldTimerManager().SetTimer(TimerHandle_CheckPowerLevel, this, &ATrackerBot::OnCheckNearbyBots, 1.0f, true);
	}
}

// Called every frame
void ATrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Role == ROLE_Authority && !bExploded)
	{
		//ƒистанци€ будет посто€нно обновл€тс€ и становитс€ больше или меньше
		float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

		if (DistanceToTarget <= RequiredDistanceToTarget)
		{
			//≈сли дистанци€ стала меньше или равна, то мы мен€ем направление. »з-за этого шар заносит
			NextPathPoint = GetPathNextPoint();

			if (DebugTrackerBotDrawing)
			{
				//ѕишет если мен€ем направление 
				DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached!");
			}
			
		}
		else
		{
			//определ€ет направление силы с помощью двух точек
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();

			ForceDirection *= MovementForce;
			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);
			if (DebugTrackerBotDrawing)
			{
				//–исует если катитьс€
				DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection,
					32, FColor::Yellow, false, 0.0f, 0, 1.0f);
			}
			
		}
		if (DebugTrackerBotDrawing)
		{
			//–исует всегда место NextPathPoint
			DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 0.0f, 1.0f);
		}
		
	}
}

FVector ATrackerBot::GetPathNextPoint()
{
	//Best Target variable
	AActor* BestTarget = nullptr;
	float NearestTargetDistance = FLT_MAX;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		//Get the pawn
		APawn* TestPawn = It->Get();
		//check if TestPawn is not nullptr and is it friendly or not
		if (TestPawn == nullptr || USHealthComponent::IsFriendly(TestPawn, this))
		{
			//skip this pawn
			continue;
		}
		//Get HealthComp of Pawn
		USHealthComponent* TestPawnHealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		//Check if HealthComp not nullptr and if health is higher than 0
		if (TestPawnHealthComp && TestPawnHealthComp->GetHealth() > 0.0f)
		{
			//Find a distance between bot and target
			float Distance = (TestPawn->GetActorLocation() - GetActorLocation()).Size();

			//Check if this Distance is bigger or smaller than NearestTargetDistance
			if (Distance < NearestTargetDistance)
			{
				//then changing the BestTarget and NearestTargetDistance
				BestTarget = TestPawn;
				NearestTargetDistance = Distance;
			}
		}
	}

	if (BestTarget)
	{
		//Array which will be writing path point of trackerbot
		UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), BestTarget);

		GetWorldTimerManager().ClearTimer(TimerHandle_RefreshPath);
		GetWorldTimerManager().SetTimer(TimerHandle_RefreshPath, this, &ATrackerBot::RefreshPath, 5.0, false);

		if (NavPath && NavPath->PathPoints.Num() > 1)
		{
			return NavPath->PathPoints[1];
		}
	}
	
	//Failed to get actor's path
	return GetActorLocation();
}

void ATrackerBot::HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float HealthDelta,
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
	if (MatInst)
	{
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
		//ChangeLastTimeDamageTaken();
	}
	UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName());

	if (Health <= 0.0f)
	{
		SelfDestruct();
	}
}

//allah akbar
void ATrackerBot::SelfDestruct()
{
	if (bExploded)
	{
		return;
	}

	bExploded = true;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());

	MeshComp->SetVisibility(false, true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	if (Role == ROLE_Authority)
	{
		//Ignored actors, ignores other bots
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		//Damage Multiplier
		float ActualDamage = ExplosionDamage + (ExplosionDamage * PowerLevel);
		
		UGameplayStatics::ApplyRadialDamage(this, ActualDamage, GetActorLocation(), ExplosionRadius, nullptr,
			IgnoredActors, this, GetInstigatorController(), true);

		RadialForce->FireImpulse();
		if (DebugTrackerBotDrawing)
		{
			DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 0, 0.0f);
		}
		

		if (Role == ROLE_Authority)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			StarterDrop = GetWorld()->SpawnActor<AAmmoDrop>(DropToCreate, GetActorLocation(),
			GetActorRotation(), SpawnParams);

			if (StarterDrop)
			{
				SetLifeSpan(2.0);
			}
		}

	}
	
}

void ATrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	//In the way to not lose some BP functionality
	Super::NotifyActorBeginOverlap(OtherActor);
	//Check if it's already boomed
	if (!bStartedToBoom && !bExploded)
	{
		// checking if OtherActor is our character
		ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);
		if (PlayerPawn && !USHealthComponent::IsFriendly(PlayerPawn, this))
		{
			if (Role == ROLE_Authority)
			{
				// otchet for boom
				GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ATrackerBot::SelfDamage, 0.5f, true, 0.0f);
			}
			
			//Set boom boolean to true
			bStartedToBoom = true;

			UGameplayStatics::SpawnSoundAttached(SelfDamageSound, RootComponent);
		}
	}
	
}

//func for otchet 
void ATrackerBot::SelfDamage()
{
	UGameplayStatics::ApplyDamage(this, 20.0f, GetInstigatorController(), this, nullptr);
}

void ATrackerBot::OnCheckNearbyBots()
{
	const float Radius = 600.f;

	//Collision shape that will check to nearby bots
	FCollisionShape CollShape;
	CollShape.SetSphere(Radius);

	//Checker itself
	FCollisionObjectQueryParams  QueryParams;
	QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	QueryParams.AddObjectTypesToQuery(ECC_Pawn);

	//this massiv will contain info about overlaps
	TArray<FOverlapResult> Overlaps;
    //this will add info in massiv
	GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, QueryParams, CollShape);
	if (DebugTrackerBotDrawing)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 12, FColor::White, false, 1.0f);
	}
	
	int32 NumberOfBots = 0;
	//Range based loop
	for (FOverlapResult Result : Overlaps)
	{
		//Check if ATrackerBot
		ATrackerBot* Bot = Cast<ATrackerBot>(Result.GetActor());
		//Check if it's not this bot
		if (Bot && Bot != this)
		{
			NumberOfBots++;
		}
	}

	const int32 MaxPowerLevel = 4;

	//Making borders of max value
	PowerLevel = FMath::Clamp(NumberOfBots, 0, MaxPowerLevel);

	//if matinst doesn't exist yet
	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
	if (MatInst)
	{
		//Converting to float and vichislyat
		Alpha = PowerLevel / (float)MaxPowerLevel;
		MatInst->SetScalarParameterValue("ParamLevelAlpha", Alpha);
		//ChangeParamLevelAlpha();
	}
	
		
	if (DebugTrackerBotDrawing)
	{
		DrawDebugString(GetWorld(), FVector(0, 0, 0), FString::FromInt(PowerLevel),this, FColor::White, 1.0f,true);
	}

	
}

void ATrackerBot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATrackerBot, Alpha);
	DOREPLIFETIME(ATrackerBot, StarterDrop);
}

void ATrackerBot::ChangeLastTimeDamageTaken_Implementation()
{
	MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
}

void ATrackerBot::ChangeParamLevelAlpha_Implementation()
{
	MatInst->SetScalarParameterValue("ParamLevelAlpha", Alpha);
}

void ATrackerBot::RefreshPath()
{
	NextPathPoint = GetPathNextPoint();
}