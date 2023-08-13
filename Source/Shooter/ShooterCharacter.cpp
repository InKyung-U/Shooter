// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Engine/EngineTypes.h"

#include "Particles/ParticleSystemComponent.h"

#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Item.h"
#include "Weapon.h"
#include "Ammo.h"
#include "Shooter.h"
#include "Enemy.h"
#include "EnemyController.h"

#include "Components/WidgetComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "PhysicalMaterials/PhysicalMaterial.h"
#include "BulletHitInterface.h"


// Sets default values
AShooterCharacter::AShooterCharacter()
	// 기본 turn/lookup values
	: BaseTurnRate(45.f)
	, BaseLookUpRate(45.f)
	, bAiming(false)
	// 카메라 FOV values
	, CameraDefaultFOV(0.f) // set in beginplay
	, CameraZoomedFOV(25.f)
	, CameraCurrentFOV(0.f)
	, ZoomInterpSpeed(20.f)
	// aiming/not-aiming values
	, HipTurnRate(90.f)
	, HipLookUpRate(90.f)
	, AimingTurnRate(20.f)
	, AimingLookUpRate(20.f)
	// mouse sensitivity scale
	, MouseHipLookUpRate(1.0f)
	, MouseHipTurnRate(1.0f)
	, MouseAimingTurnRate(0.6f)
	, MouseAimingLookUpRate(0.6f)
	//crosshair
	, CrosshairSpreadMultiplier(0.f)
	, CrosshairVelocityFactor(0.f)
	, CrosshairInAirFactor(0.f)
	, CrosshairAimFactor(0.f)
	, CrosshairShootingFactor(0.f)
	// Bullet Fire timer variable
	, ShootTimeDuration(0.05f)
	, bFiringBullet(false)
	// Automatic gun Fire Rate
	, bShouldFire(true)
	, bFireButtonPressed(false)
	// item trace variable
	, bShouldTraceForItems(false)
	// camera interp lacation variable
	, CameraInterpDistance(250.f)
	, CameraInterpElevation(65.f)
	, Starting9mmAmmo(85)
	, StartingARAmmo(120)
	, CombatState(ECombatState::ECS_Unoccupied)
	, bCrouching(false)
	, BaseMovementSpeed(650.f)
	, CrouchMovementSpeed(300.f)
	, StandingCapsuleHalfHeight(88.f)
	, CrouchingCapsuleHalfHeight(44.f)
	, BaseGroundFriction(2.f)
	, CrouchingGroundFriction(100.f)
	, bAimingButtonPressed(false)
	, bShouldPlayPickupSound(false)
	, bShouldPlayEquipSound(false)
	, PickupSoundResetTime(0.2f)
	, EquipSoundResetTime(0.2f)
	, HighlightSlot(-1)
	, Health(100.f)
	, MaxHealth(100.f)
	, StunChance(0.25f)
	, bDead(false)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create CameraBoom (pulls in toward the character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.f;
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	// Create a Follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach camera to end of
	FollowCamera->bUsePawnControlRotation = false; // Camera dose not rotate relative to arm

	// dont rotate when controller rotates.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// configure character movement 
	GetCharacterMovement()->bOrientRotationToMovement = false;
	// FRotator(Pitch, Yaw, Roll);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);

	// 점프 속도
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComp"));

	WeaponInterpComp = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Interpolation Component"));
	WeaponInterpComp->SetupAttachment(GetFollowCamera());

	InterpComp1 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 1"));
	InterpComp1->SetupAttachment(GetFollowCamera());
	InterpComp2 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 2"));
	InterpComp2->SetupAttachment(GetFollowCamera());
	InterpComp3 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 3"));
	InterpComp3->SetupAttachment(GetFollowCamera());
	InterpComp4 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 4"));
	InterpComp4->SetupAttachment(GetFollowCamera());
	InterpComp5 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 5"));
	InterpComp5->SetupAttachment(GetFollowCamera());
	InterpComp6 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 6"));
	InterpComp6->SetupAttachment(GetFollowCamera());

}

float AShooterCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (Health - DamageAmount <= 0.f)
	{
		Health = 0.f;
		Die();

		auto EnemyController = Cast<AEnemyController>(EventInstigator);
		if (EnemyController)
		{
			EnemyController->GetBlackBoardComponent()->SetValueAsBool(FName("CharacterDead"), true);
		}
	}
	else
	{
		Health -= DamageAmount;
	}

	return DamageAmount;
}

void AShooterCharacter::Die()
{
	bDead = true;
	auto AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
		//AnimInstance->Montage_JumpToSection(FName("DeathA"), DeathMontage);
	}
}

void AShooterCharacter::FinishDeath()
{
	GetMesh()->bPauseAnims = true;
	auto PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		DisableInput(PC);
	}
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// delegate binding
	CrosshairDelegate.BindUFunction(this, FName("FinishCrooshirBullecFire"));
	AutoFireDelegate.BindUFunction(this, FName("AutoFireReset"));

	if (FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}
	EquipWeapon(SpawnDefaultWeapon());
	Inventory.Add(EquippedWeapon);
	EquippedWeapon->SetSlotIndex(0);

	EquippedWeapon->DisableCustomDepth();
	EquippedWeapon->DisableGlowMaterial();
	EquippedWeapon->SetCharacter(this);

	InitializeAmmoMap();
	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	InitializeInterpLocation();
}

void AShooterCharacter::MoveForward(float _value)
{
	if ((Controller != nullptr) && (_value != 0.0))
	{
		// find out which way is forward
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X)};
		AddMovementInput(Direction, _value);
	}
}

void AShooterCharacter::MoveRigth(float _value)
{
	if ((Controller != nullptr) && (_value != 0.0))
	{
		// find out which way is right
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
		AddMovementInput(Direction, _value);
	}
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::LookUPAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::Turn(float Value)
{
	float TurnScaleFactor{};
	if (bAiming)
	{
		TurnScaleFactor = MouseAimingTurnRate;
	}
	else
	{
		TurnScaleFactor = MouseHipTurnRate;
	}
	AddControllerYawInput(Value * TurnScaleFactor);
}

void AShooterCharacter::LookUp(float Value)
{
	float LookUpScaleFactor{};
	if (bAiming)
	{
		LookUpScaleFactor = MouseAimingLookUpRate;
	}
	else
	{
		LookUpScaleFactor = MouseHipLookUpRate;
	}
	AddControllerPitchInput(Value * LookUpScaleFactor);
}

void AShooterCharacter::FireWeapon()
{
	if (EquippedWeapon == nullptr)
		return;
	if (CombatState != ECombatState::ECS_Unoccupied)
		return;
	if (WeaponHasAmmo())
	{
		PlayFireSound();
		SendBullet();
		PlayGunFireMontage();
		EquippedWeapon->DecrementAmmo();

		StartFireTimer();

		if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol)
		{
			// Start Moving Slide Timer;
			EquippedWeapon->StartSlideTimer();
		}
	}
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult)
{
	FVector OutBeamLocation;
	FHitResult CroohairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CroohairHitResult, OutBeamLocation);

	if (bCrosshairHit)
	{
		// Tentative beam Location - still need to trace from gun
		OutBeamLocation = CroohairHitResult.Location;
	}
	else // no crosshair trace hit
	{
		// Out Beam Location is the End location for the line trace
	}

	// 총구에서 두번쨰 트레이스를 설정해준다.
	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	const FVector StartToEnd{ OutBeamLocation - MuzzleSocketLocation };
	const FVector WeaponTraceEnd{ MuzzleSocketLocation + StartToEnd * 1.25f };

	GetWorld()->LineTraceSingleByChannel(
		OutHitResult, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

	if (!OutHitResult.bBlockingHit) // 목표지점과 오브젝트 사이에 다른 물체가 있을시
	{
		OutHitResult.Location = OutBeamLocation;
		return false;
	}
	return true;
}

void AShooterCharacter::AimingButtonPressed()
{
	bAimingButtonPressed = true;
	if (CombatState != ECombatState::ECS_Reloading && CombatState != ECombatState::ECS_Equipping
		&& CombatState != ECombatState::ECS_Stunned)
	{
		Aim();
	}
}

void AShooterCharacter::AimingButtonReleased()
{
	bAimingButtonPressed = false;
	StopAiming();
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	/* Animing button pressed? */
	if (bAiming)
	{
		CameraCurrentFOV = FMath::FInterpTo(
			CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		CameraCurrentFOV = FMath::FInterpTo(
			CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::SetLookRates()
{
	if (bAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange(0.f, 600.f);
	FVector2D VelocityMultiplierRange(0.f, 1.f);
	FVector Velocity(GetVelocity());
	Velocity.Z = 0.f;

	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(
		WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	if (GetCharacterMovement()->IsFalling())
	{
		// 공중에 있을시 천천히 조준점이 좁혀짐
		CrosshairInAirFactor = FMath::FInterpTo(
			CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else
	{
		// 땅에 있을시 빠르게 조준점이 좁혀짐
		CrosshairInAirFactor = FMath::FInterpTo(
			CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
	}

	if (bAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(
			CrosshairAimFactor, 0.6f, DeltaTime, 30.f);
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(
			CrosshairAimFactor, 0.f, DeltaTime, 2.25f);
	}

	// True 0.05 second after firing
	if (bFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 60.f);
	}

	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor
		- CrosshairAimFactor + CrosshairShootingFactor;
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;

	GetWorldTimerManager().SetTimer(CrosshairShootTimer, CrosshairDelegate, ShootTimeDuration, false);

	CrosshairShootTimer;
}

void AShooterCharacter::FinishCrooshirBullecFire()
{
	bFiringBullet = false;
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	FireWeapon();
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	if (EquippedWeapon == nullptr)
		return;

	CombatState = ECombatState::ECS_FireTimerInProgress;
	GetWorldTimerManager().SetTimer(AutoFireTimer, AutoFireDelegate, EquippedWeapon->GetAutoFireRate(), false);
}

void AShooterCharacter::AutoFireReset()
{
	if (CombatState == ECombatState::ECS_Stunned)
		return;

	CombatState = ECombatState::ECS_Unoccupied;

	if (EquippedWeapon == nullptr)
		return;

	if (WeaponHasAmmo())
	{
		if (bFireButtonPressed && EquippedWeapon->GetAutomatic())
		{
			FireWeapon();
		}
	}
	else
	{
		ReloadWeapon();
	}

}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult , FVector& OutHitLocation)
{
	// 뷰포트의 사이즈를 받아옴
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// 십자선의 위치를 지정함.
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	CrosshairLocation.Y -= 50.f;
	FVector CorsshairWorldPosition;
	FVector CorsshairWorldDirection;

	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation, CorsshairWorldPosition, CorsshairWorldDirection);

	if (bScreenToWorld)
	{
		// Trace from crosshair world location outward
		const FVector Start{ CorsshairWorldPosition };
		const FVector End{ Start + CorsshairWorldDirection * 50'000.f };
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}
	return false;
}

void AShooterCharacter::TraceForItems()
{
	if (bShouldTraceForItems)
	{
		FHitResult ItemTraceResult;
		FVector HitLocation;
		TraceUnderCrosshairs(ItemTraceResult, HitLocation);
		if (ItemTraceResult.bBlockingHit)
		{
			TraceHitItem = Cast<AItem>(ItemTraceResult.GetActor());
			const auto TraceHitWeapn = Cast<AWeapon>(TraceHitItem);
			if (TraceHitWeapn)
			{
				if (HighlightSlot == -1)
				{
					HighlightInventorySlot();
				}
			}
			else
			{
				if (HighlightSlot != -1)
				{
					UnHighlightInventorySlot();
				}
			}

			if (TraceHitItem && TraceHitItem->GetItemState() == EItemState::EIS_EquipInterping)
			{
				TraceHitItem = nullptr;
			}

			if (TraceHitItem && TraceHitItem->GetPickupWidget())
			{
				// Show Item's widget
				TraceHitItem->GetPickupWidget()->SetVisibility(true);
				TraceHitItem->EnableCustomDepth();

				if (Inventory.Num() >= INVENTORY_CAPACITY)
				{
					TraceHitItem->SetCharacterInventoryFull(true);
				}
				else
				{
					TraceHitItem->SetCharacterInventoryFull(false);
				}
			}

			// we hit an Aitem last frame
			if (TraceHitItemLastFrame)
			{
				if(TraceHitItem != TraceHitItemLastFrame)
				{
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
					TraceHitItemLastFrame->DisableCustomDepth();
				}
			}

			// Store a reference to hititem for next frame
			TraceHitItemLastFrame = TraceHitItem;
		}
	}
	else if(TraceHitItemLastFrame)
	{
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
		TraceHitItemLastFrame->DisableCustomDepth();

	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	if (DefaultWeaponClass)
	{
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);	 
	}
	return nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip, bool bSwapping)
{
	if (WeaponToEquip)
	{
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}

		if (EquippedWeapon == nullptr)
		{
			// -1 은 장착 무기 x
			EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSloatIndex());
		}
		else if(!bSwapping)
		{
			EquipItemDelegate.Broadcast(EquippedWeapon->GetSloatIndex(), WeaponToEquip->GetSloatIndex());
		}

		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}
}

void AShooterCharacter::DropWeapon()
{
	if (EquippedWeapon)
	{
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);

		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();
	}
}

void AShooterCharacter::SelectButtonPressed()
{
	if (CombatState != ECombatState::ECS_Unoccupied)
		return;


	if (TraceHitItem)
	{
		TraceHitItem->StartItemCurve(this, true);
		TraceHitItem = nullptr;
	}
}

void AShooterCharacter::SelectButtonReleased()
{
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaonToSwap)
{
	if (Inventory.Num() - 1 >= EquippedWeapon->GetSloatIndex())
	{
		Inventory[EquippedWeapon->GetSloatIndex()] = WeaonToSwap;
		WeaonToSwap->SetSlotIndex(EquippedWeapon->GetSloatIndex());
	}

	DropWeapon();
	EquipWeapon(WeaonToSwap, true);
	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);

}

bool AShooterCharacter::WeaponHasAmmo()
{
	if (EquippedWeapon == nullptr)
		return false;

	return EquippedWeapon->GetAmmo() > 0;
}

void AShooterCharacter::PlayFireSound()
{
	// paly fire sound
	if (EquippedWeapon->GetFireSound())
	{
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->GetFireSound());
	}
}

void AShooterCharacter::SendBullet()
{// Send Bullet
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());

		if (EquippedWeapon->GetMuzzleFlash())
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EquippedWeapon->GetMuzzleFlash(), SocketTransform);
		}
		FHitResult BeamHitResult;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamHitResult);

		if (bBeamEnd)
		{
			// hit actor 가 bullethitInterface 를 구현했나요?
			if (BeamHitResult.GetActor())
			{
				IBulletHitInterface* BulletHitInterface = Cast<IBulletHitInterface>(BeamHitResult.GetActor());
				if (BulletHitInterface)
				{
					BulletHitInterface->BulletHit_Implementation(BeamHitResult, this, GetController());
				}

				AEnemy* HitEnemy = Cast<AEnemy>(BeamHitResult.GetActor());
				if (HitEnemy)
				{
					int32 Damage{};

					AItem* EquippedWeaponItem = Cast<AItem>(EquippedWeapon);
					float CirticalRate = EquippedWeaponItem->GetCriticalRate();
					float MaxDamageRate;

					if (CirticalRate >= FMath::FRandRange(0.f, 1.f))
					{
						// Critical Hit!
						MaxDamageRate = EquippedWeaponItem->GetMaxCriticalRate();
					}
					else
					{
						// Normal Hit!
						MaxDamageRate = EquippedWeaponItem->GetMaxNormalDamageRate();
					}

					if (BeamHitResult.BoneName.ToString() == HitEnemy->GetHeadBone())
					{
						Damage = EquippedWeapon->GetHeadShotDamage();
						Damage = FMath::FRandRange(Damage, Damage * MaxDamageRate);

						UGameplayStatics::ApplyDamage(BeamHitResult.GetActor(), Damage,
							GetController(), this, UDamageType::StaticClass());
						HitEnemy->ShowHitNumber(Damage, BeamHitResult.Location, true);

					}
					else
					{
						Damage = EquippedWeapon->GetDamage();
						Damage = FMath::FRandRange(Damage, Damage * MaxDamageRate);

						UGameplayStatics::ApplyDamage(BeamHitResult.GetActor(), Damage,
							GetController(), this, UDamageType::StaticClass());
						HitEnemy->ShowHitNumber(Damage, BeamHitResult.Location, false);

					}

					//UE_LOG(LogTemp, Warning, TEXT("Hit Component : %s"), *BeamHitResult.BoneName.ToString());

				}

			}
			else
			{
				// 적이 안맞았을 때 디폴트 파티클 사출
				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamHitResult.Location);
				}
			}

			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(), BeamParticles, SocketTransform);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamHitResult.Location);
			}
		}
	}
}

void AShooterCharacter::PlayGunFireMontage()
{
	// 몽타주를 가지고 애니메이션을 만듦.
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

void AShooterCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied)
		return;
	
	if (EquippedWeapon == nullptr)
		return;

	if (CarryingAmmo() && !EquippedWeapon->ClipIsFull())
	{
		if (bAiming)
		{
			StopAiming();
		}

		CombatState = ECombatState::ECS_Reloading;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (ReloadMontage && AnimInstance)
		{
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());
		}
	}
}

void AShooterCharacter::FinishReloading()
{
	if (CombatState == ECombatState::ECS_Stunned)
		return;

	CombatState = ECombatState::ECS_Unoccupied;

	if (bAimingButtonPressed)
	{
		Aim();
	}

	if (EquippedWeapon == nullptr)
		return;

	const auto AmmoType = EquippedWeapon->GetAmmoType();
	// update ammo map
	if (AmmoMap.Contains(AmmoType))
	{
		int32 CarriedAmmo = AmmoMap[AmmoType];
		const int32 MagEmptySpace = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo();

		if (MagEmptySpace > CarriedAmmo)
		{
			// reload the magazine with all the ammo we are carrying
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
		else
		{
			// fill all magazine
			EquippedWeapon->ReloadAmmo(MagEmptySpace);
			CarriedAmmo -= MagEmptySpace;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
	}
}

void AShooterCharacter::FinishEquipping()
{
	if (CombatState == ECombatState::ECS_Stunned)
		return;

	CombatState = ECombatState::ECS_Unoccupied;
	if (bAimingButtonPressed)
	{
		Aim();
	}
}

int32 AShooterCharacter::GetEmptyInventorySlot()
{
	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		if (Inventory[i] == nullptr)
		{
			return i;
		}
	}
	if (Inventory.Num() < INVENTORY_CAPACITY)
	{
		return Inventory.Num();
	}

	return -1; // inventory is full!
}

void AShooterCharacter::HighlightInventorySlot()
{
	const int32 EmptySlot{ GetEmptyInventorySlot() };
	HighlightIconDelegate.Broadcast(EmptySlot, true);
	HighlightSlot = EmptySlot;
}

EPhysicalSurface AShooterCharacter:: GetSurfaceType()
{
	FHitResult HitResult;
	const FVector Start{ GetActorLocation() };
	const FVector End{ Start + FVector{0.f,0.f, -400.f} };

	// 충돌 쿼리
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;

	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, 
		ECollisionChannel::ECC_Visibility, QueryParams);

	return UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
}

void AShooterCharacter::EndStun()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if (bAimingButtonPressed)
	{
		Aim();
	}
}

void AShooterCharacter::UnHighlightInventorySlot()
{
	HighlightIconDelegate.Broadcast(HighlightSlot, false);
	HighlightSlot = -1;
}

void AShooterCharacter::Stun()
{
	if (Health <= 0.f)
		return;

	CombatState = ECombatState::ECS_Stunned;

	auto AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);

	}
}

bool AShooterCharacter::CarryingAmmo()
{
	if (EquippedWeapon == nullptr)
		return false;
	auto AmmoType = EquippedWeapon->GetAmmoType();

	if (AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}

	return false;
}

void AShooterCharacter::GrabClip()
{
	if (EquippedWeapon == nullptr)
		return;
	if (HandSceneComponent == nullptr)
		return;

	int32 ClipBoneIndex{ EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName()) };
	ClipTransform = EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("Hand_L")));
	HandSceneComponent->SetWorldTransform(ClipTransform);
	EquippedWeapon->SetMovingClip(true);
}

void AShooterCharacter::ReleaseClip()
{
	if (EquippedWeapon == nullptr)
		return;
	EquippedWeapon->SetMovingClip(false);

}

void AShooterCharacter::CrouchButtonPressed()
{
	if (!GetCharacterMovement()->IsFalling())
	{
		bCrouching = !bCrouching;
	}

	if (bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
		GetCharacterMovement()->GroundFriction = CrouchingGroundFriction;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
		GetCharacterMovement()->GroundFriction = BaseGroundFriction;
	}

}

void AShooterCharacter::Jump()
{
	if (bCrouching)
	{
		bCrouching = false;
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;

	}
	else
	{
		ACharacter::Jump();
	}
}

void AShooterCharacter::InterpCapsuleHalfHeight(float DeltaTime)
{
	float TargetCapsuleHalfHeight{};

	if (bCrouching)
	{
		TargetCapsuleHalfHeight = CrouchingCapsuleHalfHeight;
	}
	else
		TargetCapsuleHalfHeight = StandingCapsuleHalfHeight;

	const float InterpHalfHeight{ FMath::FInterpTo(
		GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetCapsuleHalfHeight, DeltaTime, 20.f) };

	const float DeltaCapsuleHalfHeight{ InterpHalfHeight  - GetCapsuleComponent()->GetScaledCapsuleHalfHeight() };
	const FVector MeshOffset{ 0.f, 0.f, -DeltaCapsuleHalfHeight };
	GetMesh()->AddLocalOffset(MeshOffset);

	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHeight);
}

void AShooterCharacter::Aim()
{
	bAiming = true;
	GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
}

void AShooterCharacter::StopAiming()
{
	bAiming = false;
	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
}

void AShooterCharacter::PickupAmmo(class AAmmo* Ammo)
{
	if (AmmoMap.Find(Ammo->GetAmmoType()))
	{
		int32 AmmoCount{ AmmoMap[Ammo->GetAmmoType()] };
		AmmoCount += Ammo->GetItemCount();
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}

	if (EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType())
	{
		if (EquippedWeapon->GetAmmo() == 0)
			ReloadWeapon();
	}

	Ammo->Destroy();
}

void AShooterCharacter::InitializeInterpLocation()
{
	FInterpLocation WeaponLocation{ WeaponInterpComp ,0 };
	InterpLocations.Add(WeaponLocation);

	FInterpLocation InterpLoc1{ InterpComp1, 0 };
	InterpLocations.Add(InterpLoc1);
 	FInterpLocation InterpLoc2{ InterpComp2, 0 };
	InterpLocations.Add(InterpLoc2);
 	FInterpLocation InterpLoc3{ InterpComp3, 0 };
	InterpLocations.Add(InterpLoc3);
 	FInterpLocation InterpLoc4{ InterpComp4, 0 };
	InterpLocations.Add(InterpLoc4);
 	FInterpLocation InterpLoc5{ InterpComp5, 0 };
	InterpLocations.Add(InterpLoc5);
 	FInterpLocation InterpLoc6{ InterpComp6, 0 };
	InterpLocations.Add(InterpLoc6);

}

void AShooterCharacter::FKeyPressed()
{
	if (EquippedWeapon->GetSloatIndex() == 0)
		return;
	ExchangedInventoryItems(EquippedWeapon->GetSloatIndex(), 0);
}

void AShooterCharacter::OneKeyPressed()
{
	if (EquippedWeapon->GetSloatIndex() == 1)
		return;
	ExchangedInventoryItems(EquippedWeapon->GetSloatIndex(), 1);
}

void AShooterCharacter::TwoKeyPressed()
{
	if (EquippedWeapon->GetSloatIndex() == 2)
		return;
	ExchangedInventoryItems(EquippedWeapon->GetSloatIndex(), 2);
}

void AShooterCharacter::ThreeKeyPressed()
{
	if (EquippedWeapon->GetSloatIndex() == 3)
		return;
	ExchangedInventoryItems(EquippedWeapon->GetSloatIndex(), 3);
}

void AShooterCharacter::FourKeyPressed()
{
	if (EquippedWeapon->GetSloatIndex() == 4)
		return;
	ExchangedInventoryItems(EquippedWeapon->GetSloatIndex(), 4);
}

void AShooterCharacter::FiveKeyPressed()
{
	if (EquippedWeapon->GetSloatIndex() == 5)
		return;
	ExchangedInventoryItems(EquippedWeapon->GetSloatIndex(), 5);
}

void AShooterCharacter::ExchangedInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
{
	const bool bCanExchangeItems = (CurrentItemIndex != NewItemIndex) && (NewItemIndex < Inventory.Num())
		&& (CombatState == ECombatState::ECS_Unoccupied || CombatState != ECombatState::ECS_Equipping);
	
	if (bCanExchangeItems)
	{
		if (bAiming)
		{
			StopAiming();
		}

		auto OldEquippedWeapon = EquippedWeapon;
		auto NewWeapon = Cast<AWeapon>(Inventory[NewItemIndex]);
		EquipWeapon(NewWeapon);

		OldEquippedWeapon->SetItemState(EItemState::EIS_Pickedup);
		NewWeapon->SetItemState(EItemState::EIS_Equipped);

		CombatState = ECombatState::ECS_Equipping;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && EquipMontage)
		{
			//UE_LOG(LogTemp, Warning, TEXT("called EquipMontage"));
			AnimInstance->Montage_Play(EquipMontage, 1.f);
			AnimInstance->Montage_JumpToSection(FName("Equip"));
		}
		NewWeapon->PlayEquipSound(true);
	}
}

int32 AShooterCharacter::GetInterpLocationIndex()
{
	int32 LowestIndex = 1;
	int32 LowsetCount = INT_MAX;

	for (int32 i = 1; i < InterpLocations.Num(); i++)
	{
		if (InterpLocations[i].ItemCount < LowsetCount)
		{
			LowestIndex = i;
			LowsetCount = InterpLocations[i].ItemCount;
		}
	}

	return LowestIndex;
}

void AShooterCharacter::IncermentInterpLocItemCount(int32 Index, int32 Amount)
{
	if (Amount < -1 || Amount > 1)
		return;

	if (InterpLocations.Num() >= Index)
	{
		InterpLocations[Index].ItemCount += Amount;
	}
}

void AShooterCharacter::StartPickupSoundTimer()
{
	bShouldPlayPickupSound = false;
	GetWorldTimerManager().SetTimer(PickupSoundTimer, this, &AShooterCharacter::ResetPickupSoundTimer, PickupSoundResetTime);
}

void AShooterCharacter::StartEquipSoundTimer()
{
	bShouldPlayEquipSound = false;
	GetWorldTimerManager().SetTimer(EquipSoundTimer, this, &AShooterCharacter::ResetEquipSoundTimer, EquipSoundResetTime);
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CameraInterpZoom(DeltaTime);
	SetLookRates();

	CalculateCrosshairSpread(DeltaTime);

	TraceForItems();

	InterpCapsuleHalfHeight(DeltaTime);
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRigth);

	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUPAtRate);

	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);

	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AShooterCharacter::SelectButtonReleased);

	PlayerInputComponent->BindAction("ReloadButton", IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterCharacter::CrouchButtonPressed);

	PlayerInputComponent->BindAction("FKey", IE_Pressed, this, &AShooterCharacter::FKeyPressed);
	PlayerInputComponent->BindAction("1Key", IE_Pressed, this, &AShooterCharacter::OneKeyPressed);
	PlayerInputComponent->BindAction("2Key", IE_Pressed, this, &AShooterCharacter::TwoKeyPressed);
	PlayerInputComponent->BindAction("3Key", IE_Pressed, this, &AShooterCharacter::ThreeKeyPressed);
	PlayerInputComponent->BindAction("4Key", IE_Pressed, this, &AShooterCharacter::FourKeyPressed);
	PlayerInputComponent->BindAction("5Key", IE_Pressed, this, &AShooterCharacter::FiveKeyPressed);

}

void AShooterCharacter::ResetPickupSoundTimer()
{
	bShouldPlayPickupSound = true;
}

void AShooterCharacter::ResetEquipSoundTimer()
{
	bShouldPlayEquipSound = true;
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}

/* no longer need */
//FVector AShooterCharacter::GetCameraInterpLocation()
//{
//	const FVector CameraWorldLocation{ FollowCamera->GetComponentLocation() };
//	const FVector CameraForward{ FollowCamera->GetForwardVector() };
//	// Desired = CameraWorldLocation + Forward * A + up * B;
//	return CameraWorldLocation + CameraForward * CameraInterpDistance
//		+ FVector(0.f, 0.f, CameraInterpElevation);
//}

void AShooterCharacter::GetPickupItem(AItem* Item)
{
	Item->PlayEquipSound();

	auto Weapon = Cast<AWeapon>(Item);
	if (Weapon)
	{
		if (Inventory.Num() < INVENTORY_CAPACITY)
		{
			Weapon->SetSlotIndex(Inventory.Num());
			Inventory.Add(Weapon);
			Weapon->SetItemState(EItemState::EIS_Pickedup);
		}
		else
		{
			SwapWeapon(Weapon);
		}
	}

	auto Ammo = Cast<AAmmo>(Item);
	if (Ammo)
	{
		PickupAmmo(Ammo);
	}
}

FInterpLocation AShooterCharacter::GetInterpLocation(int32 Index)
{
	if (Index <= InterpLocations.Num())
	{
		return InterpLocations[Index];
	}
	return FInterpLocation();
}
