param(
    [string]$BinaryPath = "",
    [string]$OutputRoot = "",
    [string]$DeviceDir = "",
    [string]$AndroidSerial = ""
)

$ErrorActionPreference = "Stop"

function Write-LoggedOutput {
    param(
        [Parameter(Mandatory = $true)]
        [string]$LogPath,
        [Parameter()]
        [AllowNull()]
        [object[]]$Output,
        [switch]$Append
    )

    if ($null -eq $Output) {
        return
    }

    $lines = @($Output)
    if ($lines.Count -eq 0) {
        return
    }

    $lines | Write-Output
    if ($Append) {
        $lines | Add-Content -Path $LogPath
    } else {
        $lines | Set-Content -Path $LogPath
    }
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$releaseBinary = Join-Path $scriptDir "..\packages\android\pso_benchmark"

if (-not $BinaryPath) {
    if (Test-Path $releaseBinary) {
        $BinaryPath = $releaseBinary
    } else {
        $appRoot = Resolve-Path (Join-Path $scriptDir "..\..")
        $BinaryPath = Join-Path $appRoot "..\build\BenchmarkApp\android-arm64-v8a\platform\android\pso_benchmark"
    }
}

if (-not (Test-Path $BinaryPath)) {
    throw "Android benchmark binary not found: $BinaryPath"
}

if (-not $OutputRoot) {
    if (Test-Path $releaseBinary) {
        $OutputRoot = Join-Path $scriptDir "..\..\release\results"
    } else {
        $appRoot = Resolve-Path (Join-Path $scriptDir "..\..")
        $OutputRoot = Join-Path $appRoot "..\release\results"
    }
}

if (-not $DeviceDir) {
    $DeviceDir = if ($env:ANDROID_DEVICE_DIR) { $env:ANDROID_DEVICE_DIR } else { "/data/local/tmp/pso_benchmark" }
}

$adbCommand = Get-Command adb -ErrorAction Stop
$adbExe = $adbCommand.Source

$deviceListOutput = & $adbExe devices 2>&1
$deviceListExit = $LASTEXITCODE
if ($deviceListExit -ne 0) {
    throw "adb devices failed: $($deviceListOutput -join [Environment]::NewLine)"
}

if (-not $AndroidSerial) {
    if ($env:ANDROID_SERIAL) {
        $AndroidSerial = $env:ANDROID_SERIAL
    } else {
        $AndroidSerial = $deviceListOutput |
            Where-Object { $_ -match '^\S+\s+device$' } |
            ForEach-Object { ($_ -split '\s+')[0] } |
            Select-Object -First 1
    }
}

if (-not $AndroidSerial) {
    throw "No Android device detected. Connect a device or set ANDROID_SERIAL."
}

$deviceName = ((& $adbExe -s $AndroidSerial shell getprop ro.product.model 2>&1) | Out-String).Trim()
$deviceNameExit = $LASTEXITCODE
if ($deviceNameExit -ne 0) {
    throw "Failed to query Android device name for $AndroidSerial."
}

$androidVersion = ((& $adbExe -s $AndroidSerial shell getprop ro.build.version.release 2>&1) | Out-String).Trim()
$androidVersionExit = $LASTEXITCODE
if ($androidVersionExit -ne 0) {
    throw "Failed to query Android OS version for $AndroidSerial."
}

$startedAt = (Get-Date).ToUniversalTime().ToString("yyyy-MM-ddTHH:mm:ssZ")
$dirTimestamp = (Get-Date).ToUniversalTime().ToString("yyyyMMddTHHmmssZ")
$resultDir = Join-Path $OutputRoot "android\$dirTimestamp"
$null = New-Item -ItemType Directory -Path $resultDir -Force

$logPath = Join-Path $resultDir "console.log"
$jsonPath = Join-Path $resultDir "benchmark_report.json"
$csvPath = Join-Path $resultDir "compression_results.csv"

$mkdirOutput = & $adbExe -s $AndroidSerial shell "mkdir -p $DeviceDir" 2>&1
$mkdirExit = $LASTEXITCODE
Write-LoggedOutput -LogPath $logPath -Output $mkdirOutput
if ($mkdirExit -ne 0) {
    throw "Failed to create Android device directory: $DeviceDir"
}

$pushOutput = & $adbExe -s $AndroidSerial push $BinaryPath "$DeviceDir/pso_benchmark" 2>&1
$pushExit = $LASTEXITCODE
Write-LoggedOutput -LogPath $logPath -Output $pushOutput -Append
if ($pushExit -ne 0) {
    throw "Failed to push Android benchmark binary to device."
}

$chmodOutput = & $adbExe -s $AndroidSerial shell "chmod +x $DeviceDir/pso_benchmark" 2>&1
$chmodExit = $LASTEXITCODE
Write-LoggedOutput -LogPath $logPath -Output $chmodOutput -Append
if ($chmodExit -ne 0) {
    throw "Failed to mark Android benchmark binary executable on device."
}

$runOutput = & $adbExe -s $AndroidSerial shell "cd $DeviceDir && ./pso_benchmark --json results.json --csv results.csv" 2>&1
$runExit = $LASTEXITCODE
Write-LoggedOutput -LogPath $logPath -Output $runOutput -Append
if ($runExit -ne 0) {
    throw "Android benchmark execution failed."
}

$pullJsonOutput = & $adbExe -s $AndroidSerial pull "$DeviceDir/results.json" $jsonPath 2>&1
$pullJsonExit = $LASTEXITCODE
Write-LoggedOutput -LogPath $logPath -Output $pullJsonOutput -Append
if ($pullJsonExit -ne 0) {
    throw "Failed to pull Android benchmark JSON results."
}

$pullCsvOutput = & $adbExe -s $AndroidSerial pull "$DeviceDir/results.csv" $csvPath 2>&1
$pullCsvExit = $LASTEXITCODE
Write-LoggedOutput -LogPath $logPath -Output $pullCsvOutput -Append
if ($pullCsvExit -ne 0) {
    throw "Failed to pull Android benchmark CSV results."
}

$finishedAt = (Get-Date).ToUniversalTime().ToString("yyyy-MM-ddTHH:mm:ssZ")
$runInfoPath = Join-Path $resultDir "run_info.txt"
@(
    "Platform: android"
    "Device: $deviceName"
    "Device ID: $AndroidSerial"
    "Started At (UTC): $startedAt"
    "Finished At (UTC): $finishedAt"
    "Source Path: $BinaryPath"
    "JSON: benchmark_report.json"
    "CSV: compression_results.csv"
    "OS Version: Android $androidVersion"
) | Set-Content -Path $runInfoPath

Write-Host "=== Benchmark Complete ==="
Write-Host "Platform: android"
Write-Host "Device: $deviceName"
Write-Host "Started At (UTC): $startedAt"
Write-Host "Finished At (UTC): $finishedAt"
Write-Host "Results Directory: $resultDir"
