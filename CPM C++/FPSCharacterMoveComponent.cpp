// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSCharacterMoveComponent.h"
#include "AFPS.h"
#include "FPSCharacter.h"
#include "FPSCharacterCollisionComponent.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformMath.h"

#include "Engine.h"

#include "Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h"	// For ground check
#include "Runtime/Core/Public/Misc/App.h"	//Deltatime


// Notes:
// Set kinematic?
// Calculations might not work exactly like in Unity


// Sets Component`s default properties
UFPSCharacterMoveComponent::UFPSCharacterMoveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called every frame
//void UFPSCharacterMoveComponent::TickComponent(float DeltaTime)
void UFPSCharacterMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	//Super::Tick(DeltaTime);

	Delta = DeltaTime;

	/*
	if (!Player)
	{
		GEngine->AddOnScreenDebugMessage(0, 0.01f, FColor::Red, TEXT("NO OWNER"));
		return;
	}
	*/

	QueueJump();

	// Get input from the player
	//WishMove = Player->ConsumeMovementInput();				// Player reg? pawn?

	CollisionComponent->TraceGround();

	//if (GetCharacterMovement()->IsFalling())
	if (CollisionComponent->CanGroundMove)			// On the ground. Colud also use default movement component just for ground check etc.
	{
		GroundMove();
	}
	else
	{
		AirMove();
	}

	//CollisionComponent->TraceGround();
	//Player->Collider->SetWorldLocation(Origin);




	// Move the player. vector, scale			//In examples movement is vector + input -1 or 1 to change direction
	//AddMovementInput(playerVelocity);		//Q3 movement physics calculate movement vector based on players input & current velocity


	/* In C# Unity				moving controller different from movementinput?
	// Move the controller
		_controller.Move(playerVelocity * Time.deltaTime);
	*/
}

void UFPSCharacterMoveComponent::SetMovementDir()
{
	// Movement inputs
	//_inputs.forwardMove = PLAYER->ConsumeMovementInput();
	//_inputs.rightMove = PLAYER->ConsumeMovementInput();
}

void UFPSCharacterMoveComponent::QueueJump()
{
	/*
	if (Input.GetButtonDown("Jump") && !wishJump)
	{
		wishJump = true;
	}
	if (Input.GetButtonUp("Jump"))
	{
		wishJump = false;
	}
	*/
}

void UFPSCharacterMoveComponent::GroundMove()
{
	FVector wishdir;

	// Do not apply friction if the player is queueing up the next jump
	if (!wishJump)
	{
		ApplyFriction(1.0f);
	}
	else
	{
		ApplyFriction(0);
	}

	SetMovementDir();

	wishdir = FVector(_inputs.rightMove, 0, _inputs.forwardMove);
	//wishdir = transform.TransformDirection(wishdir);					//Unity: Transforms direction from local space to world space.
	wishdir = wishdir + GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();		// LocalVector + LocalObjectWorldLocation
	wishdir.Normalize();
	moveDirectionNorm = wishdir;

	//var wishspeed = wishdir.magnitude;				// Unity .magnitude is vector length, UE4 .Size
	float wishspeed = wishdir.Size();
	wishspeed *= moveSpeed;

	Accelerate(wishdir, wishspeed, runAcceleration);

	// Reset the gravity velocity
	playerVelocity.Y = -gravity * Delta;	//*GetWorld()->GetDeltaSeconds();

	if (wishJump)
	{
		// Reset the gravity velocity
		playerVelocity.Y = jumpSpeed;

		//PlayJumpSound();
		wishJump = false;
	}
}

void UFPSCharacterMoveComponent::AirMove()
{
	FVector wishdir;
	float wishvel = airAcceleration;
	float accel;

	SetMovementDir();

	wishdir = FVector(_inputs.rightMove, 0, _inputs.forwardMove);
	//wishdir = transform.TransformDirection(wishdir);
	wishdir = wishdir + GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	float wishspeed = wishdir.Size();
	wishspeed *= moveSpeed;

	wishdir.Normalize();
	moveDirectionNorm = wishdir;
	//wishspeed *= scale;

	// CPM: Aircontrol
	float wishspeed2 = wishspeed;
	//if (Vector3.Dot(playerVelocity, wishdir) < 0)				// Unity: .Dot The dot product is a float value equal to the magnitudes of the two vectors multiplied together and then multiplied by the cosine of the angle between them.
	if (FVector::DotProduct(playerVelocity, wishdir) < 0)
		accel = airDecceleration;
	else
		accel = airAcceleration;
	// If the player is ONLY strafing left or right
	if (_inputs.forwardMove == 0 && _inputs.rightMove != 0)
	{
		if (wishspeed > sideStrafeSpeed)
			wishspeed = sideStrafeSpeed;
		accel = sideStrafeAcceleration;
	}

	Accelerate(wishdir, wishspeed, accel);
	if (airControl > 0)
		AirControl(wishdir, wishspeed2);
	// !CPM: Aircontrol

	// Apply gravity
	playerVelocity.Y -= gravity * Delta;		// Unity: Time.deltaTime --> UE4: GetWorld()->GetDeltaSeconds()
}

void UFPSCharacterMoveComponent::AirControl(FVector wishdir, float wishspeed)
{
	float zspeed;
	float speed;
	float dot;
	float k;

	// Can't control movement if not moving forward or backward
	if (FGenericPlatformMath::Abs(_inputs.forwardMove) < 0.001 || FGenericPlatformMath::Abs(wishspeed) < 0.001)					// old abs()
		return;
	zspeed = playerVelocity.Y;
	playerVelocity.Y = 0;
	/* Next two lines are equivalent to idTech's VectorNormalize() */
	speed = playerVelocity.Size();
	playerVelocity.Normalize();

	dot = FVector::DotProduct(playerVelocity, wishdir);
	k = 32;
	k *= airControl * dot * dot * GetWorld()->GetDeltaSeconds();

	// Change direction while slowing down
	if (dot > 0)
	{
		playerVelocity.X = playerVelocity.X * speed + wishdir.X * k;
		playerVelocity.Y = playerVelocity.Y * speed + wishdir.Y * k;
		playerVelocity.Z = playerVelocity.Z * speed + wishdir.Z * k;

		playerVelocity.Normalize();
		moveDirectionNorm = playerVelocity;
	}

	playerVelocity.X *= speed;
	playerVelocity.Y = zspeed; // Note this line
	playerVelocity.Z *= speed;
}

void UFPSCharacterMoveComponent::ApplyFriction(float t)
{
	FVector vec = playerVelocity; // Equivalent to: VectorCopy();
	float speed;
	float newspeed;
	float control;
	float drop;

	vec.Y = 0.0f;
	speed = vec.Size();
	drop = 0.0f;

	/* Only if the player is on the ground then apply friction */
	if (CollisionComponent->CanGroundMove)																		//////
	{
		control = speed < runDeacceleration ? runDeacceleration : speed;
		drop = control * friction * Delta * t;
	}

	newspeed = speed - drop;
	playerFriction = newspeed;
	if (newspeed < 0)
		newspeed = 0;
	if (speed > 0)
		newspeed /= speed;

	playerVelocity.X *= newspeed;
	playerVelocity.Z *= newspeed;
}

void UFPSCharacterMoveComponent::Accelerate(FVector wishdir, float wishspeed, float accel)
{
	float addspeed;
	float accelspeed;
	float currentspeed;

	currentspeed = FVector::DotProduct(playerVelocity, wishdir);
	addspeed = wishspeed - currentspeed;
	if (addspeed <= 0)
		return;
	accelspeed = accel * Delta * wishspeed;
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	playerVelocity.X += accelspeed * wishdir.X;
	playerVelocity.Z += accelspeed * wishdir.Z;
}

float UFPSCharacterMoveComponent::InputScale()
{
	int max;
	float total;
	float scale;

	max = (int)FGenericPlatformMath::Abs(_inputs.forwardMove);
	if (FGenericPlatformMath::Abs(_inputs.rightMove) > max)
		max = (int)FGenericPlatformMath::Abs(_inputs.rightMove);
	if (max <= 0)
		return 0;

	total = sqrt(_inputs.forwardMove * _inputs.forwardMove + _inputs.rightMove * _inputs.rightMove);
	scale = moveSpeed * max / (moveScale * total);

	return scale;
}