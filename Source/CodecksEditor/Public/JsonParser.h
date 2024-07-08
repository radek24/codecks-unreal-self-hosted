

#pragma once

#include "CoreMinimal.h"
#include "JsonParserBase.h"
#include "JsonParser.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class CODECKSEDITOR_API UJsonParser : public UJsonParserBase
{
	GENERATED_BODY()
	virtual void ParseJsonFromClipboard_Implementation(const FString& input) override;


private:
	const FText jsonErrorMessage = FText::FromString( "Didn't found all the fields in json");
};
