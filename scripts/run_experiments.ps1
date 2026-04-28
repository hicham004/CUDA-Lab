[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel")]
    [string]$Config = "Release",

    [string]$BuildDir = "",

    [int[]]$GridSizes = @(64, 128, 256),

    [int]$Steps = 50,

    [uint32]$Seed = 12345,

    [double]$AliveProbability = 0.3,

    [string[]]$BlockSizes = @("8x8x8", "16x8x8", "8x16x8"),

    [bool]$VerifyCuda = $true,

    [switch]$Execute
)

. "$PSScriptRoot\common.ps1"

$repoRoot = Get-RepoRoot
$resolvedBuildDir = if ($BuildDir) { $BuildDir } else { Join-Path $repoRoot "build" }
$timingsDir = Join-Path $repoRoot "results\raw\timings"
$statesDir = Join-Path $repoRoot "results\raw\final_states"
Ensure-Directory -Path $timingsDir
Ensure-Directory -Path $statesDir

$plannedRuns = @()

foreach ($size in $GridSizes) {
    $seqState = Join-Path $statesDir ("seq_N{0}_T{1}_seed{2}.bin" -f $size, $Steps, $Seed)
    $seqMetrics = Join-Path $timingsDir ("seq_N{0}_T{1}_seed{2}.json" -f $size, $Steps, $Seed)
    $seqLog = Join-Path $timingsDir ("seq_N{0}_T{1}_seed{2}.txt" -f $size, $Steps, $Seed)

    $plannedRuns += [pscustomobject]@{
        Label = "seq N=$size"
        LogPath = $seqLog
        Args = @(
            "-ExecutionPolicy", "Bypass",
            "-File", (Join-Path $PSScriptRoot "run_seq.ps1"),
            "-Config", $Config,
            "-BuildDir", $resolvedBuildDir,
            "-Size", $size,
            "-Steps", $Steps,
            "-Seed", $Seed,
            "-AliveProbability", $AliveProbability,
            "-OutputState", $seqState,
            "-MetricsOut", $seqMetrics
        )
    }

    foreach ($blockSize in $BlockSizes) {
        if ($blockSize -notmatch "^(\d+)x(\d+)x(\d+)$") {
            throw "Invalid block size '$blockSize'. Use the format AxBxC, for example 8x8x8."
        }

        $blockX = [int]$Matches[1]
        $blockY = [int]$Matches[2]
        $blockZ = [int]$Matches[3]

        $cudaState = Join-Path $statesDir ("cuda_N{0}_T{1}_seed{2}_B{3}.bin" -f $size, $Steps, $Seed, $blockSize)
        $cudaMetrics = Join-Path $timingsDir ("cuda_N{0}_T{1}_seed{2}_B{3}.json" -f $size, $Steps, $Seed, $blockSize)
        $cudaLog = Join-Path $timingsDir ("cuda_N{0}_T{1}_seed{2}_B{3}.txt" -f $size, $Steps, $Seed, $blockSize)

        $cudaArgs = @(
            "-ExecutionPolicy", "Bypass",
            "-File", (Join-Path $PSScriptRoot "run_cuda.ps1"),
            "-Config", $Config,
            "-BuildDir", $resolvedBuildDir,
            "-Size", $size,
            "-Steps", $Steps,
            "-Seed", $Seed,
            "-AliveProbability", $AliveProbability,
            "-BlockX", $blockX,
            "-BlockY", $blockY,
            "-BlockZ", $blockZ,
            "-OutputState", $cudaState,
            "-MetricsOut", $cudaMetrics
        )

        if ($VerifyCuda) {
            $cudaArgs += "-Verify"
        }

        $plannedRuns += [pscustomobject]@{
            Label = "cuda N=$size B=$blockSize"
            LogPath = $cudaLog
            Args = $cudaArgs
        }
    }
}

if (-not $Execute) {
    Write-Host "Dry run only. Add -Execute to run the planned experiments.`n"

    foreach ($plannedRun in $plannedRuns) {
        $formattedArgs = $plannedRun.Args | ForEach-Object { Quote-Argument -Value ([string]$_) }
        Write-Host ($plannedRun.Label + ":")
        Write-Host ("  powershell " + ($formattedArgs -join " "))
    }

    return
}

foreach ($plannedRun in $plannedRuns) {
    Write-Host ("Running " + $plannedRun.Label)
    $output = & powershell @($plannedRun.Args) 2>&1
    $output | Tee-Object -FilePath $plannedRun.LogPath

    if ($LASTEXITCODE -ne 0) {
        throw ("Experiment failed for " + $plannedRun.Label + ". See " + $plannedRun.LogPath)
    }
}
