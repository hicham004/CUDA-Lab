[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel")]
    [string]$Config = "Release",

    [string]$BuildDir = "",

    [bool]$EnableCuda = $true,

    [string]$CudaArchitectures = ""
)

. "$PSScriptRoot\common.ps1"

Assert-CommandExists -CommandName "cmake"

$repoRoot = Get-RepoRoot
$resolvedBuildDir = if ($BuildDir) { $BuildDir } else { Join-Path $repoRoot "build" }
$enableCudaValue = if ($EnableCuda) { "ON" } else { "OFF" }

$configureArgs = @(
    "-S", $repoRoot,
    "-B", $resolvedBuildDir,
    "-DCMAKE_BUILD_TYPE=$Config",
    "-DCUDA_LAB_ENABLE_CUDA=$enableCudaValue"
)

if ($CudaArchitectures) {
    $configureArgs += "-DCMAKE_CUDA_ARCHITECTURES=$CudaArchitectures"
}

& cmake @configureArgs
& cmake --build $resolvedBuildDir --config $Config
