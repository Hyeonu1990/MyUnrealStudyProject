// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class MyUnrealStudyProjectTarget : TargetRules
{
	public MyUnrealStudyProjectTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "MyUnrealStudyProject" } );

		// Xcode 13.3 업데이트 이후 빌드 안되는 현상을 위한 수정
		bOverrideBuildEnvironment = true;
		AdditionalCompilerArguments = "-Wno-unused-but-set-variable";
	}
}
