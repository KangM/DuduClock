param(
    [Alias("p")]
    [string]$Port = "COM5",
    [Alias("b")]
    [int]$Baud = 115200,
    [string]$SketchPath,
    [Alias("c")]
    [switch]$Clean,
    [Alias("nm")]
    [switch]$NoMonitor,
    [Alias("m")]
    [switch]$Monitor,
    [Alias("d")]
    [switch]$Dtr,
    [Alias("r")]
    [switch]$Rts,
    [switch]$UseTftFlags,
    [ValidateSet("default", "c3-240x320", "custom")]
    [string]$TftPreset = "default",
    [int]$TftWidth = 240,
    [int]$TftHeight = 320,
    [int]$TftMosi = 3,
    [int]$TftSclk = 2,
    [int]$TftCs = 7,
    [int]$TftDc = 4,
    [int]$TftRst = 5,
    [ValidateSet("TFT_RGB", "TFT_BGR")]
    [string]$TftRgbOrder = "TFT_BGR",
    [int]$SpiFrequency = 40000000,
    [int]$SpiReadFrequency = 20000000,
    [switch]$Help
)

$ErrorActionPreference = "Stop"

$SketchDir = if ($SketchPath) {
    $SketchPath
} else {
    Join-Path $PSScriptRoot "DuduClock.ino"
}
$LibrariesDir = Join-Path $PSScriptRoot "libraries"
$Fqbn = "esp32:esp32:AirM2M_CORE_ESP32C3:UploadSpeed=921600,CDCOnBoot=default,CPUFreq=80,FlashFreq=80,PartitionScheme=huge_app,DebugLevel=none,EraseFlash=all"
$BuildTime = Get-Date

if ($TftPreset -eq "default") {
    $UseTftFlags = $false
} elseif ($TftPreset -eq "c3-240x320") {
    $UseTftFlags = $true
    $TftWidth = 240
    $TftHeight = 320
    $TftMosi = 3
    $TftSclk = 2
    $TftCs = 7
    $TftDc = 4
    $TftRst = 5
    $TftRgbOrder = "TFT_BGR"
}

function Build-TftFlagString {
    return @(
        "-DUSER_SETUP_LOADED=1",
        "-DST7789_DRIVER=1",
        "-DTFT_WIDTH=$TftWidth",
        "-DTFT_HEIGHT=$TftHeight",
        "-DTFT_RGB_ORDER=$TftRgbOrder",
        "-DTFT_MISO=-1",
        "-DTFT_MOSI=$TftMosi",
        "-DTFT_SCLK=$TftSclk",
        "-DTFT_CS=$TftCs",
        "-DTFT_DC=$TftDc",
        "-DTFT_RST=$TftRst",
        "-DSPI_FREQUENCY=$SpiFrequency",
        "-DSPI_READ_FREQUENCY=$SpiReadFrequency",
        "-DSMOOTH_FONT=1"
    ) -join " "
}

$BuildTimeFlags = @(
    "-DBUILD_TIME_YY=$($BuildTime.ToString('yy'))",
    "-DBUILD_TIME_MM=$($BuildTime.ToString('MM'))",
    "-DBUILD_TIME_DD=$($BuildTime.ToString('dd'))",
    "-DBUILD_TIME_HH=$($BuildTime.ToString('HH'))",
    "-DBUILD_TIME_MIN=$($BuildTime.ToString('mm'))",
    "-DBUILD_TIME_SS=$($BuildTime.ToString('ss'))"
) -join " "
$TftFlags = ""
if ($UseTftFlags) {
    $TftFlags = Build-TftFlagString
}

function Show-Usage {
    $scriptName = Split-Path -Leaf $PSCommandPath
    Write-Host @"
Usage:
  .\$scriptName
  .\$scriptName -Port COM5 -TftPreset c3-240x320
  .\$scriptName -Port COM5 -UseTftFlags -TftMosi 3 -TftSclk 2 -TftCs 7 -TftDc 4 -TftRst 5
  .\$scriptName -Monitor
  .\$scriptName -NoMonitor

Options:
  -Port, -p <COM>              Serial port. Default: COM5
  -Baud, -b <baud>             Serial monitor baud rate. Default: 115200
  -SketchPath <dir>            Optional sketch folder to compile/upload
  -Clean, -c                   Clean build output before compiling
  -NoMonitor, -nm              Compile and upload only
  -Monitor, -m                 Open monitor only
  -UseTftFlags                 Inject TFT_eSPI compile-time pin flags
  -TftPreset <default|c3-240x320|custom>
  -TftWidth, -TftHeight        TFT resolution
  -TftMosi, -TftSclk          TFT SPI pins
  -TftCs, -TftDc, -TftRst     TFT control pins
  -TftRgbOrder                 TFT_RGB or TFT_BGR
  -SpiFrequency                SPI write clock
  -SpiReadFrequency            SPI read clock
  -Dtr, -d                     Assert DTR while monitoring
  -Rts, -r                     Assert RTS while monitoring
  -Help, -h, -?                Show this help
"@
}

if ($Help) {
    Show-Usage
    return
}

function Start-SerialMonitor {
    param([string]$PortName, [int]$BaudRate, [bool]$AssertDtr, [bool]$AssertRts)

    Write-Host ""
    Write-Host "=== Serial monitor $PortName @ $BaudRate ===" -ForegroundColor Cyan
    Write-Host "Type a line and press Enter to send. Local commands: :help, :q" -ForegroundColor DarkCyan
    Write-Host ("DTR={0}, RTS={1}" -f $AssertDtr, $AssertRts) -ForegroundColor DarkGray

    $sp = New-Object System.IO.Ports.SerialPort $PortName, $BaudRate, ([System.IO.Ports.Parity]::None), 8, ([System.IO.Ports.StopBits]::One)
    $sp.Encoding = [System.Text.Encoding]::UTF8
    $sp.ReadTimeout = 200
    $sp.NewLine = "`r`n"
    $sp.DtrEnable = $AssertDtr
    $sp.RtsEnable = $AssertRts

    function Open-SerialPort {
        if ($sp.IsOpen) { return $true }
        try {
            $sp.Open()
            $sp.DtrEnable = $AssertDtr
            $sp.RtsEnable = $AssertRts
            Write-Host "Serial port opened." -ForegroundColor DarkGray
            return $true
        } catch {
            return $false
        }
    }

    if (-not (Open-SerialPort)) {
        Write-Host "Waiting for serial port $PortName..." -ForegroundColor Yellow
    }

    $buffer = ""
    $inputLine = ""

    function Write-SerialPrompt {
        param([string]$Text)
        Write-Host -NoNewline ("> " + $Text)
    }

    function Show-MonitorHelp {
        Write-Host ""
        Write-Host "Local monitor commands:" -ForegroundColor Cyan
        Write-Host "  :help    Show this help"
        Write-Host "  :q       Quit monitor"
        Write-Host ""
        Write-Host "Any other line is sent to the serial port with CRLF."
        Write-SerialPrompt $inputLine
    }

    try {
        Write-SerialPrompt $inputLine
        while ($true) {
            if (-not $sp.IsOpen) {
                if (Open-SerialPort) {
                    Write-SerialPrompt $inputLine
                } else {
                    Start-Sleep -Milliseconds 500
                    continue
                }
            }

            try {
                $chunk = $sp.ReadExisting()
            } catch [TimeoutException] {
                $chunk = ""
            } catch [System.InvalidOperationException] {
                Write-Host "`nSerial port disconnected, waiting for reconnect..." -ForegroundColor Yellow
                try { $sp.Close() } catch {}
                Start-Sleep -Milliseconds 500
                continue
            } catch [System.IO.IOException] {
                Write-Host "`nSerial port I/O error, waiting for reconnect..." -ForegroundColor Yellow
                try { $sp.Close() } catch {}
                Start-Sleep -Milliseconds 500
                continue
            }

            if ($chunk.Length -gt 0) {
                $buffer += $chunk
                while ($buffer.Contains("`n")) {
                    $idx = $buffer.IndexOf("`n")
                    $line = $buffer.Substring(0, $idx).TrimEnd("`r")
                    $buffer = $buffer.Substring($idx + 1)
                    $ts = (Get-Date).ToString("HH:mm:ss.fff")
                    Write-Host "`r$(' ' * ([Console]::CursorLeft + 2))`r" -NoNewline
                    Write-Host "[$ts] $line"
                    Write-SerialPrompt $inputLine
                }
            }

            while ([Console]::KeyAvailable) {
                $key = [Console]::ReadKey($true)

                if ($key.Key -eq [ConsoleKey]::Enter) {
                    Write-Host ""
                    $lineToSend = $inputLine
                    $inputLine = ""

                    if ($lineToSend -eq ":q") { return }
                    if ($lineToSend -eq ":help") {
                        Show-MonitorHelp
                        continue
                    }
                    if ($lineToSend.Length -gt 0) {
                        if ($sp.IsOpen) {
                            try {
                                $sp.WriteLine($lineToSend)
                            } catch {
                                Write-Host "Send failed, serial port is reconnecting." -ForegroundColor Yellow
                                try { $sp.Close() } catch {}
                            }
                        } else {
                            Write-Host "Send skipped, serial port is not open." -ForegroundColor Yellow
                        }
                    }
                    Write-SerialPrompt $inputLine
                    continue
                }

                if ($key.Key -eq [ConsoleKey]::Backspace) {
                    if ($inputLine.Length -gt 0) {
                        $inputLine = $inputLine.Substring(0, $inputLine.Length - 1)
                        Write-Host "`b `b" -NoNewline
                    }
                    continue
                }

                if ($key.Key -eq [ConsoleKey]::Escape) {
                    $inputLine = ""
                    Write-Host "`r$(' ' * ([Console]::CursorLeft + 2))`r" -NoNewline
                    Write-SerialPrompt $inputLine
                    continue
                }

                if (-not [char]::IsControl($key.KeyChar)) {
                    $inputLine += $key.KeyChar
                    Write-Host -NoNewline $key.KeyChar
                }
            }

            if ($chunk.Length -eq 0) {
                Start-Sleep -Milliseconds 20
            }
        }
    } finally {
        if ($sp.IsOpen) { $sp.Close() }
        Write-Host "`nSerial port closed." -ForegroundColor Cyan
    }
}

if ($Monitor) {
    Start-SerialMonitor -PortName $Port -BaudRate $Baud -AssertDtr $Dtr.IsPresent -AssertRts $Rts.IsPresent
    return
}

$compileArgs = @(
    "compile",
    "--fqbn", $Fqbn,
    "--build-property", "compiler.cpp.extra_flags=$BuildTimeFlags $TftFlags",
    "--build-property", "compiler.c.extra_flags=$BuildTimeFlags $TftFlags"
)
if (Test-Path -LiteralPath $LibrariesDir) {
    $compileArgs += @("--libraries", $LibrariesDir)
}
if ($Clean) { $compileArgs += "--clean" }
$compileArgs += $SketchDir

Write-Host ("=== Compile DuduClock ({0}) ===" -f $Port) -ForegroundColor Green
Write-Host ("Build time: {0}" -f $BuildTime.ToString("yy/MM/dd HH:mm:ss")) -ForegroundColor DarkGray
if ($UseTftFlags) {
    Write-Host ("TFT flags: {0}" -f $TftFlags) -ForegroundColor DarkGray
} else {
    Write-Host "TFT flags: disabled (using library config)" -ForegroundColor DarkGray
}
& arduino-cli @compileArgs
if ($LASTEXITCODE -ne 0) {
    throw "Compile failed."
}

Write-Host ""
Write-Host ("=== Upload to {0} ===" -f $Port) -ForegroundColor Green
& arduino-cli upload -p $Port --fqbn $Fqbn $SketchDir
if ($LASTEXITCODE -ne 0) {
    throw "Upload failed."
}

Write-Host "Upload complete." -ForegroundColor Green

if (-not $NoMonitor) {
    Start-Sleep -Milliseconds 800
    Start-SerialMonitor -PortName $Port -BaudRate $Baud -AssertDtr $Dtr.IsPresent -AssertRts $Rts.IsPresent
}
