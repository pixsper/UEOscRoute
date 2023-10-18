#include "RouteOscByAddress.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
#include "GraphEditorSettings.h"
#include "OSCMessage.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "OSCRouteFunctions.h"
#include "K2Node_CallFunction.h"


#define LOCTEXT_NAMESPACE "FOSCRouteModule"

const FName URouteOSCByAddress::NAME_Message(TEXT("Message"));
const FName URouteOSCByAddress::NAME_NoMatch(TEXT("No Match"));
const FName URouteOSCByAddress::NAME_UnmatchedMessage(TEXT("Unmatched Message"));

FText URouteOSCByAddress::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("RouteOscByAddress_Title", "Route Osc By Address");
}

FText URouteOSCByAddress::GetTooltipText() const
{
	return LOCTEXT("RouteOscByAddress_Tooltip", "Routes the arguments of an OSC Message to a destination determined by it's address");
}

FText URouteOSCByAddress::GetMenuCategory() const
{
	return LOCTEXT("RouteOscByAddress_MenuCategory", "Audio|Osc");
}

FSlateIcon URouteOSCByAddress::GetIconAndTint(FLinearColor& OutColor) const
{
	OutColor = GetDefault<UGraphEditorSettings>()->FunctionCallNodeTitleColor;
	return FSlateIcon(FAppStyle::GetAppStyleSetName(), "Kismet.AllClasses.FunctionIcon");
}

void URouteOSCByAddress::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	Super::GetMenuActions(ActionRegistrar);
	if (const UClass* Action = GetClass(); ActionRegistrar.IsOpenForRegistration(Action))
	{
		UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
		check(Spawner != nullptr);
		ActionRegistrar.AddBlueprintAction(Action, Spawner);
	}
}

void URouteOSCByAddress::AllocateDefaultPins()
{
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

	// Input
	{
		FCreatePinParams PinParams;
		PinParams.Index = 0;
		CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	}
	{
		FCreatePinParams PinParams;
		PinParams.Index = 1;
		PinParams.bIsReference = true;
		UEdGraphPin* MessagePin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Struct, FOSCMessage::StaticStruct(),
			NAME_Message, PinParams);
	}


	// Output
	{
		FCreatePinParams PinParams;
		PinParams.Index = 0;
		CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, NAME_NoMatch, PinParams);
	}
	{
		FCreatePinParams PinParams;
		PinParams.Index = 1;
		PinParams.bIsReference = true;
		UEdGraphPin* UnmatchedMessagePin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Struct, FOSCMessage::StaticStruct(),
			NAME_UnmatchedMessage, PinParams);
	}

	UpdateRoutesFromConfig();
	UpdatePinsFromRoutes();

	Super::AllocateDefaultPins();
}

void URouteOSCByAddress::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	UClass* LibraryClass = UOSCRouteFunctions::StaticClass();
	UFunction* BlueprintFunction = LibraryClass->FindFunctionByName(GET_FUNCTION_NAME_CHECKED( UOSCRouteFunctions, Switch ));

	bool bIsFirst = true;

	for(const auto& Route : ValidatedRoutes)
	{
		UK2Node_CallFunction* CallFunction = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
		CallFunction->SetFromFunction(BlueprintFunction);

	}

	UK2Node_CallFunction* CallFunction = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	CallFunction->SetFromFunction(BlueprintFunction);

	CallFunction->AllocateDefaultPins();
	CompilerContext.MessageLog.NotifyIntermediateObjectCreation(CallFunction, this);

	// Input
	auto res = CompilerContext.MovePinLinksToIntermediate(*GetMessagePin(),
		*CallFunction->FindPinChecked(TEXT("Message")));

	// Output
	auto res2 = CompilerContext.MovePinLinksToIntermediate(*GetUnmatchedMessagePin(), *CallFunction->GetReturnValuePin());

	// Exec pins
	auto res3 = CompilerContext.MovePinLinksToIntermediate(*GetExecPin(), *CallFunction->GetExecPin());
	auto res4 = CompilerContext.MovePinLinksToIntermediate(*GetNoMatchPin(), *CallFunction->GetThenPin());

	BreakAllNodeLinks();
}

#if WITH_EDITOR
void URouteOSCByAddress::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	UpdateRoutesFromConfig();
	UpdatePinsFromRoutes();
	GetGraph( )->NotifyGraphChanged( );
	FBlueprintEditorUtils::MarkBlueprintAsModified( GetBlueprint( ) );
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}
#endif


void URouteOSCByAddress::UpdateRoutesFromConfig()
{
	int32_t i = 0;
	for (const auto& Config : Routes)
	{
		const FOSCAddress Address(Config.Address);
		if (Address.IsValidPath())
		{
			const FName PinName(Config.Address);
			if (FOSCRoute* ExistingRoute = ValidatedRoutes.Find(PinName))
			{
				ExistingRoute->Index = i;
			}
			else
			{
				FOSCRoute Route;
				Route.Id = Config.Id;
				Route.Index = i;
				Route.Address = Address;
				Route.PinName = PinName;

				ValidatedRoutes.Add(PinName, Route);
			}
		}

		++i;
	}

	TArray<FName> ToRemove;
	for (const auto& Pair : ValidatedRoutes)
	{
		FString PinName = Pair.Key.ToString();
		bool bIsDelete = true;
		for (const auto& Route : Routes)
		{
			if (Route.Address == PinName)
			{
				bIsDelete = false;
				break;
			}
		}

		if (bIsDelete)
			ToRemove.Push(Pair.Key);
	}

	for (const auto& Key : ToRemove)
		ValidatedRoutes.Remove(Key);
}

void URouteOSCByAddress::UpdatePinsFromRoutes()
{
	TArray<UEdGraphPin*> ToRemove;
	TArray<TTuple<UEdGraphPin*, const FOSCRoute*>> ToRecreate;

	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin->Direction == EGPD_Input
			|| Pin->PinName == NAME_Message
			|| Pin->PinName == NAME_NoMatch
			|| Pin->PinName == NAME_UnmatchedMessage)
		{
			continue;
		}

		if (const FOSCRoute* Route = ValidatedRoutes.Find(Pin->PinName))
		{
			if (Pin->SourceIndex != Route->Index)
			{
				ToRecreate.Push(MakeTuple(Pin, Route));
			}
		}
		else
		{
			ToRemove.Push(Pin);
		}
	}

	for (UEdGraphPin* Pin : ToRemove)
		RemovePin(Pin);

	for (auto Tuple : ToRecreate)
	{
		TArray<UEdGraphPin*> LinkedPins = Tuple.Get<0>()->LinkedTo;
		RemovePin(Tuple.Get<0>());

		FCreatePinParams Params;
		Params.Index = Tuple.Get<1>()->Index + 2;
		UEdGraphPin* Pin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, Tuple.Get<1>()->PinName, Params);
		if (Pin)
		{
			for (UEdGraphPin* LinkedPin : LinkedPins)
				Pin->MakeLinkTo(LinkedPin);
		}
	}

	for (const auto& Pair : ValidatedRoutes)
	{
		if (FindPin(Pair.Key) == nullptr)
		{
			FCreatePinParams Params;
			Params.Index = Pair.Value.Index + 2;
			CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, Pair.Key, Params);
		}
	}
}

UEdGraphPin* URouteOSCByAddress::GetMessagePin() const
{
	UEdGraphPin* Pin = FindPin(NAME_Message);
	check(Pin == NULL || Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* URouteOSCByAddress::GetNoMatchPin() const
{
	UEdGraphPin* Pin = FindPin(NAME_NoMatch);
	check(Pin == NULL || Pin->Direction == EGPD_Output);
	return Pin;
}

UEdGraphPin* URouteOSCByAddress::GetUnmatchedMessagePin() const
{
	UEdGraphPin* Pin = FindPin(NAME_UnmatchedMessage);
	check(Pin == NULL || Pin->Direction == EGPD_Output);
	return Pin;
}

#undef LOCTEXT_NAMESPACE
