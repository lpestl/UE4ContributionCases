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

