# Incorrect function result FPackageName::SplitFullObjectPath #

In the process work, I needed a function that parses textual references to objects and returns the class name, path to the package, the name of the object and subobject (if any). I found that there is such a function in the engine code (`Engine/Source/Runtime/CoreUObject/Private/Misc/PackageName.cpp` - `FPackageName::SplitFullObjectPath`), **but it did not work correctly**. 
_Maybe someone else will need it_.
There is no problem to move this function into my own code and rewrite it to work correctly. But if it is in the engine, then it need returns the correct result there too.

**To demonstrate** BEFORE and AFTER, I made a small **test commandlet** that can be found **here**: https://github.com/lpestl/UE4ContributionCases/blob/main/Source/UE4ContributionCases/SplitFullObjectPathCase/TestsSplitFullObjectPathCommandlet.h

Here are the results of its execution:

BEFORE:
```
[2020.09.29-07.22.32:693][  0]LogTestRef: Display: UTestsSplitFullObjectPathCommandlet::Main => ' -skipcompile -run=TestsSplitFullObjectPath'
[2020.09.29-07.22.32:706][  0]LogTestRef: Display: UTestsSplitFullObjectPathCommandlet::Main => Test # '0'
[2020.09.29-07.22.32:712][  0]LogTestRef: Display: Results:
InString: 'Class'/Script/UE4ContributionCases.SomePrimaryDataAsset''
	OutClassName: 'Class'/Script/UE4ContributionCases.SomePrimaryDataAsset''
	OutPackageName: ''
	OutObjectName: ''
	OutSubObjectName: ''
[2020.09.29-07.22.32:722][  0]LogTestRef: Display: UTestsSplitFullObjectPathCommandlet::Main => Test # '1'
[2020.09.29-07.22.32:727][  0]LogTestRef: Display: Results:
InString: 'SomePrimaryDataAsset'/Game/ExamplesAssets/CustomDataAssets/DA_SomePrimaryDataAsset.DA_SomePrimaryDataAsset''
	OutClassName: 'SomePrimaryDataAsset'/Game/ExamplesAssets/CustomDataAssets/DA_SomePrimaryDataAsset.DA_SomePrimaryDataAsset''
	OutPackageName: ''
	OutObjectName: ''
	OutSubObjectName: ''
[2020.09.29-07.22.32:813][  0]LogTestRef: Display: UTestsSplitFullObjectPathCommandlet::Main => Test # '2'
[2020.09.29-07.22.32:907][  0]LogTestRef: Display: Results:
InString: 'SomeDataAsset'/Game/ExamplesAssets/CustomDataAssets/DA_SomeDataAsset.DA_SomeDataAsset''
	OutClassName: 'SomeDataAsset'/Game/ExamplesAssets/CustomDataAssets/DA_SomeDataAsset.DA_SomeDataAsset''
	OutPackageName: ''
	OutObjectName: ''
	OutSubObjectName: ''
[2020.09.29-07.22.33:011][  0]LogTestRef: Display: UTestsSplitFullObjectPathCommandlet::Main => Test # '3'
[2020.09.29-07.22.33:085][  0]LogTestRef: Display: Results:
InString: 'SecondTypeForInstancing'/Game/ExamplesAssets/CustomDataAssets/DA_SomeDataAsset.DA_SomeDataAsset:SecondTypeForInstancing_0''
	OutClassName: 'SecondTypeForInstancing'/Game/ExamplesAssets/CustomDataAssets/DA_SomeDataAsset.DA_SomeDataAsset:SecondTypeForInstancing_0''
	OutPackageName: ''
	OutObjectName: ''
	OutSubObjectName: ''
[2020.09.29-07.22.33:263][  0]LogInit: Display: 
[2020.09.29-07.22.33:315][  0]LogInit: Display: Success - 0 error(s), 0 warning(s)
[2020.09.29-07.22.33:346][  0]LogInit: Display: 

Execution of commandlet took:  0.65 seconds
```

AFTER:
```
[2020.09.29-07.25.03:398][  0]LogTestRef: Display: UTestsSplitFullObjectPathCommandlet::Main => ' -skipcompile -run=TestsSplitFullObjectPath'
[2020.09.29-07.25.03:515][  0]LogTestRef: Display: UTestsSplitFullObjectPathCommandlet::Main => Test # '0'
[2020.09.29-07.25.03:579][  0]LogTestRef: Display: Results:
InString: 'Class'/Script/UE4ContributionCases.SomePrimaryDataAsset''
	OutClassName: 'Class'
	OutPackageName: '/Script/UE4ContributionCases'
	OutObjectName: 'SomePrimaryDataAsset'
	OutSubObjectName: ''
[2020.09.29-07.25.03:771][  0]LogTestRef: Display: UTestsSplitFullObjectPathCommandlet::Main => Test # '1'
[2020.09.29-07.25.03:850][  0]LogTestRef: Display: Results:
InString: 'SomePrimaryDataAsset'/Game/ExamplesAssets/CustomDataAssets/DA_SomePrimaryDataAsset.DA_SomePrimaryDataAsset''
	OutClassName: 'SomePrimaryDataAsset'
	OutPackageName: '/Game/ExamplesAssets/CustomDataAssets/DA_SomePrimaryDataAsset'
	OutObjectName: 'DA_SomePrimaryDataAsset'
	OutSubObjectName: ''
[2020.09.29-07.25.03:998][  0]LogTestRef: Display: UTestsSplitFullObjectPathCommandlet::Main => Test # '2'
[2020.09.29-07.25.04:061][  0]LogTestRef: Display: Results:
InString: 'SomeDataAsset'/Game/ExamplesAssets/CustomDataAssets/DA_SomeDataAsset.DA_SomeDataAsset''
	OutClassName: 'SomeDataAsset'
	OutPackageName: '/Game/ExamplesAssets/CustomDataAssets/DA_SomeDataAsset'
	OutObjectName: 'DA_SomeDataAsset'
	OutSubObjectName: ''
[2020.09.29-07.25.04:352][  0]LogTestRef: Display: UTestsSplitFullObjectPathCommandlet::Main => Test # '3'
[2020.09.29-07.25.04:478][  0]LogTestRef: Display: Results:
InString: 'SecondTypeForInstancing'/Game/ExamplesAssets/CustomDataAssets/DA_SomeDataAsset.DA_SomeDataAsset:SecondTypeForInstancing_0''
	OutClassName: 'SecondTypeForInstancing'
	OutPackageName: '/Game/ExamplesAssets/CustomDataAssets/DA_SomeDataAsset'
	OutObjectName: 'DA_SomeDataAsset'
	OutSubObjectName: 'SecondTypeForInstancing_0'
[2020.09.29-07.25.04:908][  0]LogInit: Display: 
[2020.09.29-07.25.04:974][  0]LogInit: Display: Success - 0 error(s), 0 warning(s)
[2020.09.29-07.25.05:059][  0]LogInit: Display: 

Execution of commandlet took:  1.66 seconds
```