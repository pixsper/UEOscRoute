#pragma once

#include "K2Node.h"
#include "RouteOSCByAddress.generated.h"

UCLASS( )
class OSCROUTE_API URouteOSCByAddress : public UK2Node
{
	GENERATED_BODY( )

	static const FName NAME_Message;
	static const FName NAME_NoMatch;
	static const FName NAME_UnmatchedMessage;

	TMap<FName, FOSCRoute> ValidatedRoutes;

public:
	UPROPERTY(EditAnywhere)
	TArray<FOSCRouteConfig> Routes;

	UE_NODISCARD virtual bool IsNodeSafeToIgnore( ) const override { return true; }

	UE_NODISCARD virtual bool ShouldShowNodeProperties() const override { return true; }

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual FText GetTooltipText() const override;

	virtual FText GetMenuCategory() const override;

	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;

	virtual void GetMenuActions( FBlueprintActionDatabaseRegistrar& ActionRegistrar ) const override;



	virtual void AllocateDefaultPins() override;

	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;


#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

private:

	void UpdateRoutesFromConfig();
	void UpdatePinsFromRoutes();

	UEdGraphPin* GetMessagePin() const;
	UEdGraphPin* GetNoMatchPin() const;
	UEdGraphPin* GetUnmatchedMessagePin() const;
};