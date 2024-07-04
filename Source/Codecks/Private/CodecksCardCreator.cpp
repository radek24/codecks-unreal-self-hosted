// Copyright 2022 Maschinen-Mensch UG

#include "CodecksCardCreator.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonSerializer.h"
#include <Misc/OutputDeviceFile.h>
#include <UnrealClient.h>
#include "CodecksSettings.h"
#include "ImageUtils.h"

#define ENDL "\r\n"
#define DASHES "--"


#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

#include "Kismet/GameplayStatics.h"

#define stringify(name) #name
#define EnumValStr(Enum, Val)  EnumToString( (stringify(Enum)) , Val)

FString UCodecksCardCreator::EnumToString(const FString& enumName, const uint8 value)
{
	UEnum* pEnum = FindObject<UEnum>(ANY_PACKAGE, *enumName);
	return *(pEnum ? pEnum->GetNameStringByIndex(static_cast<uint8>(value)) : "null");
}



void UCodecksCardCreator::CreateCardText(FString& outCardText, const FString Header, const FString content, const Category category, const Reproducibility reproducibility, const FString email)
{
	FString PlatformName = UGameplayStatics::GetPlatformName();

	FString AppVersion;
	
	GConfig->GetString(
		TEXT("/Script/EngineSettings.GeneralProjectSettings"),
		TEXT("ProjectVersion"),
		AppVersion,
		GGameIni
	);

	UE_LOG(LogTemp, Warning, TEXT("Response: %s"), *EnumValStr(Category, (uint8)category));
	
	outCardText = "**"+ EnumValStr(Category, (uint8)category) +" - "+Header+"**\n"
		"- **Platforma:** "+PlatformName+"\n"
		"- **Verze:** "+AppVersion+"\n"
		"- **Reprodukovatelnost:** "+ EnumValStr(Reproducibility, (uint8)reproducibility) +"\n"
		"- **Kategorie:** " + EnumValStr(Category, (uint8)category) + "\n"
		"- **Uzivatel:** " + email + "\n"
		+ content;
}


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
	

	UE_LOG(LogTemp, Warning, TEXT("Response: %s"), *ResponseContent);

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
	payload.Reserve(2048*2);

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

	//FOREACH uploadURL
	TArray< TSharedPtr<FJsonValue> > uploadUrls;
	const TArray< TSharedPtr<FJsonValue> >* puploadUrls = &uploadUrls;
	if (OutObject->TryGetArrayField(TEXT("uploadUrls"), puploadUrls))
	{
		for (auto& upload : *puploadUrls)
		{
			payload.Reset();

			auto doc = upload->AsObject();

			FString fileName;
			if (!doc->TryGetStringField(TEXT("fileName"), fileName))
			{
				//No fileName field
				continue;
			}

			const FCodecksFileInfo* pfileInfo = nullptr;

			for (const FCodecksFileInfo& fi : files)
			{
				if (fi.filename == fileName)
				{
					pfileInfo = &fi;
					break;
				}
			}

			if (!pfileInfo)
			{
				//couldn't find matching file in attachments
				continue;
			}

			const FCodecksFileInfo& file = *pfileInfo;

			


			FString uriQuery;
			if (!doc->TryGetStringField(TEXT("url"), uriQuery))
			{
				//No url field
				continue;
			}

			const TSharedPtr<FJsonObject>* pfields;
			if (!doc->TryGetObjectField(TEXT("fields"), pfields))
			{
				//No fields field
				continue;
			}


			TSharedPtr<FJsonObject> fields = *pfields;

			FHttpModule& httpModule = FHttpModule::Get();
			TSharedRef<IHttpRequest, ESPMode::ThreadSafe> pRequest = httpModule.CreateRequest();
			pRequest->SetVerb(TEXT("POST"));


			FString metaContentType = TEXT("multipart/form-data; boundary=");
			metaContentType.Append(terminator);

			pRequest->SetHeader(TEXT("Content-Type"), metaContentType);
			pRequest->SetHeader(TEXT("X-Account"), TEXT("enterprise"));

			FString reportToken = TEXT("");
			if (const UCodecksSettings* CodecksSettings = GetDefault<UCodecksSettings>())
			{
				reportToken = CodecksSettings->ReportToken;
			}

			pRequest->SetHeader(TEXT("X-Auth-Token"), reportToken);

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
			FString contentType = TEXT("application/octet-stream");
			switch (file.type)
			{
			case CodecksFileType::PlainText:
				contentType = TEXT("text/plain");
				break;
			case CodecksFileType::JSON:
				contentType = TEXT("application/json");
				break;
			case CodecksFileType::PNG:
				contentType = TEXT("image/png");
				break;
			case CodecksFileType::JPG:
				contentType = TEXT("image/jpeg");
				break;
			case CodecksFileType::Binary:
			default:
				break;
			}

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

			FString FilePath3 = FPaths::ProjectSavedDir() + TEXT("Reports/request.bin");

			bool bSuccess = FFileHelper::SaveArrayToFile(payload, *FilePath3);

			FString str = (char*)payload.GetData();
			UE_LOG(LogTemp, Log, TEXT("Payload: %s"), *str);

			pRequest->SetContent(payload);

			pRequest->SetURL("/*anonymized URL*/");

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
}

void UCodecksCardCreator::CreateNewCodecksCard(
	const FString& cardTextContent,
	TArray<FCodecksFileInfo>& files,
	const FCodecksCardCreated& createdCallback,
	const FCodecksCardError& errorCallback,
	const CodecksSeverity severity,
	const FString& userEmail)
{
	FString reportToken = TEXT("");
	FString uriBase = TEXT("");
	if (const UCodecksSettings* CodecksSettings = GetDefault<UCodecksSettings>())
	{
		uriBase = CodecksSettings->ReportCreateUrl;
		reportToken = CodecksSettings->ReportToken;
	}

	//https://dev.epicgames.com/community/learning/tutorials/ZdXD/call-rest-api-using-http-json-from-ue5-c
	 
	FString uriQuery = uriBase + reportToken;

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
	default:
		break;
	}

	FString cleanMail = userEmail.TrimStartAndEnd();
	if (!cleanMail.IsEmpty())
	{
		json->SetStringField(TEXT("userEmail"), cleanMail);
	}

	TArray<TSharedPtr<FJsonValue>> jFileNames;
	for (auto& file : files)
	{
		TSharedPtr <FJsonValueString> jfile = TSharedPtr <FJsonValueString>(new FJsonValueString(file.filename));
		jFileNames.Add(jfile);
	}
	json->SetArrayField("fileNames", jFileNames);

	FString RequestContent;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&RequestContent);

	FJsonSerializer::Serialize(json, Writer);


	FString OutputString4;
	TSharedRef< TJsonWriter<> > MyWriter = TJsonWriterFactory<>::Create(&OutputString4);
	FJsonSerializer::Serialize(json, MyWriter);

	UE_LOG(LogTemp, Warning, TEXT("resulting jsonString -> %s"), *OutputString4);


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

FDelegateHandle UCodecksCardCreator::screenshotDelegateHandle;



void UCodecksCardCreator::TakeScreenshotHelper(bool showUI, FString filename, const FCodecksScreenshotCreated& createdCallback)
{
	UCodecksCardCreator::screenshotDelegateHandle = UGameViewportClient::OnScreenshotCaptured().AddLambda([=](int32 Width, int32 Height, const TArray<FColor>& Colors) {

		UGameViewportClient::OnScreenshotCaptured().Remove(UCodecksCardCreator::screenshotDelegateHandle);

		FCodecksFileInfo screenshotFile;

		const FString DateName = FDateTime::Now().ToString(TEXT("%Y-%m-%d %H-%M-%S"));
		const FString Filename = DateName + ".png";

		screenshotFile.filename = Filename;
		screenshotFile.type = CodecksFileType::PNG;
		FImageUtils::ThumbnailCompressImageArray(Width, Height, Colors, screenshotFile.data);
		
		/*FString FilePath = FPaths::ProjectSavedDir() + TEXT("Reports/Screen.png");

		bool bSuccess = FFileHelper::SaveArrayToFile((screenshotFile.data), *FilePath);

		if (bSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("File saved successfully at %s"), *FilePath);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to save file at %s"), *FilePath);
		}*/


		createdCallback.ExecuteIfBound(MoveTemp(screenshotFile));
	});

	FScreenshotRequest::RequestScreenshot(showUI);
}
