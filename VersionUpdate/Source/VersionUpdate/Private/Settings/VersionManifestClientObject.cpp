// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/VersionManifestClientObject.h"

#include "Json.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Version/CtrlVersion.h"

namespace VersionClientJson
{
	bool Save(const FClientVersionFilesList& Info, FString& OutJson)
	{
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);

		Writer->WriteObjectStart();

		// currentVersion
		Writer->WriteValue(TEXT("currentVersion"), Info.CurrentVersion);

		// installedPatchFiles
		Writer->WriteArrayStart(TEXT("installedPatchFiles"));
		for (const FRemotePatchFile& File : Info.InstalledPatchFiles)
		{
			VersionControl::WritePatchFile(Writer, File);
		}
		Writer->WriteArrayEnd();

		Writer->WriteObjectEnd();
		Writer->Close();

		return true;
	}

	bool Read(const FString& Json, FClientVersionFilesList& OutInfo)
	{
		TSharedPtr<FJsonObject> Root;
		if (!FJsonSerializer::Deserialize(
			TJsonReaderFactory<>::Create(Json), Root) || !Root)
		{
			return false;
		}

		// currentVersion
		Root->TryGetStringField(TEXT("currentVersion"), OutInfo.CurrentVersion);

		// installedPatchFiles
		const TArray<TSharedPtr<FJsonValue>>* Files = nullptr;
		if (Root->TryGetArrayField(TEXT("installedPatchFiles"), Files))
		{
			OutInfo.InstalledPatchFiles.Empty();

			for (const auto& FileValue : *Files)
			{
				FRemotePatchFile File;
				VersionControl::ReadPatchFile(FileValue->AsObject(), File);
				OutInfo.InstalledPatchFiles.Add(MoveTemp(File));
			}
		}

		return true;
	}

}