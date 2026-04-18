param(
    [string]$AppPath = "",
    [string]$OutputRoot = "",
    [int]$PollIntervalSeconds = 5,
    [int]$PollTimeoutSeconds = 900,
    [string]$Device = ""
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

if (-not $IsMacOS) {
    throw "The iOS PowerShell runner requires macOS with PowerShell 7+ and Xcode command line tools."
}

$null = Get-Command xcrun -ErrorAction Stop

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$releaseApp = Join-Path $scriptDir "..\packages\ios\PSOBenchmarkApp.app"
$buildConfiguration = if ($env:BENCHMARK_IOS_CONFIGURATION) {
    $env:BENCHMARK_IOS_CONFIGURATION
} elseif ($env:BENCHMARK_APPLE_CONFIGURATION) {
    $env:BENCHMARK_APPLE_CONFIGURATION
} elseif ($env:CONFIGURATION) {
    $env:CONFIGURATION
} elseif ($env:BENCHMARK_BUILD_TYPE) {
    $env:BENCHMARK_BUILD_TYPE
} elseif ($env:CMAKE_BUILD_TYPE) {
    $env:CMAKE_BUILD_TYPE
} else {
    "Release"
}

if (-not $AppPath) {
    if (Test-Path $releaseApp) {
        $AppPath = $releaseApp
    } else {
        $appRoot = Resolve-Path (Join-Path $scriptDir "..\..")
        $AppPath = Join-Path $appRoot "..\build\BenchmarkApp\ios-device\platform\ios\$buildConfiguration-iphoneos\PSOBenchmarkApp.app"
    }
}

if (-not (Test-Path $AppPath)) {
    throw "iOS app bundle not found: $AppPath"
}

if (-not $OutputRoot) {
    if (Test-Path $releaseApp) {
        $OutputRoot = Join-Path $scriptDir "..\..\benchmark_results"
    } else {
        $appRoot = Resolve-Path (Join-Path $scriptDir "..\..")
        $OutputRoot = Join-Path $appRoot "..\benchmark_results"
    }
}

$infoPlistPath = Join-Path $AppPath "Info.plist"
$bundleId = ((& /usr/bin/plutil -extract CFBundleIdentifier raw -o - $infoPlistPath 2>&1) | Out-String).Trim()
$bundleIdExit = $LASTEXITCODE
if ($bundleIdExit -ne 0 -or [string]::IsNullOrWhiteSpace($bundleId)) {
    throw "Failed to read bundle identifier from: $infoPlistPath"
}

$deviceJson = New-TemporaryFile
try {
    & xcrun devicectl list devices --json-output $deviceJson.FullName | Out-Null
    $deviceListExit = $LASTEXITCODE
    if ($deviceListExit -ne 0) {
        throw "xcrun devicectl list devices failed."
    }

    $devicePayload = Get-Content -Path $deviceJson.FullName -Raw | ConvertFrom-Json
    $devices = @($devicePayload.result.devices)
    if ($devices.Count -eq 0) {
        throw "No iOS devices were reported by xcrun devicectl."
    }

    if ($Device) {
        $selectedDevice = $devices | Where-Object { $_.identifier -eq $Device } | Select-Object -First 1
        if (-not $selectedDevice) {
            throw "Requested iOS device not found: $Device"
        }
    } else {
        $selectedDevice = $devices |
            Where-Object { $_.connectionProperties.tunnelState -ne "unavailable" } |
            Select-Object -First 1
        if (-not $selectedDevice) {
            throw "No available iOS device found. Connect and unlock a device, then try again."
        }
    }

    $deviceIdentifier = $selectedDevice.identifier
    $deviceName = $selectedDevice.deviceProperties.name
    $deviceUdid = $selectedDevice.hardwareProperties.udid
    $marketingName = $selectedDevice.hardwareProperties.marketingName
    $productType = $selectedDevice.hardwareProperties.productType
    $hardwareModel = $selectedDevice.hardwareProperties.hardwareModel

    $startedAt = (Get-Date).ToUniversalTime().ToString("yyyy-MM-ddTHH:mm:ssZ")
    $dirTimestamp = (Get-Date).ToUniversalTime().ToString("yyyyMMddTHHmmssZ")
    $resultDir = Join-Path $OutputRoot "ios\$dirTimestamp"
    $null = New-Item -ItemType Directory -Path $resultDir -Force

    $logPath = Join-Path $resultDir "console.log"
    $uninstallJson = Join-Path $resultDir "uninstall.json"
    $installJson = Join-Path $resultDir "install.json"
    $launchJson = Join-Path $resultDir "launch.json"
    $jsonPath = Join-Path $resultDir "benchmark_report.json"
    $csvPath = Join-Path $resultDir "compression_results.csv"

    Write-Host "Preparing device $deviceName..."
    $uninstallOutput = & xcrun devicectl device uninstall app --device $deviceIdentifier $bundleId --json-output $uninstallJson 2>&1
    Write-LoggedOutput -LogPath $logPath -Output $uninstallOutput

    $installOutput = & xcrun devicectl device install app --device $deviceIdentifier $AppPath --json-output $installJson 2>&1
    $installExit = $LASTEXITCODE
    Write-LoggedOutput -LogPath $logPath -Output $installOutput -Append
    if ($installExit -ne 0) {
        throw "Failed to install iOS app on device $deviceName."
    }

    $launchOutput = & xcrun devicectl device process launch --device $deviceIdentifier --terminate-existing $bundleId --json-output $launchJson 2>&1
    $launchExit = $LASTEXITCODE
    Write-LoggedOutput -LogPath $logPath -Output $launchOutput -Append
    if ($launchExit -ne 0) {
        throw "Failed to launch iOS app on device $deviceName."
    }

    Write-Host "Waiting for benchmark output..."
    $deadline = (Get-Date).ToUniversalTime().AddSeconds($PollTimeoutSeconds)
    $jsonReady = $false
    $csvReady = $false
    while ((Get-Date).ToUniversalTime() -lt $deadline) {
        & xcrun devicectl device copy from `
            --device $deviceIdentifier `
            --domain-type appDataContainer `
            --domain-identifier $bundleId `
            --source "Documents/compression_results.json" `
            --destination $jsonPath *> $null
        $jsonReady = ($LASTEXITCODE -eq 0)

        & xcrun devicectl device copy from `
            --device $deviceIdentifier `
            --domain-type appDataContainer `
            --domain-identifier $bundleId `
            --source "Documents/compression_results.csv" `
            --destination $csvPath *> $null
        $csvReady = ($LASTEXITCODE -eq 0)

        if ($jsonReady -and $csvReady) {
            break
        }

        Start-Sleep -Seconds $PollIntervalSeconds
    }

    if (-not (Test-Path $jsonPath) -or -not (Test-Path $csvPath)) {
        throw "Timed out waiting for iOS benchmark results. Check the app on device and inspect: $resultDir"
    }

    $finishedAt = (Get-Date).ToUniversalTime().ToString("yyyy-MM-ddTHH:mm:ssZ")
    $runInfoPath = Join-Path $resultDir "run_info.txt"
    @(
        "Platform: ios"
        "Device: $deviceName"
        "Device ID: $deviceUdid"
        "Started At (UTC): $startedAt"
        "Finished At (UTC): $finishedAt"
        "Source Path: $AppPath"
        "JSON: benchmark_report.json"
        "CSV: compression_results.csv"
        "Bundle Identifier: $bundleId"
        "Device Identifier: $deviceIdentifier"
        $(if ($marketingName) { "Marketing Name: $marketingName" })
        $(if ($productType) { "Product Type: $productType" })
        $(if ($hardwareModel) { "Hardware Model: $hardwareModel" })
    ) | Set-Content -Path $runInfoPath

    Write-Host "=== Benchmark Complete ==="
    Write-Host "Platform: ios"
    Write-Host "Device: $deviceName"
    Write-Host "Started At (UTC): $startedAt"
    Write-Host "Finished At (UTC): $finishedAt"
    Write-Host "Results Directory: $resultDir"
}
finally {
    if (Test-Path $deviceJson.FullName) {
        Remove-Item $deviceJson.FullName -Force
    }
}
