# Need for CustomImportCallback in the implementation of FJsonObjectConverter #

I am writing a small but useful tool for my own purposes, with the help of which it will be possible to export data from DataAssets to json files and import this data from json files into existing DataAssetts, or create new DataAssets if necessary.
While working with the `FJsonObjectConverter` class, I noticed a strange fragment. To export data to json in the function parameters, it is possible to set `CustomExportCallback` in order to override the export behavior in special cases. But for importing data from json, similar to `CustomImportCallback` is not defined. This is really weird and makes things difficult.

I suggest, in the same way, as in the case of export, to create a callback for import.

## At the moment in the engine code ##

In the class `FJsonObjectConverter` there is such a fragment:
```C++
// UStruct -> JSON

	/**
	 * Optional callback to run when exporting a type which we don't already understand.
	 * If this returns a valid pointer it will be inserted into the export chain. If not, or if this is not
	 * passed in, then we will call the generic ToString on the type and export as a JSON string.
	 */
	DECLARE_DELEGATE_RetVal_TwoParams(TSharedPtr<FJsonValue>, CustomExportCallback, FProperty* /* Property */, const void* /* Value */);
```

And in all export-related functions, an optional parameter is forwarded:
```C++
const CustomExportCallback* ExportCb
```

As exaple:
```C++
/* * Converts from a FProperty to a Json Value using exportText
*
* @param Property			The property to export
* @param Value				Pointer to the value of the property
* @param CheckFlags		Only convert properties that match at least one of these flags. If 0 check all properties.
* @param SkipFlags			Skip properties that match any of these flags
* @param ExportCb			Optional callback for types we don't understand. This is called right before falling back to the generic ToString()
*
* @return					The constructed JsonValue from the property
*/
static TSharedPtr<FJsonValue> UPropertyToJsonValue(FProperty* Property, const void* Value, int64 CheckFlags, int64 SkipFlags, const CustomExportCallback* ExportCb = nullptr);
```

But for example, a similar function for import has the following description:
```C++
// JSON -> UStruct
	
	/**
	 * Converts a single JsonValue to the corresponding FProperty (this may recurse if the property is a UStruct for instance).
	 *
	 * @param JsonValue The value to assign to this property
	 * @param Property The FProperty definition of the property we're setting.
	 * @param OutValue Pointer to the property instance to be modified.
	 * @param CheckFlags Only convert sub-properties that match at least one of these flags. If 0 check all properties.
	 * @param SkipFlags Skip sub-properties that match any of these flags
	 *
	 * @return False if the property failed to serialize
	 */
	static bool JsonValueToUProperty(const TSharedPtr<FJsonValue>& JsonValue, FProperty* Property, void* OutValue, int64 CheckFlags, int64 SkipFlags);
```

## I suggest in the Pull Request: ##

* Add declaration CustomImportCallback:
```C++
// JSON -> UStruct
	/**
	* Optional callback to run when importing a type which we don't already understand.
	* If this returns a true bool value it will be inserted into the import chain and be handled. No need continued importing chain. 
	* If this returns a false bool value, then JsonValue not have been handled and need continued importing chain
	*/
	DECLARE_DELEGATE_RetVal_ThreeParams(bool, CustomImportCallback, const TSharedPtr<FJsonValue>& /*JsonValue*/,  FProperty* /* Property */, void* /* OutValue */);
```

* Pass this callback as an optional parameter to all import-related functions, as example:
```C++
	/**
	 * Converts a single JsonValue to the corresponding FProperty (this may recurse if the property is a UStruct for instance).
	 *
	 * @param JsonValue The value to assign to this property
	 * @param Property The FProperty definition of the property we're setting.
	 * @param OutValue Pointer to the property instance to be modified.
	 * @param CheckFlags Only convert sub-properties that match at least one of these flags. If 0 check all properties.
	 * @param SkipFlags Skip sub-properties that match any of these flags
	 * @param ImportCb Optional callback for types we don't understand or want to override the standard conversion. This is called right before returning to the JsonValue general import chain
	 *
	 * @return False if the property failed to serialize
	 */
	static bool JsonValueToUProperty(const TSharedPtr<FJsonValue>& JsonValue, FProperty* Property, void* OutValue, int64 CheckFlags, int64 SkipFlags, const CustomImportCallback* ImportCb = nullptr);
```

* And similarly, as in the case of export, call this callback at the beginning of the `bool ConvertScalarJsonValueToFPropertyWithContainer` function and, depending on the returned result, continue the import chain or stop it:
```C++
    /** Convert JSON to property, assuming either the property is not an array or the value is an individual array element */
	bool ConvertScalarJsonValueToFPropertyWithContainer(const TSharedPtr<FJsonValue>& JsonValue, FProperty* Property, void* OutValue, const UStruct* ContainerStruct, void* Container, int64 CheckFlags, int64 SkipFlags, const FJsonObjectConverter::CustomImportCallback* ImportCb)
	{
		// See if there's a custom import callback first, so it can override default behavior
		if (ImportCb && ImportCb->IsBound())
		{
			if (ImportCb->Execute(JsonValue, Property, OutValue))
				return true;
			// fall through to default cases
		}
		
        if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property)) {...}
        else {...}

        return true;
    }
```

## Demonstration of the need for this feature ##

I created a special repository to demonstrate cases in which this feature is VERY needed: https://github.com/lpestl/UE4ContributionCases

For a simple and step-by-step reproduction of the problem, there is a special Commandlet class ([CustomImportCallbackCommandlet.h](https://github.com/lpestl/UE4ContributionCases/blob/main/Source/UE4ContributionCases/JsonObjectConverter/CustomImportCallbackCommandlet.h) and [CustomImportCallbackCommandlet.cpp](https://github.com/lpestl/UE4ContributionCases/blob/main/Source/UE4ContributionCases/JsonObjectConverter/CustomImportCallbackCommandlet.cpp)), by running which you can see the output in the log, and the source code file contains a detailed description of the steps.

In the above example, after exporting data from DataAsset to Json using `CustomExportCallback`, I then try to restore the data in DataAsset by importing a Json file. But without the presence of `CustomImportCallback`, the data is lost...

Output to the console before we add the ability to define CustomImportCallback:
```
[2020.09.30-09.19.56:580][  0]LogDemoJsonCallback: Display: UCustomImportCallbackCommandlet::Main => Params: ' -skipcompile -run=CustomImportCallback'
[2020.09.30-09.19.56:612][  0]LogDemoJsonCallback: Display: UCustomImportCallbackCommandlet::Main => Step 1: Export DA_SomeDataAsset to CustomExportData.json using FJsonObjectConverter::CustomExportCallback.
[2020.09.30-09.19.56:631][  0]LogDemoJsonCallback: Display: --- Demo for FJsonObjectConverter::CustomExportCallback started ---
[2020.09.30-09.19.56:644][  0]LogDemoJsonCallback: Display: --- Demo for FJsonObjectConverter::CustomExportCallback succesfull finished ---
[2020.09.30-09.19.56:675][  0]LogDemoJsonCallback: Display: Export custom result:
{
	"ArrayStructWithInstancedObject": [
		{
			"objectForInstancing":
			{
				"SubObjectRef": "FirstTypeForInstancing'/Game/ExamplesAssets/CustomDataAssets/DA_SomeDataAsset.DA_SomeDataAsset:FirstTypeForInstancing_0'",
				"SomeString": "String in property"
			}
		}
	],
	"NativeClass": "Class'/Script/UE4ContributionCases.SomeDataAsset'"
}
[2020.09.30-09.19.56:712][  0]LogDemoJsonCallback: Display: UCustomImportCallbackCommandlet::Main => Step 2: Rewrite the instanced subobject from the property in DA_SomeDataAsset to demonstrate.
[2020.09.30-09.19.56:719][  0]LogDemoJsonCallback: Display: UCustomImportCallbackCommandlet::Main => Property 'ArrayStructWithInstancedObject' rewrited on empty Array.
[2020.09.30-09.19.56:738][  0]LogDemoJsonCallback: Display: UCustomImportCallbackCommandlet::Main => After changed property and save package, DataAsset content:
[2020.09.30-09.19.56:784][  0]LogDemoJsonCallback: Display: --- Demo for FJsonObjectConverter::CustomExportCallback started ---
[2020.09.30-09.19.56:790][  0]LogDemoJsonCallback: Display: --- Demo for FJsonObjectConverter::CustomExportCallback succesfull finished ---
[2020.09.30-09.19.56:829][  0]LogDemoJsonCallback: Display: Export custom result:
{
	"ArrayStructWithInstancedObject": [],
	"NativeClass": "Class'/Script/UE4ContributionCases.SomeDataAsset'"
}
[2020.09.30-09.19.56:844][  0]LogDemoJsonCallback: Display: UCustomImportCallbackCommandlet::Main => Step 3: Let's try to recover the DA_SomeDataAsset from the CustomExportData.json file
[2020.09.30-09.19.56:851][  0]LogDemoJsonCallback: Display: --- Demo for FJsonObjectConverter::CustomImportCallback started ---
[2020.09.30-09.19.56:877][  0]LogDemoJsonCallback: Display: Succesful save DA_SomeDataAsset after importing data from json file
[2020.09.30-09.19.56:898][  0]LogDemoJsonCallback: Display: --- Demo for FJsonObjectConverter::CustomImportCallback succesfull finished ---
[2020.09.30-09.19.56:902][  0]LogDemoJsonCallback: Display: UCustomImportCallbackCommandlet::Main => After import (an attempt to restore the original data).
[2020.09.30-09.19.56:911][  0]LogDemoJsonCallback: Display: --- Demo for FJsonObjectConverter::CustomExportCallback started ---
[2020.09.30-09.19.56:916][  0]LogDemoJsonCallback: Display: --- Demo for FJsonObjectConverter::CustomExportCallback succesfull finished ---
[2020.09.30-09.19.56:929][  0]LogDemoJsonCallback: Display: Export custom result:
{
	"ArrayStructWithInstancedObject": [
		{
			"objectForInstancing": "SomeClassForInstancedProperties'/Engine/Transient.SomeClassForInstancedProperties_0'"
		}
	],
	"NativeClass": "Class'/Script/UE4ContributionCases.SomeDataAsset'"
}
[2020.09.30-09.19.56:942][  0]LogInit: Display: 
[2020.09.30-09.19.56:943][  0]LogInit: Display: Success - 0 error(s), 0 warning(s)
[2020.09.30-09.19.56:946][  0]LogInit: Display: 
Execution of commandlet took:  0.37 seconds
```

Output to the console after I added the changes suggested in this pull request to the engine code:
```
[2020.09.30-11.31.04:241][  0]LogDemoJsonCallback: Display: UCustomImportCallbackCommandlet::Main => Params: ' -skipcompile -run=CustomImportCallback'
[2020.09.30-11.31.04:252][  0]LogDemoJsonCallback: Display: UCustomImportCallbackCommandlet::Main => Step 1: Export DA_SomeDataAsset to CustomExportData.json using FJsonObjectConverter::CustomExportCallback.
[2020.09.30-11.31.04:264][  0]LogDemoJsonCallback: Display: --- Demo for FJsonObjectConverter::CustomExportCallback started ---
[2020.09.30-11.31.04:272][  0]LogDemoJsonCallback: Display: --- Demo for FJsonObjectConverter::CustomExportCallback succesfull finished ---
[2020.09.30-11.31.04:286][  0]LogDemoJsonCallback: Display: Export custom result:
{
	"ArrayStructWithInstancedObject": [
		{
			"objectForInstancing":
			{
				"SubObjectRef": "FirstTypeForInstancing'/Game/ExamplesAssets/CustomDataAssets/DA_SomeDataAsset.DA_SomeDataAsset:FirstTypeForInstancing_0'",
				"SomeString": "String in property"
			}
		}
	],
	"NativeClass": "Class'/Script/UE4ContributionCases.SomeDataAsset'"
}
[2020.09.30-11.31.04:328][  0]LogDemoJsonCallback: Display: UCustomImportCallbackCommandlet::Main => Step 2: Rewrite the instanced subobject from the property in DA_SomeDataAsset to demonstrate.
[2020.09.30-11.31.04:425][  0]LogDemoJsonCallback: Display: UCustomImportCallbackCommandlet::Main => Property 'ArrayStructWithInstancedObject' rewrited on empty Array.
[2020.09.30-11.31.04:472][  0]LogDemoJsonCallback: Display: UCustomImportCallbackCommandlet::Main => After changed property and save package, DataAsset content:
[2020.09.30-11.31.04:514][  0]LogDemoJsonCallback: Display: --- Demo for FJsonObjectConverter::CustomExportCallback started ---
[2020.09.30-11.31.04:563][  0]LogDemoJsonCallback: Display: --- Demo for FJsonObjectConverter::CustomExportCallback succesfull finished ---
[2020.09.30-11.31.04:568][  0]LogDemoJsonCallback: Display: Export custom result:
{
	"ArrayStructWithInstancedObject": [],
	"NativeClass": "Class'/Script/UE4ContributionCases.SomeDataAsset'"
}
[2020.09.30-11.31.04:600][  0]LogDemoJsonCallback: Display: UCustomImportCallbackCommandlet::Main => Step 3: Let's try to recover the DA_SomeDataAsset from the CustomExportData.json file
[2020.09.30-11.31.04:656][  0]LogDemoJsonCallback: Display: --- Demo for FJsonObjectConverter::CustomImportCallback started ---
[2020.09.30-11.31.04:662][  0]LogDemoJsonCallback: Display: Succesful save DA_SomeDataAsset after importing data from json file
[2020.09.30-11.31.04:698][  0]LogDemoJsonCallback: Display: --- Demo for FJsonObjectConverter::CustomImportCallback succesfull finished ---
[2020.09.30-11.31.04:717][  0]LogDemoJsonCallback: Display: UCustomImportCallbackCommandlet::Main => After import (an attempt to restore the original data).
[2020.09.30-11.31.04:726][  0]LogDemoJsonCallback: Display: --- Demo for FJsonObjectConverter::CustomExportCallback started ---
[2020.09.30-11.31.04:733][  0]LogDemoJsonCallback: Display: --- Demo for FJsonObjectConverter::CustomExportCallback succesfull finished ---
[2020.09.30-11.31.04:752][  0]LogDemoJsonCallback: Display: Export custom result:
{
	"ArrayStructWithInstancedObject": [
		{
			"objectForInstancing":
			{
				"SubObjectRef": "FirstTypeForInstancing'/Game/ExamplesAssets/CustomDataAssets/DA_SomeDataAsset.DA_SomeDataAsset:FirstTypeForInstancing_1'",
				"SomeString": "String in property"
			}
		}
	],
	"NativeClass": "Class'/Script/UE4ContributionCases.SomeDataAsset'"
}
[2020.09.30-11.31.04:789][  0]LogInit: Display: 
[2020.09.30-11.31.04:816][  0]LogInit: Display: Success - 0 error(s), 0 warning(s)
[2020.09.30-11.31.04:843][  0]LogInit: Display: 
Execution of commandlet took:  0.60 seconds
```