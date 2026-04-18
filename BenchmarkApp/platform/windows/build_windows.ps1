param(
    [string]$BuildRoot = "",
    [string]$BuildType = "",
    [string]$Generator = "",
    [string]$Target = "compression_benchmark_cli",
    [switch]$ConfigureOnly
)

$ErrorActionPreference = "Stop"

function Get-BenchmarkBuildType {
    if ($env:BENCHMARK_WINDOWS_CONFIGURATION) { return $env:BENCHMARK_WINDOWS_CONFIGURATION }
    if ($env:CONFIGURATION) { return $env:CONFIGURATION }
    if ($env:BENCHMARK_BUILD_TYPE) { return $env:BENCHMARK_BUILD_TYPE }
    if ($env:CMAKE_BUILD_TYPE) { return $env:CMAKE_BUILD_TYPE }
    return "Release"
}

function Get-BenchmarkBuildJobs {
    if ($env:BENCHMARK_BUILD_JOBS) { return $env:BENCHMARK_BUILD_JOBS }
    if ($env:CMAKE_BUILD_PARALLEL_LEVEL) { return $env:CMAKE_BUILD_PARALLEL_LEVEL }
    return "4"
}

function Get-WindowsBinaryCandidates {
    param(
        [Parameter(Mandatory = $true)]
        [string]$ResolvedBuildRoot,
        [Parameter(Mandatory = $true)]
        [string]$ResolvedBuildType
    )

    return @(
        (Join-Path $ResolvedBuildRoot "windows\platform\windows\compression_benchmark_cli.exe"),
        (Join-Path $ResolvedBuildRoot "windows\platform\windows\$ResolvedBuildType\compression_benchmark_cli.exe"),
        (Join-Path $ResolvedBuildRoot "windows\$ResolvedBuildType\platform\windows\compression_benchmark_cli.exe")
    )
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$appRoot = Resolve-Path (Join-Path $scriptDir "..\..")

if (-not $BuildRoot) {
    if ($env:BUILD_ROOT) {
        $BuildRoot = $env:BUILD_ROOT
    } else {
        $BuildRoot = Join-Path $appRoot "..\build\BenchmarkApp"
    }
}

if (-not $BuildType) {
    $BuildType = Get-BenchmarkBuildType
}

$buildJobs = Get-BenchmarkBuildJobs
$buildDir = Join-Path $BuildRoot "windows"

$configureArgs = @("-S", $appRoot, "-B", $buildDir, "-DCMAKE_BUILD_TYPE=$BuildType")
if ($Generator) {
    $configureArgs += @("-G", $Generator)
}

& cmake @configureArgs
if ($LASTEXITCODE -ne 0) {
    throw "Windows configure failed."
}

if ($ConfigureOnly) {
    Write-Host "Configured Windows benchmark build at: $buildDir"
    exit 0
}

$buildArgs = @("--build", $buildDir, "--target", $Target, "--parallel", $buildJobs, "--config", $BuildType)
& cmake @buildArgs
if ($LASTEXITCODE -ne 0) {
    throw "Windows build failed."
}

$binaryPath = Get-WindowsBinaryCandidates -ResolvedBuildRoot $BuildRoot -ResolvedBuildType $BuildType |
    Where-Object { Test-Path $_ } |
    Select-Object -First 1

if (-not $binaryPath) {
    $binaryPath = (Get-WindowsBinaryCandidates -ResolvedBuildRoot $BuildRoot -ResolvedBuildType $BuildType)[0]
}

Write-Host "Windows benchmark binary built at: $binaryPath"
