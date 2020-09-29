#include "CustomImportCallbackCommandlet.h"


#include "JsonObjectConverter.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "UObject/ConstructorHelpers.h"

UCustomImportCallbackCommandlet::UCustomImportCallbackCommandlet(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	LogToConsole = true;
}

namespace FCustomCallbacksDemoLocal
{
	// Additional custom property name for custom json data
	static const FString CustomAdditionalPropertyName = TEXT("SubObjectRef");

	// FPackageName::SplitFullObjectPath(StringValue, ClassName, PackagePath, ObjectName, SubObjectName);
	// TODO: https://github.com/EpicGames/UnrealEngine/pull/7371
	void CustomSplitFullObjectPath(const FString& InFullObjectPath, FString& OutClassName, FString& OutPackageName, FString& OutObjectName, FString& OutSubObjectName)
	{
		FString Sanitized = InFullObjectPath.TrimStartAndEnd();
		const TCHAR* Cur = *Sanitized;

		auto ExtractBeforeDelim = [&Cur](TCHAR Delim, FString& OutString)
		{
			const TCHAR* Start = Cur;
			while (*Cur != '\0' && *Cur != Delim && *Cur != '\'')
			{
				++Cur;
			}

			OutString = FString(Cur - Start, Start);

			if ((*Cur == Delim) || (*Cur == '\''))
			{
				++Cur;
			}
		};

		ExtractBeforeDelim(' ', OutClassName);
		ExtractBeforeDelim('.', OutPackageName);
		ExtractBeforeDelim(':', OutObjectName);
		ExtractBeforeDelim('\0', OutSubObjectName);
	}

	// Implementation for CustomExportCallback
	TSharedPtr<FJsonValue> ObjectJsonCallback(FProperty* Property, const void* Value)
	{
		if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
		{
			FString StringValue;
			Property->ExportTextItem(StringValue, Value, NULL, NULL, PPF_None);

			FString ClassName;
			FString PackagePath;
			FString ObjectName;
			FString SubObjectName;

			// This don`t work. See PR: https://github.com/EpicGames/UnrealEngine/pull/7371
			//FPackageName::SplitFullObjectPath(StringValue, ClassName, PackagePath, ObjectName, SubObjectName);
			CustomSplitFullObjectPath(StringValue, ClassName, PackagePath, ObjectName, SubObjectName);

			// If this ObjectProperty not include instanced object
			if (SubObjectName.IsEmpty())
				// Then return String Reference on this object
				return MakeShared<FJsonValueString>(StringValue);

			FString ObjectPath = StringValue;
			ConstructorHelpers::StripObjectClass(ObjectPath);

			UObject* Object = LoadObject<UObject>(nullptr, *ObjectPath);
			if (!Object)
			{
				UE_LOG(LogDemoJsonCallback, Fatal, TEXT("Unable to load object '%s'."), *PackagePath);
				// invalid
				return TSharedPtr<FJsonValue>();
			}

			// Create json object
			TSharedPtr<FJsonObject> JsonInstancedObject = MakeShared<FJsonObject>();
			// Add custom Property for save sub (instanced) object full path
			JsonInstancedObject->SetField(CustomAdditionalPropertyName, MakeShared<FJsonValueString>(StringValue));
			// Iterate by sub (instanced) object properties
			UClass* Class = Object->GetClass();
			for (TFieldIterator<FProperty> Prop(Class); Prop; ++Prop)
			{
				FJsonObjectConverter::CustomExportCallback CustomCB;
				CustomCB.BindStatic(ObjectJsonCallback);
			
				void const* ClassPropertyData = (*Prop)->ContainerPtrToValuePtr<void>(Object);
				const TSharedPtr<FJsonValue> JsonValue =
                    FJsonObjectConverter::UPropertyToJsonValue(*Prop, ClassPropertyData, 0, 0, &CustomCB);

				JsonInstancedObject->SetField(*Prop->GetNameCPP(), JsonValue);
			}

			return MakeShared<FJsonValueObject>(JsonInstancedObject);			
		}

		// invalid
		return TSharedPtr<FJsonValue>();
	}
}

int32 UCustomImportCallbackCommandlet::Main(const FString& Params)
{
	UE_LOG(LogDemoJsonCallback, Display, TEXT("UCustomImportCallbackCommandlet::Main => Params: '%s'"), *Params);
	
	// To demonstrate a specific case, export to json and import from json are well reproduced,
	// the reference of this specific date asset:
	// SomeDataAsset'/Game/ExamplesAssets/CustomDataAssets/DA_SomeDataAsset.DA_SomeDataAsset'
	const FString ReferenceString("SomeDataAsset'/Game/ExamplesAssets/CustomDataAssets/DA_SomeDataAsset.DA_SomeDataAsset'");
		
	// Save exported custom data into "%ProjectSavedDir%/CustomExportData.json" file as example  
	const FString OutputFilePath(FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("CustomExportData.json")));

	// Demo for CustomExportCallback
	ExportCase(ReferenceString, OutputFilePath);
	
	return 0;
}

void UCustomImportCallbackCommandlet::ExportCase(const FString& InReferenceString, const FString& InFilePath)
{
	using namespace FCustomCallbacksDemoLocal;
	
	UE_LOG(LogDemoJsonCallback, Display, TEXT("--- Demo for FJsonObjectConverter::CustomExportCallback started ---"));

	// Make a JsonObject to collect textual representation of object property values
	TSharedPtr<FJsonObject> JsonAssetObject = MakeShared<FJsonObject>();
	// Get asset package
	FString PackagePath = InReferenceString;
	ConstructorHelpers::StripObjectClass(PackagePath);
	
	// Check package exist
	FString OutPackageFileName;
	const bool bIsExist = FPackageName::DoesPackageExist(*PackagePath, nullptr, &OutPackageFileName);
	if (!bIsExist)
		UE_LOG(LogDemoJsonCallback, Fatal, TEXT("Package '%s' does not exist"), *PackagePath);

	// Load object by path 
	UObject* Object = LoadObject<UObject>(nullptr, *PackagePath);
	if (!Object)
		UE_LOG(LogDemoJsonCallback, Fatal, TEXT("Unable to load object '%s'."), *PackagePath);

	// Iterate by properties
	UClass* ObjectClass = Object->GetClass();
	for (TFieldIterator<FProperty> Prop(ObjectClass); Prop; ++Prop)
	{
		// Define a custom callback to handle specific property values
		FJsonObjectConverter::CustomExportCallback CustomCB;
		CustomCB.BindStatic(ObjectJsonCallback);
		// Convert property to JsonValue
		void const* ClassPropertyData = (*Prop)->ContainerPtrToValuePtr<void>(Object);
		const TSharedPtr<FJsonValue> JsonValue = FJsonObjectConverter::UPropertyToJsonValue(*Prop, ClassPropertyData, 0, 0, &CustomCB);
		// And collect it into JsonObject
		JsonAssetObject->SetField((*Prop)->GetNameCPP(), JsonValue);
	}
	
	// Serialize
	FString SerializedJson;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&SerializedJson);
	FJsonSerializer::Serialize(JsonAssetObject.ToSharedRef(), TJsonWriterFactory<>::Create(&SerializedJson));
	
	if (!FFileHelper::SaveStringToFile(SerializedJson, *InFilePath))
		UE_LOG(LogDemoJsonCallback, Fatal, TEXT("Unable to save file '%s'."), *InFilePath);

	UE_LOG(LogDemoJsonCallback, Display, TEXT("Export custom result:\n%s"), *SerializedJson);	
	UE_LOG(LogDemoJsonCallback, Display, TEXT("--- Demo for FJsonObjectConverter::CustomExportCallback succesfull finished ---"));
}


