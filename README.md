# UE4ContributionCases #

 Here are examples of my participation in UE4. Bugs were discovered, reproduced before and after the fix. The project also includes a rationale for the necessary new features in the engine.

 **Content** - The content folder includes several assets that will be needed to reproduce bugs and/or demonstrate features.  
 **Source\UE4ContributionCases** - the code folder includes subfolders for the implementation of classes and functions of some cases. Each subfolder contains a description of a specific problem or a justification for the need for a feature. Each subfolder contains an implementation of its own Commandlet class for a simple demonstration.

To run a Commandlet, on Windows OS use the command line parameters:
```
%UE4EnginePath%\Engine\Binaries\Win64\UE4Editor.exe %UE4ContributionCases_FolderPath%\UE4ContributionCases.uproject -run=%CommandletName%
```
Where:
* `%UE4EnginePath%` - your personal specific path to the UE4 folder. As example my path: `C:\Source\UnrealEngine\`
* `%UE4ContributionCases_FolderPath%` - your personal specific path to the folder by this repository.As example my path: `C:\Source\Projects\UE4ContributionCases\`
* `%CommandletName%` - the specific name of the Commandlet you want to run. As an example for the first case, this is: `TestsSplitFullObjectPath`

## Cases ##

1. [Incorrect function result FPackageName::SplitFullObjectPath](Source/UE4ContributionCases/SplitFullObjectPathCase/README.md)
1. [Need for CustomImportCallback in the implementation of FJsonObjectConverter](Source/UE4ContributionCases/JsonObjectConverter/README.md)
