// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/SoulDamageExecution.h"
#include "Abilities/SoulAttributeSet.h"
#include "SoulCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "BPFL/BPFL_Math.h"

struct SoulDamageStatics
{
    //For Target
    DECLARE_ATTRIBUTE_CAPTUREDEF(DefensePower);
    DECLARE_ATTRIBUTE_CAPTUREDEF(PostureStrength);

    //For Source -> Health Damage
    DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);
    DECLARE_ATTRIBUTE_CAPTUREDEF(Damage);
    DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalStrike);
    DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalMulti);

    //For Source -> Posture Damage
    DECLARE_ATTRIBUTE_CAPTUREDEF(PostureDamage);
    DECLARE_ATTRIBUTE_CAPTUREDEF(PostureCrumble);


    SoulDamageStatics()
    {
        // Capture the Target's DefensePower attribute. Do not snapshot it, because we want to use the health value at the moment we apply the execution.
        DEFINE_ATTRIBUTE_CAPTUREDEF(USoulAttributeSet, DefensePower, Target, false);
        DEFINE_ATTRIBUTE_CAPTUREDEF(USoulAttributeSet, PostureStrength, Target, false);

        // Capture the Source's AttackPower. We do want to snapshot this at the moment we create the GameplayEffectSpec that will execute the damage.
        // (imagine we fire a projectile: we create the GE Spec when the projectile is fired. When it hits the target, we want to use the AttackPower at the moment
        // the projectile was launched, not when it hits).
        DEFINE_ATTRIBUTE_CAPTUREDEF(USoulAttributeSet, Damage, Source, true);
        DEFINE_ATTRIBUTE_CAPTUREDEF(USoulAttributeSet, AttackPower, Source, true);
        DEFINE_ATTRIBUTE_CAPTUREDEF(USoulAttributeSet, CriticalStrike, Source, true);
        DEFINE_ATTRIBUTE_CAPTUREDEF(USoulAttributeSet, CriticalMulti, Source, true);

        // Also capture the source's raw Damage, which is normally passed in directly via the execution
        DEFINE_ATTRIBUTE_CAPTUREDEF(USoulAttributeSet, PostureDamage, Source, true);
        DEFINE_ATTRIBUTE_CAPTUREDEF(USoulAttributeSet, PostureCrumble, Source, true);
    }
};

static const SoulDamageStatics& DamageStatics()
{
    static SoulDamageStatics DmgStatics;
    return DmgStatics;
}


USoulDamageExecution::USoulDamageExecution()
{
    RelevantAttributesToCapture.Add(DamageStatics().PostureStrengthDef);
    RelevantAttributesToCapture.Add(DamageStatics().DamageDef);

    RelevantAttributesToCapture.Add(DamageStatics().AttackPowerDef);
    RelevantAttributesToCapture.Add(DamageStatics().CriticalStrikeDef);
    RelevantAttributesToCapture.Add(DamageStatics().CriticalMultiDef);

    RelevantAttributesToCapture.Add(DamageStatics().PostureDamageDef);
    RelevantAttributesToCapture.Add(DamageStatics().PostureCrumbleDef);
}

USoulReflectionDamageExecution::USoulReflectionDamageExecution()
{
    RelevantAttributesToCapture.Add(DamageStatics().PostureStrengthDef);
    RelevantAttributesToCapture.Add(DamageStatics().DamageDef);

    RelevantAttributesToCapture.Add(DamageStatics().AttackPowerDef);
    RelevantAttributesToCapture.Add(DamageStatics().CriticalStrikeDef);
    RelevantAttributesToCapture.Add(DamageStatics().CriticalMultiDef);

    RelevantAttributesToCapture.Add(DamageStatics().PostureDamageDef);
    RelevantAttributesToCapture.Add(DamageStatics().PostureCrumbleDef);
}

void USoulReflectionDamageExecution::Execute_Implementation(
    const FGameplayEffectCustomExecutionParameters& ExecutionParams,
    FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
 UAbilitySystemComponent* TargetAbilitySystemComponent = ExecutionParams.GetTargetAbilitySystemComponent();
    UAbilitySystemComponent* SourceAbilitySystemComponent = ExecutionParams.GetSourceAbilitySystemComponent();

    AActor* SourceActor = SourceAbilitySystemComponent ? SourceAbilitySystemComponent->AvatarActor : nullptr;
    AActor* TargetActor = TargetAbilitySystemComponent ? TargetAbilitySystemComponent->AvatarActor : nullptr;

    //Warning: it's non-static. Be careful when modify the GE
    FGameplayEffectSpec* Spec = ExecutionParams.GetOwningSpecForPreExecuteMod();

    // Gather the tags from the source and target as that can affect which buffs should be used
    const FGameplayTagContainer* SourceTags = Spec->CapturedSourceTags.GetAggregatedTags();
    const FGameplayTagContainer* TargetTags = Spec->CapturedTargetTags.GetAggregatedTags();

    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = SourceTags;
    EvaluationParameters.TargetTags = TargetTags;

    // --------------------------------------
    //	Damage Done = (Damage + AP) * (bCritical ? (1 + CriticalMulti ; 1) - DP
    // --------------------------------------

    float DefensePower = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DefensePowerDef, EvaluationParameters,
                                                               DefensePower);
    float PostureStrength = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().PostureStrengthDef, EvaluationParameters,
                                                               PostureStrength);

    float DamageMulti = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DamageDef, EvaluationParameters,
                                                               DamageMulti);
    float AttackPower = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackPowerDef, EvaluationParameters,
                                                               AttackPower);
    float CriticalStrike = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalStrikeDef, EvaluationParameters,
                                                               CriticalStrike);
    float CriticalMulti = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalMultiDef, EvaluationParameters,
                                                               CriticalMulti);

    float PostureMulti = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().PostureDamageDef, EvaluationParameters,
                                                               PostureMulti);
    float PostureCrumble = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().PostureCrumbleDef, EvaluationParameters,
                                                               PostureCrumble);

    //HEALTH DAMAGE
    float DamageDone = (DamageMulti + 1.f) * AttackPower;

    //Defense calculation
    DamageDone *= (DamageDone / (DamageDone + DefensePower));

    //POSTURE DAMAGE
    const float PostureCrumbleFinal = PostureCrumble + 10.f;
    float PostureDamageDone = (1.f + PostureMulti) * PostureCrumbleFinal * (PostureCrumbleFinal / (PostureCrumbleFinal +
        PostureStrength));

    //If source actor is perfectly parrying -> reflect 1x damage and posture damage and proc stun
    //If normal parrying -> reflect .4x
    //else .25x
    if (SourceTags->HasTagExact(FGameplayTag::RequestGameplayTag(FName{"Buffer.Parry.Perfect"}, true)))
    {
        Spec->DynamicAssetTags.AddTagFast(FGameplayTag::RequestGameplayTag(FName{"Damage.Stun"}, true));

    }
    else if (SourceTags->HasTagExact(FGameplayTag::RequestGameplayTag(FName{"Buffer.Parry.Normal"}, true)))
    {
        PostureDamageDone *= .4f;
        DamageDone *= .4f;
    }
    else
    {
        PostureDamageDone *= .25f;
        DamageDone *= .25f;
    }

    //Passed the critical tag to the gameplay effect spec
    //We shall see that when the change of the Damage is passed to the target's AttriuteSet

    (Cast<ASoulCharacterBase>(SourceActor))->
       Notify_OnMeleeAttack(TargetActor, Spec->GetContext().GetHitResult() ? *(Spec->GetContext().GetHitResult()) : FHitResult());

    OutExecutionOutput.AddOutputModifier(
        FGameplayModifierEvaluatedData(DamageStatics().DamageProperty, EGameplayModOp::Additive, DamageDone));
    OutExecutionOutput.AddOutputModifier(
        FGameplayModifierEvaluatedData(DamageStatics().PostureDamageProperty, EGameplayModOp::Additive,
                                       PostureDamageDone));
}

USoulDotDamageExecution::USoulDotDamageExecution()
{
    RelevantAttributesToCapture.Add(DamageStatics().DamageDef);
    RelevantAttributesToCapture.Add(DamageStatics().AttackPowerDef);
}

void USoulDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                                  OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    UAbilitySystemComponent* TargetAbilitySystemComponent = ExecutionParams.GetTargetAbilitySystemComponent();
    UAbilitySystemComponent* SourceAbilitySystemComponent = ExecutionParams.GetSourceAbilitySystemComponent();

    AActor* SourceActor = SourceAbilitySystemComponent ? SourceAbilitySystemComponent->AvatarActor : nullptr;
    AActor* TargetActor = TargetAbilitySystemComponent ? TargetAbilitySystemComponent->AvatarActor : nullptr;

    //Warning: it's non-static. Be careful when modify the GE
    FGameplayEffectSpec* Spec = ExecutionParams.GetOwningSpecForPreExecuteMod();

    // Gather the tags from the source and target as that can affect which buffs should be used
    const FGameplayTagContainer* SourceTags = Spec->CapturedSourceTags.GetAggregatedTags();
    const FGameplayTagContainer* TargetTags = Spec->CapturedTargetTags.GetAggregatedTags();

    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = SourceTags;
    EvaluationParameters.TargetTags = TargetTags;

    // --------------------------------------
    //	Damage Done = (Damage + AP) * (bCritical ? (1 + CriticalMulti ; 1) - DP
    // --------------------------------------

    float DefensePower = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DefensePowerDef, EvaluationParameters,
                                                               DefensePower);
    float PostureStrength = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().PostureStrengthDef, EvaluationParameters,
                                                               PostureStrength);

    float DamageMulti = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DamageDef, EvaluationParameters,
                                                               DamageMulti);
    float AttackPower = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackPowerDef, EvaluationParameters,
                                                               AttackPower);
    float CriticalStrike = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalStrikeDef, EvaluationParameters,
                                                               CriticalStrike);
    float CriticalMulti = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalMultiDef, EvaluationParameters,
                                                               CriticalMulti);

    float PostureMulti = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().PostureDamageDef, EvaluationParameters,
                                                               PostureMulti);
    float PostureCrumble = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().PostureCrumbleDef, EvaluationParameters,
                                                               PostureCrumble);

    //Check whether it's a crit strike from source
    //TODO: "Can't crit" tag to prevent crit triggered
    const int32 TempCritRoll = FMath::RandRange(0, 100);
    const bool bIsCrit = CriticalStrike >= TempCritRoll ? true : false;

    //HEALTH DAMAGE
    float DamageDone = (DamageMulti + 1.f) * AttackPower;

    if (CriticalStrike >= TempCritRoll)
    {
        Spec->DynamicAssetTags.AddTagFast(FGameplayTag::RequestGameplayTag(FName{"Damage.Critical"}, true));
        DamageDone *=  (1 + CriticalMulti / 100.f);
    }

    //Defense calculation
    DamageDone *= (DamageDone / (DamageDone + DefensePower));

    //POSTURE DAMAGE
    const float PostureCrumbleFinal = PostureCrumble + 10.f;
    float PostureDamageDone = (1.f + PostureMulti) * PostureCrumbleFinal * (PostureCrumbleFinal / (PostureCrumbleFinal +
        PostureStrength));


    float CuttingAngle = 0.f;
    UBPFL_Math::FindDegreeToTarget(TargetActor, SourceActor, CuttingAngle);
    //GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::SanitizeFloat(CuttingAngle));

    if (TargetTags->HasTag(FGameplayTag::RequestGameplayTag(FName{"Buffer.Parry"}, true)) && FMath::Abs(CuttingAngle) <
        90.f)
    {
        if (TargetTags->HasTagExact(FGameplayTag::RequestGameplayTag(FName{"Buffer.Parry.Perfect"}, true)))
        {
            //Warning: Pass the tag through the GE, just in case the Parry's GA ends before the Notify_OnMeleeAttack is triggered;
            Spec->DynamicAssetTags.AddTagFast(FGameplayTag::RequestGameplayTag(FName{"Buffer.Parry.Perfect"}, true));

            PostureDamageDone = DamageDone = 0.f;
        }
        else if (TargetTags->HasTagExact(FGameplayTag::RequestGameplayTag(FName{"Buffer.Parry.Normal"}, true)))
        {
            Spec->DynamicAssetTags.AddTagFast(FGameplayTag::RequestGameplayTag(FName{"Buffer.Parry.Normal"}, true));

            DamageDone *= 0.25f;
            PostureDamageDone *= .25f;
        }
    }
    else if (TargetTags->HasTagExact(FGameplayTag::RequestGameplayTag(FName{"Buffer.Dodge"}, true)))
    {
        //GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "Buffer.Dodge");

        Spec->DynamicAssetTags.AddTagFast(FGameplayTag::RequestGameplayTag(FName{"Buffer.Dodge"}, true));

        PostureDamageDone = DamageDone = 0.f;
    }
    else if (!Spec->DynamicAssetTags.HasTagExact(FGameplayTag::RequestGameplayTag(FName{"Damage.Stun"}, true)))
    {
        Spec->DynamicAssetTags.AddTagFast(FGameplayTag::RequestGameplayTag(FName{"Damage.Stun"}, true));
    }

    //Passed the critical tag to the gameplay effect spec
    //We shall see that when the change of the Damage is passed to the target's AttriuteSet

    (Cast<ASoulCharacterBase>(SourceActor))->
       Notify_OnMeleeAttack(TargetActor, Spec->GetContext().GetHitResult() ? *(Spec->GetContext().GetHitResult()) : FHitResult());

    OutExecutionOutput.AddOutputModifier(
        FGameplayModifierEvaluatedData(DamageStatics().DamageProperty, EGameplayModOp::Additive, DamageDone));
    OutExecutionOutput.AddOutputModifier(
        FGameplayModifierEvaluatedData(DamageStatics().PostureDamageProperty, EGameplayModOp::Additive,
                                       PostureDamageDone));
}



void USoulDotDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                                     OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    UAbilitySystemComponent* TargetAbilitySystemComponent = ExecutionParams.GetTargetAbilitySystemComponent();
    UAbilitySystemComponent* SourceAbilitySystemComponent = ExecutionParams.GetSourceAbilitySystemComponent();

    AActor* SourceActor = SourceAbilitySystemComponent ? SourceAbilitySystemComponent->AvatarActor : nullptr;
    AActor* TargetActor = TargetAbilitySystemComponent ? TargetAbilitySystemComponent->AvatarActor : nullptr;


    //Warning: it's non-static. Be careful when modify the GE
    FGameplayEffectSpec* Spec = ExecutionParams.GetOwningSpecForPreExecuteMod();

    // Gather the tags from the source and target as that can affect which buffs should be used
    const FGameplayTagContainer* SourceTags = Spec->CapturedSourceTags.GetAggregatedTags();
    const FGameplayTagContainer* TargetTags = Spec->CapturedTargetTags.GetAggregatedTags();

    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = SourceTags;
    EvaluationParameters.TargetTags = TargetTags;

    float DefensePower = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DefensePowerDef, EvaluationParameters,
                                                               DefensePower);

    float DamageMulti = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DamageDef, EvaluationParameters,
                                                               DamageMulti);
    float AttackPower = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackPowerDef, EvaluationParameters,
                                                               AttackPower);

    float DamageDone = 0.f;
    DamageDone = (DamageMulti + 1.f) * AttackPower;

    DamageDone *= (DamageDone / (DamageDone + DefensePower));

    if (DamageDone >= 0.f)
    {
        OutExecutionOutput.AddOutputModifier(
            FGameplayModifierEvaluatedData(DamageStatics().DamageProperty, EGameplayModOp::Additive, DamageDone));
    }
}
