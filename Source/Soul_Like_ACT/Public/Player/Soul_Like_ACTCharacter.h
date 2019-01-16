// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TargetableActor.h"
#include "Soul_Like_ACTCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGetLeanAmount, float, LeanAmount);

UCLASS(config=Game)
class ASoul_Like_ACTCharacter : public ATargetableActor
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UAnimManager* AnimManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UArrowComponent* TargetLockArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class ULockTargetComponent *TargetLockingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = AI)
	class UAIPerceptionStimuliSourceComponent *AIPerceptionStimuliSource;

protected:
	virtual void BeginPlay() override;

public:

	static const float BattleMovementScale;
	static const float TravelMovementScale;

	ASoul_Like_ACTCharacter();

	virtual void Tick(float DeltaTime) override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	float ForwardAxisValue, RightAxisValue;
	float LeanAmount_Char, LeanSpeed_Char, LeanAmount_Anim;

protected:
	//Tick------------------------------
	void MoveForward(float Value);
	void MoveRight(float Value);
	void MakeMove();

	void TurnAtRate(float Rate);

	void LookUpAtRate(float Rate);

	void UseLMB();

	void UseRMB_Pressed();
	void UseRMB_Released ();


	void UseDodge();

	void CalculateLeanValue(float TurnValue);
	//----------------------------------

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	class UAnimManager* GetAnimManager() const { return AnimManager; }

	UFUNCTION(BlueprintCallable)
		void ResetRotation();

	UPROPERTY(BlueprintAssignable)
		FGetLeanAmount GetLaneAmountDelegate;

	//Warning: Link this to AnyDamage node in BP
	UFUNCTION(BlueprintCallable, meta = (ExpandEnumAsExecs = Outp))
		void Exec_TryGetHit(float Damage, class UDamageType const* UDamageType, AController* EventInstigator, AActor* DamageCauser, EOnHitRefelction &Outp);
};

