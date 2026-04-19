Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Get-RepoRoot {
    return Split-Path -Parent $PSScriptRoot
}

function Assert-CommandExists {
    param(
        [Parameter(Mandatory = $true)]
        [string]$CommandName
    )

    if (-not (Get-Command $CommandName -ErrorAction SilentlyContinue)) {
        throw "Required command '$CommandName' was not found on PATH."
    }
}

function Ensure-Directory {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    if (-not (Test-Path -LiteralPath $Path)) {
        New-Item -ItemType Directory -Path $Path -Force | Out-Null
    }
}

function Resolve-BinaryPath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$BuildDir,

        [Parameter(Mandatory = $true)]
        [string]$Config,

        [Parameter(Mandatory = $true)]
        [string]$BinaryName
    )

    $candidateRoots = @(
        (Join-Path $BuildDir ("bin\" + $Config)),
        (Join-Path $BuildDir "bin"),
        (Join-Path $BuildDir $Config),
        $BuildDir
    )

    foreach ($root in $candidateRoots) {
        $plainCandidate = Join-Path $root $BinaryName
        $exeCandidate = $plainCandidate + ".exe"

        if (Test-Path -LiteralPath $plainCandidate) {
            return (Resolve-Path -LiteralPath $plainCandidate).Path
        }

        if (Test-Path -LiteralPath $exeCandidate) {
            return (Resolve-Path -LiteralPath $exeCandidate).Path
        }
    }

    throw "Could not find binary '$BinaryName' under build directory '$BuildDir'."
}

function Quote-Argument {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Value
    )

    if ($Value -match "\s") {
        return '"' + $Value + '"'
    }

    return $Value
}
