#include "CustomImportCallbackCommandlet.h"

#include "FileHelpers.h"
#include "JsonObjectConverter.h"
#include "PackageTools.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "UE4ContributionCases/SplitFullObjectPathCase/SomeDataAsset.h"
#include "UObject/ConstructorHelpers.h"

UCustomImportCallbackCommandlet::UCustomImportCallbackCommandlet(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	LogToConsole = true;
}

namespace FCustomCallbacksDemoLocal
{
	// Additional custom property name for custom json data
	static const FString CustomAdditionalPropertyName = TEXT("SubObjectRef");

	// Load data from json file
	TSharedPtr<FJsonValue> LoadJsonFile(FString const& FilePath);
	
	// FPackageName::SplitFullObjectPath(StringValue, ClassName, PackagePath, ObjectName, SubObjectName);
	// TODO: https://github.com/EpicGames/UnrealEngine/pull/7371
	void CustomSplitFullObjectPath(const FString& InFullObjectPath, FString& OutClassName, FString& OutPackageName, FString& OutObjectName, FString& OutSubObjectName);

	// Implementation for CustomExportCallback (Example of use)
	TSharedPtr<FJsonValue> ObjectJsonCallback(FProperty* Property, const void* Value);

	// Implementation for my CustomImportCallback (Example of use)
	bool JsonToObjectCallback(const TSharedPtr<FJsonValue>& JsonValue, FProperty* Property , void* OutValue);
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

	// Step 1
	// Demo for CustomExportCallback
	{
		UE_LOG(LogDemoJsonCallback, Display, TEXT("UCustomImportCallbackCommandlet::Main => Step 1: Export DA_SomeDataAsset to CustomExportData.json using FJsonObjectConverter::CustomExportCallback."));
		const TSharedPtr<FJsonObject> OutputJson = ExportCase(ReferenceString);
		// Save to temp file
		SaveToJsonFile(OutputJson, OutputFilePath);
	}

	// Step 2
	// Rewrite the instanced subobject from the property in DA_SomeDataAsset to demonstrate
	// that the current implementation of function FJsonObjectConverter::JsonValueToUProperty (which is used in the ImportCase function)
	// inside the engine does not work as expected without adding CustomImportCallback
	{
		UE_LOG(LogDemoJsonCallback, Display, TEXT("UCustomImportCallbackCommandlet::Main => Step 2: Rewrite the instanced subobject from the property in DA_SomeDataAsset to demonstrate."));
		// Load object
		FString ObjectPath = ReferenceString;
		ConstructorHelpers::StripObjectClass(ObjectPath);
		USomeDataAsset* SomeDataAsset = LoadObject<USomeDataAsset>(nullptr, *ObjectPath);
		if (!SomeDataAsset)
			UE_LOG(LogDemoJsonCallback, Fatal, TEXT("Unable to load object '%s'."), *ObjectPath);
		// Change property data 
		SomeDataAsset->ArrayStructWithInstancedObject = TArray<FSomeStructWithInstancedProperty>();
		// Save changed DataAsset
		if (!UEditorLoadingAndSavingUtils::SavePackages({UPackageTools::LoadPackage(ObjectPath)}, false))
			UE_LOG(LogDemoJsonCallback, Fatal, TEXT("Unable to save package %s."), *ObjectPath)
		UE_LOG(LogDemoJsonCallback, Display, TEXT("UCustomImportCallbackCommandlet::Main => Property 'ArrayStructWithInstancedObject' rewrited on empty Array."));

		// Print to console current Asset data
		UE_LOG(LogDemoJsonCallback, Display, TEXT("UCustomImportCallbackCommandlet::Main => After changed property and save package, DataAsset content:"));
		const TSharedPtr<FJsonObject> ChangedJson = ExportCase(ReferenceString);
		SerializeJson(ChangedJson);
	}

	// Step 3
	// Demonstration of the need CustomImportCallback
	// NOTE: I am surprised that there is such a callback in the FJsonObjectConverter::UPropertyToJsonValue function,
	// but in the case of FJsonObjectConverter::JsonValueToUProperty, there is no way to set it.
	// I suggest to introduce this functionality. I am preparing a pull request into engine
	{
		UE_LOG(LogDemoJsonCallback, Display, TEXT("UCustomImportCallbackCommandlet::Main => Step 3: Let's try to recover the DA_SomeDataAsset from the CustomExportData.json file"));
		ImportCase(ReferenceString, OutputFilePath);

		// Print to console current Asset data
		UE_LOG(LogDemoJsonCallback, Display, TEXT("UCustomImportCallbackCommandlet::Main => After import (an attempt to restore the original data)."));
		const TSharedPtr<FJsonObject> AfterImportJson = ExportCase(ReferenceString);
		SerializeJson(AfterImportJson);
	}

	// Summary:
	// Without the ability to define my CustomImportCallback, I still have to write additional code, otherwise, I will lose some of the data in the properties that are marked as instantiated.
    // If CustomImportCallback is in the engine code - this situation can be avoided and successfully restore all properties of the object.
    // I will prepare a pull request to the engine code with this functionality.
	
	return 0;
}

TSharedPtr<FJsonObject> UCustomImportCallbackCommandlet::ExportCase(const FString& InReferenceString)
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

	UE_LOG(LogDemoJsonCallback, Display, TEXT("--- Demo for FJsonObjectConverter::CustomExportCallback succesfull finished ---"));
	return JsonAssetObject;	
}

void UCustomImportCallbackCommandlet::ImportCase(const FString& InReferenceString, const FString& InOpenFilePath)
{
	using namespace FCustomCallbacksDemoLocal;
	
	UE_LOG(LogDemoJsonCallback, Display, TEXT("--- Demo for FJsonObjectConverter::CustomImportCallback started ---"));

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

	// Load data from json file
	TSharedPtr<FJsonValue> JsonFile = LoadJsonFile(InOpenFilePath);
	TSharedPtr<FJsonObject> const* JsonObjectContent;
	if (!JsonFile->TryGetObject(JsonObjectContent))
		UE_LOG(LogDemoJsonCallback, Fatal, TEXT("Unexpected file content '%s'."), *InOpenFilePath);

	// Parse properties
	for (TPair<FString, TSharedPtr<FJsonValue>> const& JsonObjectItemPair : (*JsonObjectContent)->Values)
	{
		FProperty* Property = Object->GetClass()->FindPropertyByName(*JsonObjectItemPair.Key);
		if (Property == nullptr)
			UE_LOG(LogDemoJsonCallback, Fatal, TEXT("Property '%s' not found in class %s. Property scepped."), *JsonObjectItemPair.Key, *Object->GetClass()->GetName());

		/* TODO: Uncomment next lines when CustomImportCallback if it is available in FJsonObjectConverter::JsonValueToUProperty*/
		// FJsonObjectConverter::CustomImportCallback CustomCB;
		// CustomCB.BindStatic(JsonToObjectCallback);
		FJsonObjectConverter::JsonValueToUProperty(
			JsonObjectItemPair.Value,
			Property,
			Property->ContainerPtrToValuePtr<void>(Object),
			0,
			0
			// /* TODO: Uncomment next arg if it is available in FJsonObjectConverter::JsonValueToUProperty*/ , &CustomCB
			);
	}

	// Save changed DataAsset
	if (!UEditorLoadingAndSavingUtils::SavePackages({UPackageTools::LoadPackage(PackagePath)}, false))
		UE_LOG(LogDemoJsonCallback, Fatal, TEXT("Unable to save package %s."), *PackagePath)
	
	UE_LOG(LogDemoJsonCallback, Display, TEXT("Succesful save DA_SomeDataAsset after importing data from json file"));
	
	UE_LOG(LogDemoJsonCallback, Display, TEXT("--- Demo for FJsonObjectConverter::CustomImportCallback succesfull finished ---"));
}

void UCustomImportCallbackCommandlet::SaveToJsonFile(const TSharedPtr<FJsonObject> InJsonObject, const FString& InSaveFilePath)
{
	const FString SerializedJson = SerializeJson(InJsonObject);	
	// Save to file
	if (!FFileHelper::SaveStringToFile(SerializedJson, *InSaveFilePath))
		UE_LOG(LogDemoJsonCallback, Fatal, TEXT("Unable to save file '%s'."), *InSaveFilePath);
}

FString UCustomImportCallbackCommandlet::SerializeJson(const TSharedPtr<FJsonObject> InJsonObject)
{
	// Serialize
	FString SerializedJson;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&SerializedJson);
	FJsonSerializer::Serialize(InJsonObject.ToSharedRef(), TJsonWriterFactory<>::Create(&SerializedJson));
	
	UE_LOG(LogDemoJsonCallback, Display, TEXT("Export custom result:\n%s"), *SerializedJson);

	return SerializedJson;
}

namespace FCustomCallbacksDemoLocal
{
	// Load data from json file
	TSharedPtr<FJsonValue> LoadJsonFile(FString const& FilePath)
	{
		// load text file
		FString FileText;
		if (!FFileHelper::LoadFileToString(FileText, *FilePath))
			return nullptr;

		// parse as json
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(FileText);
		TSharedPtr<FJsonValue> JsonFile;
		if (!FJsonSerializer::Deserialize(JsonReader, JsonFile))
			return nullptr;

		return JsonFile;
	}
	
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

	// Implementation for CustomExportCallback (Example of use)
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

	// Implementation for my CustomImportCallback (Example of use)
	bool JsonToObjectCallback(const TSharedPtr<FJsonValue>& JsonValue, FProperty* Property , void* OutValue)
	{
		const TSharedPtr<FJsonObject>* JsonObject;
		if (!JsonValue->TryGetObject(JsonObject))
			// By default, false, means not handled
			return false;
		
		FString SubObjectRef;
		if (!(*JsonObject)->TryGetStringField(CustomAdditionalPropertyName, SubObjectRef))
			// JsonObject don`t have custom sub object (instanced object),
			// so the import is not finished and you need to continue the import chain
			// False - means not handled
			return false;
		
		
		// Detect instanced object
		FString ClassName;
		FString PackagePath;
		FString ObjectName;
		FString SubObjectName;
		
		// TODO: https://github.com/EpicGames/UnrealEngine/pull/7371
		// FPackageName::SplitFullObjectPath(StringValue, ClassName, PackagePath, ObjectName, SubObjectName);
		CustomSplitFullObjectPath(SubObjectRef, ClassName, PackagePath, ObjectName, SubObjectName);

		if (SubObjectName.IsEmpty())
			// The textual representation of a link to a sub-object does not contain its name.
			// This may mean that this is not a link to an instantiated object (sub object),
			// but to a third-party object from another package
			// False - means not handled
			return false;

		UClass* ObjectClass = FindObject<UClass>(ANY_PACKAGE, *ClassName);
		if (ObjectClass == nullptr)
		{
			UE_LOG(LogDemoJsonCallback, Fatal, TEXT("Class '%s' not found."), *ClassName);
			return false;
		}

		const FString OuterObjectPath = FString::Printf(TEXT("%s.%s"), *PackagePath, *ObjectName);
		UObject* OuterObject = LoadObject<UObject>(nullptr, *OuterObjectPath);
		UObject* SubObject = NewObject<UObject>(OuterObject, ObjectClass);
		if (SubObject == nullptr)
		{
			UE_LOG(LogDemoJsonCallback, Fatal, TEXT("Sub Object '%s' for object '%s' was not created."), *SubObjectName, *OuterObjectPath);
			return false;
		}
		
		// Let's go through the properties of the sub-object recursively to restore them.
		for (auto && PropertyJsonValuePair : (*JsonObject)->Values)
		{
			if (PropertyJsonValuePair.Key == CustomAdditionalPropertyName)
				continue;
		
			FProperty* SubObjectProperty = ObjectClass->FindPropertyByName(*PropertyJsonValuePair.Key);
			if (!SubObjectProperty)
			{
				UE_LOG(LogDemoJsonCallback, Error, TEXT("Property %s for object class %s not found."), *PropertyJsonValuePair.Key, *ObjectClass->GetName());
				continue;
			}

			/* TODO: Uncomment next lines when CustomImportCallback if it is available in FJsonObjectConverter::JsonValueToUProperty*/
			// FJsonObjectConverter::CustomImportCallback CustomCB;
			// CustomCB.BindStatic(JsonToObjectCallback);
			FJsonObjectConverter::JsonValueToUProperty(
				PropertyJsonValuePair.Value,
				SubObjectProperty,
				SubObjectProperty->ContainerPtrToValuePtr<void>(SubObject),
				0,
				0
				// /* TODO: Uncomment next arg if it is available in FJsonObjectConverter::JsonValueToUProperty*/ , &CustomCB
				);			
		}

		// After the properties for the sub object are restored, you need to write a link to this object in the current property
		if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
			ObjectProperty->SetObjectPropertyValue(OutValue, SubObject);
		
		// All the steps have been passed, the json object has been converted to an object and saved to a property,
		// which means that for this stage the processing is over and there is no point in continuing the import chain for this property
		// True - means to handled
		return true;
	}
}
