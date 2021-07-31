# Builds a .qmod file for loading with QuestPatcher
$NDKPath = Get-Content $PSScriptRoot/ndkpath.txt

$buildScript = "$NDKPath/build/ndk-build"
if (-not ($PSVersionTable.PSEdition -eq "Core")) {
    $buildScript += ".cmd"
}

$ArchiveName = "fish-utils_v0.1.0.qmod"
$TempArchiveName = "fish-utils_v0.1.0.qmod.zip"

& $buildScript NDK_PROJECT_PATH=$PSScriptRoot APP_BUILD_SCRIPT=$PSScriptRoot/Android.mk NDK_APPLICATION_MK=$PSScriptRoot/Application.mk
Compress-Archive -Path "./libs/arm64-v8a/libfish-utils.so", "./extern/libbeatsaber-hook_2_2_1.so", "./extern/libbeatsaber-hook_2_2_2.so", "./extern/libbs-utils.so", "./mod.json" -DestinationPath $TempArchiveName -Force
Move-Item $TempArchiveName $ArchiveName -Force