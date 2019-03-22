// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "FPSCharacterMoveComponent.generated.h"

class AFPSCharacter;
class UFPSCharacterCollisionComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class AFPS_API UFPSCharacterMoveComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

public:
	UFPSCharacterMoveComponent();

	//Character this belongs to
	AFPSCharacter* Player;

	// Collision component
	UFPSCharacterCollisionComponent* CollisionComponent;

public:
	// Called every frame
	//virtual void TickComponent(float DeltaTime) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

public:
	float Delta;

protected:
	// Contains the command the user wishes upon the character
	struct Inputs
	{
		float forwardMove;
		float rightMove;
		float upMove;
	};

public:
	float gravity = 20.0f;      // Gravity
	float friction = 6;         // Ground friction

	// Q3: players can queue the next jump just before he hits the ground
	bool wishJump = false;

	// Used to display real time friction values
	float playerFriction = 0.0f;

	// Player commands
	Inputs _inputs;

	FVector moveDirectionNorm = FVector::ZeroVector;		//FVector(0.0f, 0.0f, 0.0f);
	FVector playerVelocity = FVector::ZeroVector;		
	float playerTopVelocity = 0.0f;

	float moveSpeed = 7.0f;                // Ground move speed
	float runAcceleration = 14.0f;         // Ground accel
	float runDeacceleration = 10.0f;       // Deacceleration that occurs when running on the ground
	float airAcceleration = 2.0f;          // Air accel
	float airDecceleration = 2.0f;         // Deacceleration experienced when ooposite strafing
	float airControl = 0.3f;               // How precise air control is
	float sideStrafeAcceleration = 50.0f;  // How fast acceleration occurs to get up to sideStrafeSpeed when
	float sideStrafeSpeed = 1.0f;          // What the max speed to generate when side strafing
	float jumpSpeed = 8.0f;                // The speed at which the character's up axis gains when hitting jump
	float moveScale = 1.0f;

	bool m_PreviouslyGrounded = true;

protected:
	void SetMovementDir();
	void QueueJump();
	void GroundMove();
	void AirMove();
	void AirControl(FVector wishdir, float wishspeed);
	void ApplyFriction(float t);
	void Accelerate(FVector wishdir, float wishspeed, float accel);
	float InputScale();
	
};
