// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SHUD_Widget.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API USHUD_Widget : public UUserWidget
{
	GENERATED_BODY()
	
private:
	
	/** call to update HUD to current state  */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateHUD();

	/** calls when widget is constructed  */
	virtual void NativeConstruct() override;

	/** call every frame  */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	/** widget owner character reference  */
	UPROPERTY()
	class ASCharacter* WidgetOwnerCharacter;

	/**   */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	class USHealthComponent* HealthComp;
	
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	FText AmmoText;
	
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	FText ScoreText;

	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	FText PingText;

	/** health bar percent reference  */
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	float HealthPercent;
	
};
