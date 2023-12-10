// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class MARIOLONGJUMP_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Textblock to display movement info */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* CurrentMovement;

	/*
	* Setter function to allow updates to the textblocks in the HUD
	*/
	UFUNCTION()
	void SetCurrentMovement(FText Movement);
};
