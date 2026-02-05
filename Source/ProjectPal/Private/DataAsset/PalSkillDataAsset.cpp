// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/PalSkillDataAsset.h"
#include "Curves/CurveFloat.h"

float FPalSkillDamage::GetDamage(int32 Level) const
{
	if (DamageByLevelCurve)
	{
		return DamageByLevelCurve->GetFloatValue((float)Level);
	}
	return BaseDamage;
}

float UPalSkillDataAsset::GetCooldown(int32 Level) const
{
	if (CooldownByLevelCurve)
	{
		return CooldownByLevelCurve->GetFloatValue((float)Level);
	}
	return Timing.Cooldown;
}

float UPalSkillDataAsset::GetCastRange(int32 Level) const
{
	if (CastRangeByLevelCurve)
	{
		return CastRangeByLevelCurve->GetFloatValue((float)Level);
	}
	return Activation.CastRange;
}
