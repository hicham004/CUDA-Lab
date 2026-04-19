[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel")]
    [string]$Config = "Release",

    [string]$BuildDir = "",

    [int]$Size = 64,

    [int]$Steps = 50,

    [uint32]$Seed = 12345,

    [double]$AliveProbability = 0.3,

    [string]$OutputState = "",

    [string]$MetricsOut = ""
)

. "$PSScriptRoot\common.ps1"

$repoRoot = Get-RepoRoot
$resolvedBuildDir = if ($BuildDir) { $BuildDir } else { Join-Path $repoRoot "build" }

$binary = Resolve-BinaryPath -BuildDir $resolvedBuildDir -Config $Config -BinaryName "ca_seq"
$arguments = @(
    "--size", $Size,
    "--steps", $Steps,
    "--seed", $Seed,
    "--alive-prob", $AliveProbability
)

if ($OutputState) {
    $parent = Split-Path -Parent $OutputState
    if ($parent) {
        Ensure-Directory -Path $parent
    }
    $arguments += @("--output-state", $OutputState)
}

if ($MetricsOut) {
    $parent = Split-Path -Parent $MetricsOut
    if ($parent) {
        Ensure-Directory -Path $parent
    }
    $arguments += @("--metrics-out", $MetricsOut)
}

& $binary @arguments
