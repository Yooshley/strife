// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"

#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"

void UOverheadWidget::SetDisplayText(FString Display)
{
	if(DisplayText)
	{
		DisplayText->SetText(FText::FromString(Display));
	}
}

void UOverheadWidget::ShowPlayerName(APawn* InPawn)
{
	const FString PlayerName = InPawn->GetName();
	const FString PlayerNameString = FString::Printf(TEXT("%s"), *PlayerName);
	SetDisplayText(PlayerNameString);
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	const ENetRole RemoteRole = InPawn->GetRemoteRole();
	FString Role;
	switch (RemoteRole)
	{
	case ENetRole::ROLE_Authority:
		Role = FString("Authority");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		Role = FString("AutonomousProxy");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Role = FString("SimulatedProxy");
		break;
	case ENetRole::ROLE_None:
		Role = FString("None");
		break;
	default:
		break;
	}
	const FString RemoteRoleString = FString::Printf(TEXT("Local Role: %s"), *Role);
	SetDisplayText(RemoteRoleString);
}

void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();
	Super::NativeDestruct();
}
