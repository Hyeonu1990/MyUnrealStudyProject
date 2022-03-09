#include "ShootingCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"

//////////////////////////////////////////////////////////////////////////
// AShootingCharacter

AShootingCharacter::AShootingCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bIgnoreBaseRotation = true;
	GetCharacterMovement()->MaxWalkSpeed = 200;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 160;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetRelativeLocation(FVector(0, 0, 50.f));
	CameraBoom->SocketOffset = FVector(0, 0, 30.f);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void AShootingCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShootingCharacter::PressJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AShootingCharacter::ReleaseJump);
	
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShootingCharacter::PressCrouch);

	PlayerInputComponent->BindAction("Jog", IE_Pressed, this, &AShootingCharacter::PressJog);
	PlayerInputComponent->BindAction("Jog", IE_Released, this, &AShootingCharacter::ReleaseJog);

	PlayerInputComponent->BindAction("Prone", IE_Pressed, this, &AShootingCharacter::PressProne);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShootingCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShootingCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShootingCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShootingCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AShootingCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AShootingCharacter::TouchStopped);
}

void AShootingCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AShootingCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AShootingCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShootingCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShootingCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// // find out which way is forward
		// const FRotator Rotation = Controller->GetControlRotation();
		// const FRotator YawRotation(0, Rotation.Yaw, 0);
		
		// // get forward vector
		// const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		
		// AddMovementInput(Direction, Value);
		
		const FRotator Rotation = FollowCamera->GetComponentTransform().Rotator();//Controller->GetControlRotation();
		const FRotator YawRotation(0, 0, Rotation.Yaw);
		const FVector Direction = FollowCamera->GetForwardVector();//FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AShootingCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// // find out which way is right
		// const FRotator Rotation = Controller->GetControlRotation();
		// const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// // get right vector 
		// const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// // add movement in that direction
		// AddMovementInput(Direction, Value);

		const FRotator Rotation = FollowCamera->GetComponentTransform().Rotator();//Controller->GetControlRotation();
		const FRotator YawRotation(0, 0, Rotation.Yaw);
		const FVector Direction = FollowCamera->GetRightVector();//FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		//if (GetCharacterMovement()->MovementMode != MOVE_None)
		AddMovementInput(Direction, Value);
		//AddMovementInput(FollowCamera->GetRightVector(), Value);
	}
}

void AShootingCharacter::PressCrouch()
{
	//CanCrouch() ? Crouch() : UnCrouch();

	if (!PronePressed)
	{
		CrouchPressed = !CrouchPressed;

		if (CrouchPressed)
			GetCharacterMovement()->MaxWalkSpeed = 160.f;
		else
			GetCharacterMovement()->MaxWalkSpeed = JogPressed ? 375.f : 200.f;
	}
}

void AShootingCharacter::PressJog()
{
	JogPressed = true;
	GetCharacterMovement()->MaxWalkSpeed = 375.f;
}

void AShootingCharacter::ReleaseJog()
{
	JogPressed = false;
	GetCharacterMovement()->MaxWalkSpeed = CrouchPressed ? 160.f : 200.f;
}

void AShootingCharacter::PressProne()
{
	if (!CrouchPressed && !JumpPressed)
	{
		if (!PronePressed && GetCharacterMovement()->MovementMode != MOVE_None)
		{
			PronePressed = true;
			GetCharacterMovement()->SetMovementMode(MOVE_None);
		}
		else if (PronePressed && GetCharacterMovement()->MovementMode == MOVE_None)
		{
			PronePressed = false;
			FTimerHandle WaitHandle;
			GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
			{
				GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			}), 1.44f, false);
		}
	}
}

void AShootingCharacter::PressJump()
{
	if (GetCharacterMovement()->MovementMode != MOVE_None)
	{
		if (CrouchPressed)// || GetCharacterMovement()->MaxWalkSpeed > 160.f)
			PressCrouch();
		ACharacter::Jump();
		GetCharacterMovement()->MaxWalkSpeed = (GetCharacterMovement()->Velocity.Size() > 0) ? 365.f : 340.f;
		JumpPressed = true;
		
	}
}

void AShootingCharacter::ReleaseJump()
{
	ACharacter::StopJumping();
	JumpPressed = false;
}
