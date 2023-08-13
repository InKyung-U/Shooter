// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "WeaponType.h"

#include "ShooterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EOffsetState : uint8
{
	EOS_Aiming UMETA(DisplayName = "Aiming"),
	EOS_Hip UMETA(DisplayName = "Hip"),
	EOS_Reloading UMETA(DisplayName = "Reloading"),
	EOS_InAir UMETA(DisplayName = "InAir"),

	EOS_MAX UMETA(DisplayName = "DefaultMax")
};

/**
 * 
 */
UCLASS()
class SHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UShooterAnimInstance();

	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);

	virtual void NativeInitializeAnimation() override;

protected:

	void TurnInPlace();

	/* handle calculation for leaning while running*/
	void Lean(float DeltaTime);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movemert, meta = (AllowPrivateAccess = "true"))
	class AShooterCharacter* ShooterCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	// 캐릭터가 움직이는지 확인.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	/* offset Yaw used for strafing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MovementOffsetYaw;

	/* offset Yaw the Frame before we used for strafing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float LastMovementOffsetYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	float TIPCharacterYaw;

	float TIPCharacterYawLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tunr In Place", meta = (AllowPrivateAccess = "true"))
	float RootYawOffset;

	float RotationCurve;

	float RotationCurveLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tunr In Place", meta = (AllowPrivateAccess = "true"))
	float Pitch;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tunr In Place", meta = (AllowPrivateAccess = "true"))
	bool bReload;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tunr In Place", meta = (AllowPrivateAccess = "true"))
	EOffsetState OffsetState;

	// 가만히 있지 안을때 업데이트 됨.
	FRotator CharacterRotation;

	FRotator CharacterRotationLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Lean, meta = (AllowPrivateAccess = "true"))
	float YawDelta;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Crouching, meta = (AllowPrivateAccess = "true"))
	bool bCrouching;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Crouching, meta = (AllowPrivateAccess = "true"))
	bool bEquipping;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float RecoilWeight;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bTurningInPlace;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	EWeaponType EquippedWeaponType;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bShouldUseFABRIK;
};
