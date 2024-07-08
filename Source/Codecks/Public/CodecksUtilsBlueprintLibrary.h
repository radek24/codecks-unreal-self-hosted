

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CodecksUtilsBlueprintLibrary.generated.h"

/**
 * 
 */
UCLASS()
class CODECKS_API UCodecksUtilsBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

private:
	inline static constexpr short flagToReadWrite = 4;

public:

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Game Version"), Category = "Codecks|Utils")
	static FString GetGameVersion();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Save String To File"), Category = "Codecks|Utils")
	static bool SaveStringToFile(FString path,FString textToSave);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Load String From File"), Category = "Codecks|Utils")
	static bool LoadStringFromFile(FString path, FString& textToLoad);

};
