// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Math/UnrealMathUtility.h"

AWeapon::AWeapon()
	: ThrowWeaponTime(0.7f)
	, bFalling(false)
	, Ammo(30)
	, MagazineCapacity(30)
	, WeaponType(EWeaponType::EWT_SubmachineGun)
	, AmmoType(EAmmoType::EAT_9mm)
	, ReloadMontageSection(FName(TEXT("Reload SMG")))
	, ClipBoneName(TEXT("smg_clip"))
	, SlideDisplacement(0.f)
	, SlideDisplacementTime(0.2f)
	, bMovindSlide(false)
	, MaxSlideDisplacement(4.f)
	, MaxRecoilRatation(20.f)
	, bAutomatic(true)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Keep the Weapon up right
	if (GetItemState() == EItemState::EIS_Falling && bFalling)
	{
		const FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
		GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}
	UpdateSlideDisplacement();
}

void AWeapon::ThrowWeapon()
{
	FRotator MeshRotation{ 0.5, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
	GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

	const FVector MeshForward{ GetItemMesh()->GetForwardVector() };
	const FVector MeshRight{ GetItemMesh()->GetRightVector() };
	// direction in which we throw the weapon
	FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.f, MeshForward);

	float RandomRoataton;
	RandomRoataton = FMath::FRandRange(0.f, 30.f);
	ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRoataton, FVector(0.f, 0.f, 1.f));
	ImpulseDirection *= 20'000.f;
	GetItemMesh()->AddImpulse(ImpulseDirection);

	bFalling = true;
	GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime);

	EnableGlowMaterial();
}

void AWeapon::DecrementAmmo()
{
	if (Ammo - 1 <= 0)
	{
		Ammo = 0;
	}
	else
		--Ammo;
}

void AWeapon::StartSlideTimer()
{
	bMovindSlide = true;

	GetWorldTimerManager().SetTimer(SliderTimer, this, 
		&AWeapon::FinishMovingSlide, SlideDisplacementTime);
}

void AWeapon::ReloadAmmo(int32 Amount)
{
	checkf(Ammo + Amount <= MagazineCapacity, TEXT("Attempted to realod with more than magazine"));
	Ammo += Amount;
}

bool AWeapon::ClipIsFull()
{
	return Ammo >= MagazineCapacity;
}

void AWeapon::StopFalling()
{
	bFalling = false;
	SetItemState(EItemState::EIS_Pickup);
	StartPulseTimer();
}

void AWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const FString WeaponTablePath = TEXT("/Script/Engine.DataTable'/Game/_Game/DataTable/WeaponDataTypeTable2.WeaponDataTypeTable2'");
	UDataTable* WeaponTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WeaponTablePath));

	if (WeaponTableObject)
	{
		FWeaponDataTable* WeaponDataRow = nullptr;
		
		switch (WeaponType)
		{
		case EWeaponType::EWT_SubmachineGun:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("SubmachineGun"), TEXT(""));
			break;
		case EWeaponType::EWT_AssualtRifle:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("AssultRifle"), TEXT(""));
			break;
		case EWeaponType::EWT_Pistol:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("Pistol"), TEXT(""));
			break;
		}

		if (WeaponDataRow)
		{
			AmmoType = WeaponDataRow->AmmpType;
			Ammo = WeaponDataRow->WeaponAmmo;
			MagazineCapacity = WeaponDataRow->MagazingCapacity;
			SetPickupSound(WeaponDataRow->PickupSound);
			SetEquipSound(WeaponDataRow->EquipSound);
			GetItemMesh()->SetSkeletalMesh(WeaponDataRow->ItemMesh);
			SetItemName(WeaponDataRow->ItemName);
			SetIconItem(WeaponDataRow->InventoryIcon);
			SetAmmoIcon(WeaponDataRow->AmmoIcon);

			SetMaterialInstance(WeaponDataRow->MaterialInstance);
			PreviousMaterialIndex = GetMaterialIndex();
			GetItemMesh()->SetMaterial(PreviousMaterialIndex, nullptr);
			SetMaterialIndex(WeaponDataRow->MaterialIndex);
			SetClipBoneName(WeaponDataRow->ClipBoneName);
			SetReloadMontageSection(WeaponDataRow->ReloadMontageSection);
			GetItemMesh()->SetAnimInstanceClass(WeaponDataRow->AnimBP);

			CrosshairMiddle = WeaponDataRow->CrosshairMiddle;
			CrosshairLeft = WeaponDataRow->CrosshairLeft;
			CrosshairRight = WeaponDataRow->CrosshairRight;
			CrosshairBottom = WeaponDataRow->CrosshairBottom;
			CrosshairTop = WeaponDataRow->CrosshairTop;

			AutoFireRate = WeaponDataRow->AutoFireRate;
			MuzzleFlash = WeaponDataRow->MuzzleFlash;
			FireSound = WeaponDataRow->FireSound;
			BoneToHide = WeaponDataRow->BoneToHide;
			bAutomatic = WeaponDataRow->bAutomatic;
			Damage = WeaponDataRow->Damage;
			HeadShotDamage = WeaponDataRow->HeadShotDamage;

		}

		if (GetMaterialInstance())
		{
			SetDynamicMaterialInstance(UMaterialInstanceDynamic::Create(GetMaterialInstance(), this));
			GetDynamicMaterialInstance()->SetVectorParameterValue(TEXT("FresnelColor"), GetGlowColor());
			GetItemMesh()->SetMaterial(GetMaterialIndex(), GetDynamicMaterialInstance());
			EnableGlowMaterial();
		}
	}

}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (BoneToHide != FName(""))
	{
		GetItemMesh()->HideBoneByName(BoneToHide, EPhysBodyOp::PBO_None);
	}

}

void AWeapon::FinishMovingSlide()
{
	bMovindSlide = false;
}

void AWeapon::UpdateSlideDisplacement()
{
	if (SlideDisplacementCurve && bMovindSlide)
	{
		const float ElapsedTime{ GetWorldTimerManager().GetTimerElapsed(SliderTimer) };
		const float CurveValue{ SlideDisplacementCurve->GetFloatValue(ElapsedTime) };
		SlideDisplacement = CurveValue * MaxSlideDisplacement;
		RecoilRatation = CurveValue * MaxRecoilRatation;
	}
}
