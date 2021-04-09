$NDKPath = Get-Content $PSScriptRoot/ndkpath.txt
$NDKStack = "$NDKPath/ndk-stack"

& $NDKStack -sym ./libs/arm64-v8a -dump stackTrace.log