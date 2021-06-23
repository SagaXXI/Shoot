// Fill out your copyright notice in the Description page of Project Settings.


#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "MyGameMode.h"
#include "SCharacter.h"

// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	DefaultHealth = 100;
	bIsDead = false;
	TeamNum = 255;
	SetIsReplicated(true);
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			//Вызывает функцию когда получает урон
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::WhenTakingDamage);
		}
	}
	
	Health = DefaultHealth;
}

void USHealthComponent::WhenTakingDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f || bIsDead) 
	{
		return;
	}

	//Check if it is friendlyfire or not
	if (DamagedActor != DamageCauser && IsFriendly(DamagedActor, DamageCauser))
	{
		return;
	}

	//Making a border for value of health
	Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);

	UE_LOG(LogTemp, Log, TEXT("Health changed: %s"), *FString::SanitizeFloat(Health));
	//like an "if" operator but easier to write
	bIsDead = Health <= 0.0f;

	//Передает значения событию, когда получает урон, а они в свою очередь выполняются только на серваке, оно просто просчитывает дамаг и хп.
	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

	if (bIsDead)
	{
		//Get's gamemode only on server
		AMyGameMode* GM = Cast<AMyGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			//Broadcast variables to event
			GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
		}
	}
}

void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealthComponent, Health);
	
}

// Копия ивента который просчитывает дамаг и здоровье, но защищен примитивным античитом, возможно в будущем понадобится
//На сервере выполняется все что связано с дамагом и сменой хп. Когда меняется хп, запускается эта функция и передает значение клиентам,
//типа функции гермеса, и мы начинаем видеть ATrackerBot::HandleTakeDamage
void USHealthComponent::OnRep_Health(float OldHealth)
{
	//mb tom looman dopustil oshibku
	float Damage = Health - OldHealth;

	OnHealthChanged.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
}

void USHealthComponent::Heal(float HP)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (HP <= 0.0f || Health <= 0.0f)
		{
			return;
		}

		//same as Health += HP but with clamp
		Health = FMath::Clamp(Health + HP, 0.0f, DefaultHealth);

		UE_LOG(LogTemp, Log, TEXT("Health changed: %s (+%s)"), *FString::SanitizeFloat(Health), *FString::SanitizeFloat(HP));

		//With this we can see HealthIcon removing, because we've sent value of health to character
		OnHealthChanged.Broadcast(this, Health, -HP, nullptr, nullptr, nullptr);
	}
}

float USHealthComponent::GetHealth() const
{
	return Health;
}

bool USHealthComponent::IsFriendly(AActor* ActorA, AActor* ActorB)
{
	if (ActorA == nullptr || ActorB == nullptr)
	{
		return true;
	}

	//Get HealthComp and then get TeamNum
	USHealthComponent* HealthCompA = Cast<USHealthComponent>(ActorA->GetComponentByClass(USHealthComponent::StaticClass()));
	USHealthComponent* HealthCompB = Cast<USHealthComponent>(ActorB->GetComponentByClass(USHealthComponent::StaticClass()));

	if (HealthCompA == nullptr || HealthCompB == nullptr)
	{
		return true;
	}

	//Check if they are in one team or not
	return HealthCompA->TeamNum == HealthCompB->TeamNum;
}