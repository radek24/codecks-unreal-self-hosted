#include "ReportingWindowBase.h"

FString UReportingWindowBase::GetMetadataString()
{
	FString Output = TEXT("");

	for (auto& Elem : Metadata) {
		Output.Append(Elem.Key + TEXT(": ") + Elem.Value + TEXT("\n"));
	}
	return Output.Mid(0,Output.Len()-1);
}
