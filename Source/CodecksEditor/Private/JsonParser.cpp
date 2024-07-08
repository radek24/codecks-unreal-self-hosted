#include "JsonParser.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "CoreMinimal.h"
#include "UnrealClient.h"
#include "Misc/MessageDialog.h"

#include "DirectoryReaderHelpers.h"
#include "FileHelpers.h"

void UJsonParser::ParseJsonFromClipboard_Implementation(const FString& input)
{
    TSharedPtr<FJsonObject> JsonObject;
    auto Reader = TJsonReaderFactory<>::Create(input);
    if (!FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        FText DialogText = FText::FromString("Coudn't parse JSON:" + input);
        FMessageDialog::Open(EAppMsgType::Ok, DialogText);
        return;
    }

    FString levelName;
    float x, y, z, pitch, yaw, roll=0.0f;
    JsonObject->TryGetStringField(TEXT("L"), levelName);

    const TSharedPtr<FJsonObject>* JsonItem;
    if (!(JsonObject->TryGetObjectField(TEXT("P"), JsonItem))) {
        FMessageDialog::Open(EAppMsgType::Ok, jsonErrorMessage);
        return;
    }
    JsonItem->ToSharedRef()->TryGetNumberField(TEXT("X"), x);
    JsonItem->ToSharedRef()->TryGetNumberField(TEXT("Y"), y);
    JsonItem->ToSharedRef()->TryGetNumberField(TEXT("Z"), z);

    if (!(JsonObject->TryGetObjectField(TEXT("R"), JsonItem))) {
        FMessageDialog::Open(EAppMsgType::Ok, jsonErrorMessage);
        return;
    }
    JsonItem->ToSharedRef()->TryGetNumberField(TEXT("P"), pitch);
    JsonItem->ToSharedRef()->TryGetNumberField(TEXT("Y"), yaw);
    JsonItem->ToSharedRef()->TryGetNumberField(TEXT("R"), roll);

    if ((x && y && z && pitch && yaw && roll)) {
        FMessageDialog::Open(EAppMsgType::Ok, jsonErrorMessage);
        return;
    }

    const FViewport* activeViewport = GEditor->GetActiveViewport();
    FEditorViewportClient* editorViewClient = (activeViewport != nullptr) ? (FEditorViewportClient*)activeViewport->GetClient() : nullptr;

    if (!editorViewClient) return;
    editorViewClient->SetViewLocation(FVector(x, y, z));
    editorViewClient->SetViewRotation(FRotator(pitch, yaw, roll));

    const FString currentLevel = GEditor->GetEditorWorldContext().World()->GetMapName();
    if (levelName == currentLevel) return;

    const FString rootDir = FPaths::ProjectDir();
    TArray<FString> arrayOfUmaps;
    const TCHAR* levelNameTCHAR = *levelName;
    GetFiles(rootDir, arrayOfUmaps, true, "umap");

    const int32 index = arrayOfUmaps.IndexOfByPredicate(
        [levelNameTCHAR](const FString& Str)
        {
            return Str.Contains(levelNameTCHAR);
        });

    if (index < 0)
    {
        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Couldn't find " + levelName + " in levels"));
        return;
    }
    const FString pathToLevel = arrayOfUmaps[index];

    FEditorFileUtils::LoadMap(pathToLevel);
}