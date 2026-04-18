param(
    [string]$BinaryPath = "",
    [string]$OutputRoot = ""
)

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$releaseBinary = Join-Path $scriptDir "..\packages\windows\compression_benchmark_cli.exe"
$appRoot = Resolve-Path (Join-Path $scriptDir "..\..")
$buildConfiguration = if ($env:BENCHMARK_WINDOWS_CONFIGURATION) {
    $env:BENCHMARK_WINDOWS_CONFIGURATION
} elseif ($env:CONFIGURATION) {
    $env:CONFIGURATION
} elseif ($env:BENCHMARK_BUILD_TYPE) {
    $env:BENCHMARK_BUILD_TYPE
} elseif ($env:CMAKE_BUILD_TYPE) {
    $env:CMAKE_BUILD_TYPE
} else {
    "Release"
}

if (-not $BinaryPath) {
    if (Test-Path $releaseBinary) {
        $BinaryPath = $releaseBinary
    } else {
        $buildRoot = if ($env:BUILD_ROOT) {
            $env:BUILD_ROOT
        } else {
            Join-Path $appRoot "..\build\BenchmarkApp"
        }

        $binaryCandidates = @(
            (Join-Path $buildRoot "windows\platform\windows\compression_benchmark_cli.exe"),
            (Join-Path $buildRoot "windows\platform\windows\$buildConfiguration\compression_benchmark_cli.exe"),
            (Join-Path $buildRoot "windows\$buildConfiguration\platform\windows\compression_benchmark_cli.exe")
        )

        $BinaryPath = $binaryCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
        if (-not $BinaryPath) {
            $BinaryPath = $binaryCandidates[0]
        }
    }
}

if (-not (Test-Path $BinaryPath)) {
    throw "Windows benchmark binary not found: $BinaryPath`nBuild a $buildConfiguration binary with: BenchmarkApp\\platform\\windows\\build_windows.ps1"
}

if (-not $OutputRoot) {
    if (Test-Path $releaseBinary) {
        $OutputRoot = Join-Path $scriptDir "..\..\benchmark_results"
    } else {
        $OutputRoot = Join-Path $appRoot "..\benchmark_results"
    }
}

$startedAt = (Get-Date).ToUniversalTime().ToString("yyyy-MM-ddTHH:mm:ssZ")
$dirTimestamp = (Get-Date).ToUniversalTime().ToString("yyyyMMddTHHmmssZ")
$resultDir = Join-Path $OutputRoot "windows\$dirTimestamp"
$null = New-Item -ItemType Directory -Path $resultDir -Force

$jsonPath = Join-Path $resultDir "benchmark_report.json"
$csvPath = Join-Path $resultDir "compression_results.csv"
$logPath = Join-Path $resultDir "console.log"

& $BinaryPath --json $jsonPath --csv $csvPath | Tee-Object -FilePath $logPath

$finishedAt = (Get-Date).ToUniversalTime().ToString("yyyy-MM-ddTHH:mm:ssZ")
$runInfoPath = Join-Path $resultDir "run_info.txt"
@(
    "Platform: windows"
    "Device: $env:COMPUTERNAME"
    "Device ID: $env:COMPUTERNAME"
    "Started At (UTC): $startedAt"
    "Finished At (UTC): $finishedAt"
    "Source Path: $BinaryPath"
    "JSON: benchmark_report.json"
    "CSV: compression_results.csv"
) | Set-Content -Path $runInfoPath

Write-Host "=== Benchmark Complete ==="
Write-Host "Platform: windows"
Write-Host "Device: $env:COMPUTERNAME"
Write-Host "Started At (UTC): $startedAt"
Write-Host "Finished At (UTC): $finishedAt"
Write-Host "Results Directory: $resultDir"
