// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Engine/DataTable.h"
#include "WeaponType.h"

#include "Weapon.generated.h"


USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAmmoType AmmpType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WeaponAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MagazingCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USoundCue* PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* EquipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* InventoryIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* AmmoIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstance* MaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaterialIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ClipBoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ReloadMontageSection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> AnimBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairMiddle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairLeft;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairRight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairBottom;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairTop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoFireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BoneToHide;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutomatic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeadShotDamage;
};
/**
 * 
 */
UCLASS()
class SHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()
public:
	AWeapon();

	virtual void Tick(float DeltaTime) override;

protected:
	void StopFalling();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	void FinishMovingSlide();
	void UpdateSlideDisplacement();

private:
	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bFalling;

	/* Ammo count for this weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	int32 Ammo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	int32 MagazineCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	EWeaponType WeaponType;

	UPROPERTY(VisibleAnyWhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	FName ReloadMontageSection;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	bool bMovingClip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	FName ClipBoneName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	UDataTable* WeaponDataTable;

	int32 PreviousMaterialIndex;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairMiddle;
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairLeft;
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairRight;
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairBottom;
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairTop;


	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	float AutoFireRate;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	USoundCue* FireSound;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	FName BoneToHide;

	// ±ÇÃÑ ¹ß»ç½Ã À§¿¡ ¶Ñ²± ¿òÁ÷ÀÌ´Â°Å
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	float SlideDisplacement;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	UCurveFloat* SlideDisplacementCurve;

	FTimerHandle SliderTimer;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	float SlideDisplacementTime;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	bool bMovindSlide;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	float MaxSlideDisplacement;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	float MaxRecoilRatation;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Pistol, meta = (AllowPrivateAccess = "true"))
	float RecoilRatation;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	bool bAutomatic;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	float Damage;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	float HeadShotDamage;



public:
	void ThrowWeapon();

	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagazineCapacity() const { return MagazineCapacity; }

	/* fire weapon */
	void DecrementAmmo();

	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }
	FORCEINLINE FName GetReloadMontageSection() const { return ReloadMontageSection; }
	FORCEINLINE void SetReloadMontageSection(FName Name) { ReloadMontageSection = Name; }

	FORCEINLINE FName GetClipBoneName() const { return ClipBoneName; }
	FORCEINLINE void SetClipBoneName(FName Name) { ClipBoneName = Name; }

	FORCEINLINE float GetAutoFireRate() const { return AutoFireRate; }
	FORCEINLINE UParticleSystem* GetMuzzleFlash() const { return MuzzleFlash; }
	FORCEINLINE USoundCue* GetFireSound() const { return FireSound; }
	FORCEINLINE bool GetAutomatic() const { return bAutomatic; }
	FORCEINLINE void SetAutomatic(bool Auto) { bAutomatic = Auto; }
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }



	void StartSlideTimer();

	void ReloadAmmo(int32 Amount);
	void SetMovingClip(bool Move) { bMovingClip = Move; }

	bool ClipIsFull();
};
