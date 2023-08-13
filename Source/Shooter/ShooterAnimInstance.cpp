// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon.h"

UShooterAnimInstance::UShooterAnimInstance()
	:Speed(0.f)
	, bIsInAir(false)
	, bIsAccelerating(false)
	, MovementOffsetYaw(0.f)
	, LastMovementOffsetYaw(0.f)
	, bAiming(false)
	, TIPCharacterYaw(0.f)
	, TIPCharacterYawLastFrame(0.f)
	, RootYawOffset(0.f)
	, RotationCurve(0.f)
	, RotationCurveLastFrame(0.f)
	, Pitch(0.f)
	, bReload(false)
	, OffsetState(EOffsetState::EOS_Hip)
	, CharacterRotation(FRotator(0.f))
	, CharacterRotationLastFrame(FRotator(0.f))
	, YawDelta(0.f)
	, bCrouching(false)
	, RecoilWeight(0.f)
	, bTurningInPlace(false)
	, EquippedWeaponType(EWeaponType::EWT_MAX)
	, bShouldUseFABRIK(false)
{
}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (ShooterCharacter == nullptr)
	{
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}
	if (ShooterCharacter)
	{
		bReload = ShooterCharacter->GetCombatState() == ECombatState::ECS_Reloading;
		bCrouching = ShooterCharacter->GetCrouching();
		bEquipping = ShooterCharacter->GetCombatState() == ECombatState::ECS_Equipping;
		bShouldUseFABRIK = ShooterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied
						|| ShooterCharacter->GetCombatState() == ECombatState::ECS_FireTimerInProgress;

		// get the lateral speed of character form velocity
		FVector Velocity{ ShooterCharacter->GetVelocity() };
		Velocity.Z = 0;
		Speed = Velocity.Size();

		// is the character in the air?
		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();

		// is the character accelerating
		if (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f)
		{
			bIsAccelerating = true;
		}
		else
		{
			bIsAccelerating = false;
		}

		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());

		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

		if (ShooterCharacter->GetVelocity().Size() > 0.f)
			LastMovementOffsetYaw = MovementOffsetYaw;

		bAiming = ShooterCharacter->GetAiming();

		if (bReload)
		{
			OffsetState = EOffsetState::EOS_Reloading;
		}
		else if (bIsInAir)
		{
			OffsetState = EOffsetState::EOS_InAir;
		}
		else if (ShooterCharacter->GetAiming())
		{
			OffsetState = EOffsetState::EOS_Aiming;
		}
		else
		{
			OffsetState = EOffsetState::EOS_Hip;
		}

		if (ShooterCharacter->GetEqippedWeapon())
		{
			EquippedWeaponType = ShooterCharacter->GetEqippedWeapon()->GetWeaponType();
		}

	}
	TurnInPlace();
	Lean(DeltaTime);
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::TurnInPlace()
{
	if (ShooterCharacter == nullptr)
		return;

	Pitch = ShooterCharacter->GetBaseAimRotation().Pitch;


	if (Speed > 0 || bIsInAir)
	{
		RootYawOffset = 0.f;
		TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		RotationCurveLastFrame = 0.f;
		RotationCurve = 0.f;
	}
	else
	{
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		const float TIPYawDelta{ TIPCharacterYaw - TIPCharacterYawLastFrame };
		// clamp -180 , 180
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - TIPYawDelta);

		// 1.0 if turning, not 0.0
		const float Turning{ GetCurveValue(TEXT("Turning")) };
		if (Turning > 0)
		{
			bTurningInPlace = true;
			RotationCurveLastFrame = RotationCurve;
			RotationCurve = GetCurveValue(TEXT("Rotation"));
			const float DeltaRotation{ RotationCurve - RotationCurveLastFrame };

			// rootYawOffset > 0 is Turnig left
			// rootYawOffset < 0 is Turnig right
			RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;

			const float ABSRootYawOffset{ FMath::Abs(RootYawOffset) };
			if (ABSRootYawOffset > 90.f)
			{
				const float YawExcess{ ABSRootYawOffset - 90.f };
				RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}
		else
		{
			bTurningInPlace = false;
		}
	}

	if (bTurningInPlace)
	{
		if (bReload || bEquipping)
			RecoilWeight = 1.f;
		else
			RecoilWeight = 0.f;
	}
	else
	{
		if (bCrouching)
		{
			if (bReload || bEquipping)
				RecoilWeight = 1.f;
			else
				RecoilWeight = 0.1f;
		}
		else
		{
			if (bAiming || bReload || bEquipping)
				RecoilWeight = 1.f;
			else
				RecoilWeight = 0.5f;
		}
	}
}

void UShooterAnimInstance::Lean(float DeltaTime)
{
	if (ShooterCharacter == nullptr)
		return;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = ShooterCharacter->GetActorRotation();

	FRotator Delta{ UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame)};
	float DeltaYaw = Delta.Yaw;
	const float Target{ DeltaYaw / DeltaTime };
	const float Interp{ FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.f) };
	YawDelta = FMath::Clamp(Interp, -90.f, 90.f);

}
