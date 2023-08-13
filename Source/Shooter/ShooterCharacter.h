// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AmmoType.h"
#include "ShooterCharacter.generated.h"


UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_FireTimerInProgress UMETA(DisplayName = "FireTimerInProgress"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_Equipping UMETA(DisplayName = "Equipping"),
	ECS_Stunned UMETA(DisplayName = "Stunned"),

	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};

USTRUCT(BlueprintType)
struct FInterpLocation
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ItemCount;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquipItemDelegate, int32, CurrnetSlotIndex, int32, NewSlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHighlightIconDelegate, int32, SlotIndex, bool, bStartAnimation);

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,	AActor* DamageCauser) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/* Called for forwards/backwards input */
	void MoveForward(float _value);

	/* Called for side to side input */
	void MoveRigth(float _value);

	/* Called via input to turn at a given rate
	* @param rate this is a normalized rate. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);

	/* Called via input to look up/down at a given rate
	* @param rate. this is a normalized rate. 1.0 means 100% of desired turn rate
	*/
	void LookUPAtRate(float Rate);

	void Turn(float Value);
	void LookUp(float Value);

	/* Called when the Fire Button is pressed */
	void FireWeapon();

	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult);

	void AimingButtonPressed();
	void AimingButtonReleased();

	void CameraInterpZoom(float DeltaTime);
	void SetLookRates();

	void CalculateCrosshairSpread(float DeltaTime);

	void StartCrosshairBulletFire();

	UFUNCTION()
	void FinishCrooshirBullecFire();

	void FireButtonPressed();
	void FireButtonReleased();

	void StartFireTimer();

	UFUNCTION()
	void AutoFireReset();

	/* Line trace for items under the crosshairs */
	bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation);

	/* trave for items if overlapped itemcount > 0 */
	void TraceForItems();

	class AWeapon* SpawnDefaultWeapon();

	void EquipWeapon(AWeapon* WeaponToEquip, bool bSwapping = false);

	/* detach weapon */
	void DropWeapon();

	void SelectButtonPressed();
	void SelectButtonReleased();

	void SwapWeapon(AWeapon* WeaonToSwap);

	void InitializeAmmoMap();

	bool WeaponHasAmmo();

	void PlayFireSound();
	void SendBullet();
	void PlayGunFireMontage();

	void ReloadButtonPressed();
	void ReloadWeapon();

	bool CarryingAmmo();

	UFUNCTION(BlueprintCallable)
	void GrabClip();

	UFUNCTION(BlueprintCallable)
	void ReleaseClip();

	void CrouchButtonPressed();

	virtual void Jump() override;

	void InterpCapsuleHalfHeight(float DeltaTime);

	void Aim();
	void StopAiming();

	void PickupAmmo(class AAmmo* Ammo);

	void InitializeInterpLocation();

	void FKeyPressed();
	void OneKeyPressed();
	void TwoKeyPressed();
	void ThreeKeyPressed();
	void FourKeyPressed();
	void FiveKeyPressed();

	void ExchangedInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex);

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

	int32 GetEmptyInventorySlot();

	void HighlightInventorySlot();

	UFUNCTION(BlueprintCallable)
	EPhysicalSurface GetSurfaceType();

	UFUNCTION(BlueprintCallable)
	void EndStun();

	void Die();

	UFUNCTION(BlueprintCallable)
	void FinishDeath();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	/* 카메라 붐은 캐릭터 뒤에 카메라를 위치시킨다.*/
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/* 캐릭터를 쫒는 카메라.*/
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/* 회전률에 근거. 수치 / 시간 다른 수치가 회전률에 영향받을 수 있다. */
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate;

	/* Base look up/down rate, in deg/sec. Other scaling may affect final turn rate  */
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipTurnRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipLookUpRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingTurnRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingLookUpRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float MouseHipTurnRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float MouseHipLookUpRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float MouseAimingTurnRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float MouseAimingLookUpRate;

	// 무기를 발사하기 위한 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* HipFireMontage;

	/* 총알 임팩트 효과 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;

	/* 총알 연기 효과 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	/* Default camera field of view value */
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float CameraDefaultFOV;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float CameraZoomedFOV;

	/* current field of view this frame */
	float CameraCurrentFOV;

	/* interp speed for zooming when aiming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;

	float			ShootTimeDuration;
	bool			bFiringBullet;
	FTimerHandle	CrosshairShootTimer;
	FTimerDelegate  CrosshairDelegate;

	bool bFireButtonPressed;

	/* true when we can fire */
	bool bShouldFire;

	/* Sets a timer between gunshot */
	FTimerHandle AutoFireTimer;
	FTimerDelegate  AutoFireDelegate;

	// 아이템 볼수있으면 true
	bool bShouldTraceForItems;
	// number of overlapped AItems
	int8 OverlappedItemCount;

	/* The AItem we hit last frame */
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	class AItem* TraceHitItemLastFrame;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AWeapon* EquippedWeapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeaponClass;

	/* the item currently hit by our trace int TraceForTiems */
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AItem* TraceHitItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpElevation;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TMap<EAmmoType, int32> AmmoMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
	int32 Starting9mmAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
	int32 StartingARAmmo;

	/* combat state can only fire or reload if Unoccupied */
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	ECombatState CombatState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* EquipMontage;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FTransform ClipTransform;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USceneComponent* HandSceneComponent;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bCrouching;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float BaseMovementSpeed;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float CrouchMovementSpeed;

	float CurrentCapsuleHalfHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float StandingCapsuleHalfHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchingCapsuleHalfHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseGroundFriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchingGroundFriction;

	bool bAimingButtonPressed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WeaponInterpComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp2;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp3;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp4;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp5;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp6;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FInterpLocation> InterpLocations;

	FTimerHandle PickupSoundTimer;
	FTimerHandle EquipSoundTimer;

	bool bShouldPlayPickupSound;
	bool bShouldPlayEquipSound;

	void ResetPickupSoundTimer();
	void ResetEquipSoundTimer();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item, meta = (AllowPrivateAccess = "true"))
	float PickupSoundResetTime;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item, meta = (AllowPrivateAccess = "true"))
	float EquipSoundResetTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	TArray<AItem*> Inventory;

	const int32 INVENTORY_CAPACITY{ 6 };

	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
	FEquipItemDelegate EquipItemDelegate;

	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
	FHighlightIconDelegate HighlightIconDelegate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	int32 HighlightSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* MeleeImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BloodParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float StunChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DeathMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bDead;

public:
	// FORCEINLINE -> 인라인 전용 매크로? 
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE bool GetAiming() const { return bAiming; }

	UFUNCTION(BlueprintCallable) // UFUNTION은 inline 불가하다!
	float GetCrosshairSpreadMultiplier() const;

	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }

	/* add/xub to/from overlappedItemcount adn update bShouldTraceForItems  */
	void IncrementOverlappedItemCount(int8 Amount);

	// 불필요 -> AItem has GetInterpLocation
	//FVector GetCameraInterpLocation();

	void GetPickupItem(AItem* Item);

	FORCEINLINE ECombatState GetCombatState() const { return CombatState; }
	FORCEINLINE bool GetCrouching() const { return bCrouching; }

	FInterpLocation GetInterpLocation(int32 Index);

	int32 GetInterpLocationIndex();
	void IncermentInterpLocItemCount(int32 Index, int32 Amount);

	FORCEINLINE bool ShouldPlayPickupSound() const { return bShouldPlayPickupSound; }
	FORCEINLINE bool ShouldPlayEquipSound() const { return bShouldPlayEquipSound; }

	void StartPickupSoundTimer();
	void StartEquipSoundTimer();
	void UnHighlightInventorySlot();

	FORCEINLINE AWeapon* GetEqippedWeapon() const { return EquippedWeapon; }
	FORCEINLINE USoundCue* GetMeleeImpactSound() const { return MeleeImpactSound; }
	FORCEINLINE UParticleSystem* GetBloodParticles() const { return BloodParticles; }

	void Stun();
	FORCEINLINE float GetStunChance() const { return StunChance; }

};
