// Fill out your copyright notice in the Description page of Project Settings.

#include "SHUD_Widget.h"
#include "SCharacter.h"
#include "Components/SHealthComponent.h"

void USHUD_Widget::NativeConstruct()
{
	/** sets owner reference  */
	WidgetOwnerCharacter = Cast<ASCharacter>(GetOwningPlayerPawn());

	if (WidgetOwnerCharacter)
	{
		HealthComp = WidgetOwnerCharacter->GetHealthComp();
	}

	Super::NativeConstruct();
}

void USHUD_Widget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateHUD();
}

void USHUD_Widget::UpdateHUD()
{
	if (WidgetOwnerCharacter)
	{
		/** retrieve all data from owner character  */
		WidgetOwnerCharacter->GetHUDData(AmmoText, ScoreText, PingText, HealthPercent);

		if (HealthComp)
		{
			float Health = HealthComp->GetHealth();
			float HealthDelta = HealthComp->GetDefaultHealth();
		}
	}
}