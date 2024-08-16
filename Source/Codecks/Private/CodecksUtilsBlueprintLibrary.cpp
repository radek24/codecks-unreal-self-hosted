#include "CodecksUtilsBlueprintLibrary.h"

#include "HAL/FileManagerGeneric.h"
#include "Misc/FileHelper.h"


FString UCodecksUtilsBlueprintLibrary::GetGameVersion()
{
	FString AppVersion;
	GConfig->GetString(
		TEXT("/Script/EngineSettings.GeneralProjectSettings"),
		TEXT("ProjectVersion"),
		AppVersion,
		GGameIni
	);
	return AppVersion;
}

bool UCodecksUtilsBlueprintLibrary::SaveStringToFile(FString path,FString textToSave)
{
	return FFileHelper::SaveStringToFile(textToSave, *path);
}

bool UCodecksUtilsBlueprintLibrary::LoadStringFromFile(FString path, FString& textToLoad)
{
	return FFileHelper::LoadFileToString(textToLoad, *path, FFileHelper::EHashOptions::None, flagToReadWrite);
}

bool UCodecksUtilsBlueprintLibrary::WithEditor()
{
#if WITH_EDITOR
	return true;
#else
	return false;
#endif
}