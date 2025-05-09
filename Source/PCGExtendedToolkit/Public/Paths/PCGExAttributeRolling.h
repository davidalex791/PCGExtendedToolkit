﻿// Copyright 2025 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#pragma once

#include "CoreMinimal.h"
#include "PCGExPathProcessor.h"
#include "PCGExPaths.h"
#include "PCGExPointsProcessor.h"
#include "Data/Blending/PCGExDataBlending.h"
#include "Data/Blending/PCGExMetadataBlender.h"


#include "PCGExAttributeRolling.generated.h"

UENUM()
enum class EPCGExRollingTriggerMode : uint8
{
	None  = 0 UMETA(DisplayName = "None", ToolTip="Ignore triggers"),
	Reset = 2 UMETA(DisplayName = "Reset", ToolTip="Reset rolling"),
	Pin   = 4 UMETA(DisplayName = "Pin", ToolTip="Pin triggered value to roll with until next trigger"),
};

/**
 * 
 */
UCLASS(MinimalAPI, BlueprintType, ClassGroup = (Procedural), Category="PCGEx|Clusters")
class UPCGExAttributeRollingSettings : public UPCGExPathProcessorSettings
{
	GENERATED_BODY()

public:
	UPCGExAttributeRollingSettings(const FObjectInitializer& ObjectInitializer);

	//~Begin UPCGSettings
#if WITH_EDITOR
	PCGEX_NODE_INFOS(AttributeRolling, "Path : Attribute Rolling", "Does a rolling blending of properties & attributes.");
#endif

protected:
	virtual FPCGElementPtr CreateElement() const override;
	//~End UPCGSettings

	//~Begin UPCGExPointsProcessorSettings
public:
	PCGEX_NODE_POINT_FILTER(PCGExPaths::SourceTriggerFilters, "Filters used to check if a point triggers the select behavior.", PCGExFactories::PointFilters, false)
	//~End UPCGExPointsProcessorSettings

	/** What to do when the trigger filter is true */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	EPCGExRollingTriggerMode TriggerAction = EPCGExRollingTriggerMode::None;

	/** Blending settings used to smooth attributes.*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable, ShowOnlyInnerProperties))
	FPCGExBlendingDetails BlendingSettings = FPCGExBlendingDetails(EPCGExDataBlendingType::Sum, EPCGExDataBlendingType::None);

	/** Reverse rolling order */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	bool bReverseRolling = false;
};

struct FPCGExAttributeRollingContext final : FPCGExPathProcessorContext
{
	friend class FPCGExAttributeRollingElement;
	FPCGExBlendingDetails BlendingSettings;
};

class FPCGExAttributeRollingElement final : public FPCGExPathProcessorElement
{
public:
	virtual FPCGContext* Initialize(
		const FPCGDataCollection& InputData,
		TWeakObjectPtr<UPCGComponent> SourceComponent,
		const UPCGNode* Node) override;

protected:
	virtual bool Boot(FPCGExContext* InContext) const override;
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};

namespace PCGExAttributeRolling
{
	class FProcessor final : public PCGExPointsMT::TPointsProcessor<FPCGExAttributeRollingContext, UPCGExAttributeRollingSettings>
	{
		int32 MaxIndex = 0;
		int32 LastTriggerIndex = -1;

		PCGExPaths::FPathMetrics CurrentMetric;
		TSharedPtr<PCGExDataBlending::FMetadataBlender> MetadataBlender;

		UPCGMetadata* OutMetadata = nullptr;
		TArray<FPCGPoint>* OutPoints = nullptr;

	public:
		explicit FProcessor(const TSharedRef<PCGExData::FFacade>& InPointDataFacade):
			TPointsProcessor(InPointDataFacade)
		{
		}

		virtual ~FProcessor() override;
		virtual void RegisterBuffersDependencies(PCGExData::FFacadePreloader& FacadePreloader) override;
		virtual bool Process(const TSharedPtr<PCGExMT::FTaskManager>& InAsyncManager) override;
		virtual void PrepareSingleLoopScopeForPoints(const PCGExMT::FScope& Scope) override;
		virtual void ProcessSingleRangeIteration(const int32 Iteration, const PCGExMT::FScope& Scope) override;
		virtual void CompleteWork() override;
	};
}
