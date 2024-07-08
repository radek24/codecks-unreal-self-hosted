#include "JsonParserBase.h"

UJsonParserBase::UJsonParserBase()
{
    //Empty constructor
}

void UJsonParserBase::ParseJsonFromClipboard_Implementation(const FString& input)
{
    ensureMsgf(false, TEXT("Dont use JsonParserBase as your json parser, create your own or use JsonParser"));
}