#include "ShootingCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "../FirstPlayer/FirstPlayerProjectile.h"
#include "Kismet/GameplayStatics.h"

//////////////////////////////////////////////////////////////////////////
// AShootingCharacter

AShootingCharacter::AShootingCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

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

	// Set Default Mesh
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -97.f), FRotator(0.f, -90.f, 0.f));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SM(TEXT("SkeletalMesh'/Game/AnimStarterPack/UE4_Mannequin/Mesh/SK_Mannequin.SK_Mannequin'"));
	if (SM.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(SM.Object); 
	}
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	static ConstructorHelpers::FObjectFinder<UClass> AB(TEXT("AnimBlueprint'/Game/AnimStarterPack/ABP_ShootAnim.ABP_ShootAnim_C'"));
	if (AB.Succeeded())
	{
		GetMesh()->SetAnimClass(AB.Object);
	}

#pragma region FPS
    // Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(DefaultFPCameraPos); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> SM_arm(TEXT("SkeletalMesh'/Game/FirstPerson/Character/Mesh/SK_Mannequin_Arms.SK_Mannequin_Arms'"));
	if (SM_arm.Succeeded())
	{
		Mesh1P->SetSkeletalMesh(SM_arm.Object); 
	}
    Mesh1P->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	static ConstructorHelpers::FObjectFinder<UClass> AB_arm(TEXT("AnimBlueprint'/Game/FirstPerson/Animations/FirstPerson_AnimBP.FirstPerson_AnimBP_C'"));
	if (AB_arm.Succeeded())
	{
		Mesh1P->SetAnimClass(AB_arm.Object);
	}

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SM_gun(TEXT("SkeletalMesh'/Game/FirstPerson/FPWeapon/Mesh/SK_FPGun.SK_FPGun'"));
	if (SM_gun.Succeeded())
	{
		FP_Gun->SetSkeletalMesh(SM_gun.Object); 
	}

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	static ConstructorHelpers::FClassFinder<AFirstPlayerProjectile> PR(TEXT("Blueprint'/Game/FirstPersonCPP/Blueprints/FirstPersonProjectile.FirstPersonProjectile_C'"));
	if (PR.Succeeded())
	{
		ProjectileClass = PR.Class;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> FS(TEXT("SoundWave'/Game/FirstPerson/Audio/FirstPersonTemplateWeaponFire02.FirstPersonTemplateWeaponFire02'"));
	if (FS.Succeeded())
	{
		FireSound = FS.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> FA(TEXT("AnimMontage'/Game/FirstPerson/Animations/FirstPersonFire_Montage.FirstPersonFire_Montage'"));
	if (FA.Succeeded())
	{
		FireAnimation = FA.Object;
	}
#pragma endregion
}

void AShootingCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	OnFPCamera = true;
	SetActiveCamera();
}

void AShootingCharacter::SetActiveCamera()
{
	Mesh1P->SetHiddenInGame(!OnFPCamera, false);
	GetMesh()->SetHiddenInGame(OnFPCamera, false);

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(OnFPCamera ? Mesh1P : GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	if (OnFPCamera)
	{
		FirstPersonCameraComponent->Activate();
		FollowCamera->Deactivate();
	}
	else
	{
		FirstPersonCameraComponent->Deactivate();
		FollowCamera->Activate();
	}
}

void AShootingCharacter::OnChangeCamera() 
{
	if (PronePressed || GetCharacterMovement()->MovementMode == MOVE_None)
		return;
	OnFPCamera = !OnFPCamera;
	SetActiveCamera(); 
}

void AShootingCharacter::OnFire() 
{
    // try and fire a projectile
	if (ProjectileClass != nullptr)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			const FRotator SpawnRotation = GetControlRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

			// spawn the projectile at the muzzle
			World->SpawnActor<AFirstPlayerProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		}
	}

	// try and play the sound if specified
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
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

    // Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AShootingCharacter::OnFire);

	PlayerInputComponent->BindAction("CameraChange", IE_Pressed, this, &AShootingCharacter::OnChangeCamera);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShootingCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShootingCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShootingCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShootingCharacter::LookUpAtRate);
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
		const FRotator Rotation = FollowCamera->GetComponentTransform().Rotator();//Controller->GetControlRotation();
		const FRotator YawRotation(0, 0, Rotation.Yaw);
		const FVector Direction = FollowCamera->GetForwardVector();//FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
		
        //FPS
        //AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AShootingCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = UKismetMathLibrary::GetRightVector(FRotator(0.0f, Rotation.Yaw, 0.0f));//FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		//if (GetCharacterMovement()->MovementMode != MOVE_None)
		AddMovementInput(Direction, Value);
		//AddMovementInput(FollowCamera->GetRightVector(), Value);

        //FPS
        //AddMovementInput(GetActorRightVector(), Value);
	}
}

void AShootingCharacter::PressCrouch()
{
	//CanCrouch() ? Crouch() : UnCrouch();

	if (!PronePressed)
	{
		CrouchPressed = !CrouchPressed;

		if (CrouchPressed)
		{
			FirstPersonCameraComponent->SetRelativeLocation(CrouchedFPCameraPos); // Position the camera
			GetCharacterMovement()->MaxWalkSpeed = 160.f;
		}
		else
		{
			FirstPersonCameraComponent->SetRelativeLocation(DefaultFPCameraPos); // Position the camera
			GetCharacterMovement()->MaxWalkSpeed = JogPressed ? 375.f : 200.f;
		}
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
	if (!CrouchPressed && !JumpPressed && !OnFPCamera)
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
