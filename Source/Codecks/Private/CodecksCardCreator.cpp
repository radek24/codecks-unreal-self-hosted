// Copyright 2022 Maschinen-Mensch UG

#include "CodecksCardCreator.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonSerializer.h"
#include <UnrealClient.h>
#include "CodecksSettings.h"
#include "ImageUtils.h"
#include "Codecks.h"

#define ENDL "\r\n"
#define DASHES "--"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

inline void append_data(TArray<uint8>& data, const char* str, size_t len)
{
	int begin = data.Num();
	data.AddUninitialized(len);

	for (int i = 0; i != len; ++i)
	{
		data[begin + i] = str[i];
	}
}

inline void append_ansi(TArray<uint8>& data, const char* str)
{
	int len = std::strlen(str);

	append_data(data, str, len);
}

inline void append_fstring(TArray<uint8>& data, const FString& str)
{
	int len = str.Len();

	int begin = data.Num();
	data.AddUninitialized(len);

	for (int i = 0; i != len; ++i)
	{
		data[begin + i] = (uint8)str[i];
	}
}
FString UCodecksCardCreator::GetContentTypeFromFile(CodecksFileType fileType) {
	switch (fileType)
	{
	case CodecksFileType::PlainText:
		return TEXT("text/plain");
	case CodecksFileType::JSON:
		return TEXT("application/json");
	case CodecksFileType::PNG:
		return TEXT("image/png");
	case CodecksFileType::JPG:
		return TEXT("image/jpeg");
	case CodecksFileType::Binary:
	default:
		return TEXT("application/octet-stream");
	}
}

void ProcessCardCreationResponse(
	const FString& ResponseContent,
	const TArray<FCodecksFileInfo>& files,
	const FCodecksCardCreated& createdCallback,
	const FCodecksCardError& errorCallback)
{
	// Validate http called us back on the Game Thread...
	check(IsInGameThread());

	bool ok = false;
	FString message = "Unknown Error";

	UE_LOG(LogCodeck, Log, TEXT("Response: %s"), *ResponseContent);

	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(ResponseContent);
	TSharedPtr<FJsonObject> OutObject;

	if (FJsonSerializer::Deserialize(JsonReader, OutObject))
	{
		OutObject->TryGetBoolField(TEXT("ok"), ok);
		OutObject->TryGetStringField(TEXT("message"), message);
	}

	if (!ok)
	{
		errorCallback.ExecuteIfBound(FString::Printf(TEXT("Error creating card: %s."), *message));
		createdCallback.ExecuteIfBound(CodecksCardCreationStatus::Fail);
		return;
	}

	TArray<uint8> payload;
	payload.Reserve(2048 * 2);

	//randomize terminator string
	char terminator[128] = "1";
	{
		char* p = terminator + std::strlen(terminator);
		for (int i = 0; i != 32; ++i)
		{
			*p = "0123456789"[rand() % 10];
			++p;
		}
		*p = 0;
	}


	TArray< TSharedPtr<FJsonValue> > uploadUrls;
	const TArray< TSharedPtr<FJsonValue> >* puploadUrls = &uploadUrls;
	//If we don't have any files we can just exit with success
	if (!(OutObject->TryGetArrayField(TEXT("uploadUrls"), puploadUrls))) return;

	if (puploadUrls->Num() == 0) {
		UE_LOG(LogCodeck, Log, TEXT("NO upload urls"));
		createdCallback.ExecuteIfBound(CodecksCardCreationStatus::Success);
	}

	for (auto& upload : *puploadUrls)
	{
		payload.Reset();

		auto doc = upload->AsObject();

		FString fileName;
		//No fileName field
		if (!doc->TryGetStringField(TEXT("fileName"), fileName))continue;

		const FCodecksFileInfo* pfileInfo = nullptr;

		for (const FCodecksFileInfo& fi : files)
		{
			if (fi.filename == fileName)
			{
				pfileInfo = &fi;
				break;
			}
		}
		//couldn't find matching file in attachments
		if (!pfileInfo) continue;

		const FCodecksFileInfo& file = *pfileInfo;

		FString uriQuery;
		//No url field
		if (!doc->TryGetStringField(TEXT("url"), uriQuery))continue;

		const TSharedPtr<FJsonObject>* pfields;
		//No fields field
		if (!doc->TryGetObjectField(TEXT("fields"), pfields))continue;

		TSharedPtr<FJsonObject> fields = *pfields;

		FHttpModule& httpModule = FHttpModule::Get();
		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> pRequest = httpModule.CreateRequest();
		pRequest->SetVerb(TEXT("POST"));

		FString metaContentType = TEXT("multipart/form-data; boundary=");
		metaContentType.Append(terminator);
		pRequest->SetHeader(TEXT("Content-Type"), metaContentType);

		const char* fieldNames[] = {
			"key",
			"Cache-Control",
			"acl",
			"bucket",
			"X-Amz-Algorithm",
			"X-Amz-Credential",
			"X-Amz-Date",
			"Policy",
			"X-Amz-Signature"
		};

		for (const char* fieldName : fieldNames)
		{
			FString key = TEXT("");
			key.Append(fieldName);

			FString value;
			if (fields->TryGetStringField(key, value))
			{
				append_ansi(payload, DASHES); append_ansi(payload, terminator); append_ansi(payload, ENDL);
				append_ansi(payload, "Content-Disposition: form-data; name=\"");
				append_fstring(payload, key.ToLower());
				append_ansi(payload, "\"" ENDL);
				append_ansi(payload, ENDL);
				append_fstring(payload, value);
				append_ansi(payload, ENDL);
			}
		}

		//CONTENT TYPE
		FString contentType = UCodecksCardCreator::GetContentTypeFromFile(file.type);
		append_ansi(payload, (DASHES)); append_ansi(payload, terminator); append_ansi(payload, ENDL);
		append_ansi(payload, ("Content-Disposition: form-data; name=\"Content-Type\"" ENDL ENDL));
		append_fstring(payload, contentType);
		append_ansi(payload, ENDL);

		//FILE META
		append_ansi(payload, (DASHES)); append_ansi(payload, terminator); append_ansi(payload, ENDL);
		append_ansi(payload, ("Content-Type: "));
		append_fstring(payload, contentType);
		append_ansi(payload, ENDL);

		//FILE HEADER
		append_ansi(payload, ("Content-Disposition: form-data; name=\"file\"; filename=\""));
		append_fstring(payload, fileName);
		append_ansi(payload, ("\""));
		append_ansi(payload, (ENDL));

		//FILE CONTENT
		append_ansi(payload, ENDL);
		payload.Append(file.data.GetData(), file.data.Num());
		append_ansi(payload, ENDL DASHES); append_ansi(payload, terminator); append_ansi(payload, "--");

		pRequest->SetContent(payload);

		FString uploadUrl;
		doc->TryGetStringField(TEXT("url"), uploadUrl);
		pRequest->SetURL(uploadUrl);

		pRequest->OnProcessRequestComplete().BindLambda(
			[createdCallback, errorCallback](FHttpRequestPtr pRequest, FHttpResponsePtr pResponse, bool connectedSuccessfully) mutable
			{
				bool ok = true;
				if (connectedSuccessfully)
				{
					FString message = pResponse->GetContentAsString();
					if (message.Contains(TEXT("Error")))
					{
						errorCallback.ExecuteIfBound(FString::Printf(TEXT("Error Uploading File: %s."), *message));
						ok = false;
					}
				}
				else {
					if (pRequest->GetStatus() == EHttpRequestStatus::Failed_ConnectionError)
					{
						errorCallback.ExecuteIfBound(TEXT("Connection failed."));
						ok = false;
					}
					else
					{
						errorCallback.ExecuteIfBound(TEXT("Request failed."));
						ok = false;
					}
				}

				createdCallback.ExecuteIfBound(ok ? CodecksCardCreationStatus::Success : CodecksCardCreationStatus::Partially);
			});

		// Finally, submit the request for processing
		pRequest->ProcessRequest();
	}

}



void UCodecksCardCreator::CreateNewCodecksCard(
	const FString& cardTextContent,
	const bool sendFiles,
	TArray<FCodecksFileInfo>& files,
	const FCodecksCardCreated& createdCallback,
	const FCodecksCardError& errorCallback,
	const CodecksSeverity severity,
	const FString& userEmail,
	const CodecksDeckType deckType)
{
	FString uriQuery = GetReportUrl(deckType);

	FHttpModule& httpModule = FHttpModule::Get();

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> pRequest = httpModule.CreateRequest();

	pRequest->SetVerb(TEXT("POST"));
	pRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	TSharedRef<FJsonObject> json = TSharedRef<FJsonObject>(new FJsonObject);
	json->SetStringField(TEXT("content"), cardTextContent);

	switch (severity)
	{
	case CodecksSeverity::Critical:
		json->SetStringField(TEXT("severity"), TEXT("critical")); break;
	case CodecksSeverity::High:
		json->SetStringField(TEXT("severity"), TEXT("high")); break;
	case CodecksSeverity::Low:
		json->SetStringField(TEXT("severity"), TEXT("low")); break;
	case CodecksSeverity::None:
		break;
	default:
		checkNoEntry();
	}

	FString cleanMail = userEmail.TrimStartAndEnd();
	if (!cleanMail.IsEmpty())
	{
		json->SetStringField(TEXT("userEmail"), cleanMail);
	}

	if (sendFiles) {
		TArray<TSharedPtr<FJsonValue>> jFileNames;
		for (auto& file : files)
		{
			TSharedPtr <FJsonValueString> jfile = TSharedPtr <FJsonValueString>(new FJsonValueString(file.filename));
			jFileNames.Add(jfile);
		}
		json->SetArrayField("fileNames", jFileNames);
	}

	FString RequestContent;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&RequestContent);

	FJsonSerializer::Serialize(json, Writer);

	FString OutputString4;
	TSharedRef< TJsonWriter<> > MyWriter = TJsonWriterFactory<>::Create(&OutputString4);
	FJsonSerializer::Serialize(json, MyWriter);

	UE_LOG(LogCodeck, Log, TEXT("resulting jsonString -> %s"), *OutputString4);

	pRequest->SetContentAsString(RequestContent);
	pRequest->SetURL(uriQuery);

	pRequest->OnProcessRequestComplete().BindLambda(
		[fileArray = MoveTemp(files), createdCallback, errorCallback](FHttpRequestPtr pRequest, FHttpResponsePtr pResponse, bool connectedSuccessfully) mutable
		{
			if (connectedSuccessfully)
			{
				if (fileArray.Num() == 0)
				{
					createdCallback.ExecuteIfBound(CodecksCardCreationStatus::Success);
				}
				else
				{
					ProcessCardCreationResponse(pResponse->GetContentAsString(), fileArray, createdCallback, errorCallback);
				}
			}
			else {
				if (pRequest->GetStatus() == EHttpRequestStatus::Failed_ConnectionError)
				{
					errorCallback.ExecuteIfBound(TEXT("Connection failed."));
				}
				else
				{
					errorCallback.ExecuteIfBound(TEXT("Request failed."));
				}
				createdCallback.ExecuteIfBound(CodecksCardCreationStatus::Fail);
			}
		});
	pRequest->ProcessRequest();
}

FString UCodecksCardCreator::GetReportUrl(CodecksDeckType type) {
	FString APIToken = TEXT("");
	FString uriBase = TEXT("");

	bool isShipping = false;
#if UE_BUILD_SHIPPING
	isShipping = true;
#endif

	const UCodecksSettings* CodecksSettings = GetDefault<UCodecksSettings>();
	if (!CodecksSettings) {
		UE_LOG(LogCodeck, Fatal, TEXT("Can't acess Codeck settings"));
		return TEXT("");
	}

	if (isShipping || (!CodecksSettings->useDifferentReportingTokenPerConfiguration)) {
		uriBase = CodecksSettings->PublicCodecksURL;
		switch (type)
		{
		case CodecksDeckType::Feedback:
			APIToken = CodecksSettings->PublicFeedbackToken;
			break;
		case CodecksDeckType::BugReport:
			APIToken = CodecksSettings->PublicReportToken;
			break;
		default:
			checkNoEntry();
		}
	}
	else {
		uriBase = CodecksSettings->InternalCodecksURL;
		switch (type)
		{
		case CodecksDeckType::Feedback:
			APIToken = CodecksSettings->InternalFeedbackToken;
			break;
		case CodecksDeckType::BugReport:
			APIToken = CodecksSettings->InternalReportToken;
			break;
		default:
			checkNoEntry();
		}
	}
	return uriBase + APIToken;
}

void UCodecksCardCreator::TakeScreenshotHelper(bool showUI, const FCodecksScreenshotCreated& createdCallback)
{
	UGameViewportClient::OnScreenshotCaptured().AddLambda([=](int32 Width, int32 Height, const TArray<FColor>& Colors) {
		FCodecksFileInfo screenshotFile;

		const FString DateName = "Screenshot";
		const FString Filename = DateName + ".png";

		screenshotFile.filename = Filename;
		screenshotFile.type = CodecksFileType::PNG;
		FImageUtils::ThumbnailCompressImageArray(Width, Height, Colors, screenshotFile.data);

		FString FilePath = FPaths::ProjectSavedDir() + TEXT("Reports/") + Filename;

		bool bSuccess = FFileHelper::SaveArrayToFile((screenshotFile.data), *FilePath);
		if (bSuccess)
		{
			UE_LOG(LogCodeck, Log, TEXT("File saved successfully at %s"), *FilePath);
		}
		else
		{
			UE_LOG(LogCodeck, Error, TEXT("Failed to save file at %s"), *FilePath);
		}
		createdCallback.ExecuteIfBound(MoveTemp(screenshotFile));
		});
	FScreenshotRequest::RequestScreenshot(showUI);
}
