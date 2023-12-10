// Fill out your copyright notice in the Description page of Project Settings.


#include "MarioLongJumpHUD.h"

AMarioLongJumpHUD::AMarioLongJumpHUD()
{
	SetActorTickEnabled(false);

	HUDWidgetClass = nullptr;
	HUDWidget = nullptr;
}

UHUDWidget* AMarioLongJumpHUD::GetHUDWidget()
{
	return HUDWidget;
}

void AMarioLongJumpHUD::BeginPlay()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void AMarioLongJumpHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	// Creates new Widget from HUDWidgetClass
	if (HUDWidgetClass != nullptr)
	{
		HUDWidget = CreateWidget<UHUDWidget>(GetWorld(), HUDWidgetClass);
		HUDWidget->AddToViewport();
	}
}
