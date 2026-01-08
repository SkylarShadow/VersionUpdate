

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Settings/VersionManifestObject.h"
#include "Settings/VersionManifestClientObject.h"

class SVersionUpdateSlateMain :public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SVersionUpdateSlateMain){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);


private:
	/** 当前编辑的 UVersionManifestObject */
	UPROPERTY()
	UVersionManifestObject* ManifestObject;

	/** DetailsView */
	TSharedPtr<class IDetailsView> DetailsView;

	/** 按钮回调 */
	FReply OnRebuildFileInfoClicked();
	FReply OnImportJsonClicked();
	FReply OnExportJsonClicked();

	// 内部工具函数
	void RebuildFileInfo();
	bool ImportJson(const FString& FilePath = TEXT(""));
	bool ExportJson(const FString& FilePath = TEXT(""));
};