param([string]$Compiler='')
$ErrorActionPreference='Stop'
$root=$PSScriptRoot
$project=Join-Path $root 'RL78_F24_UART_Xmodem_Demo.mtpj'
if(!(Test-Path -LiteralPath $project -PathType Leaf)){throw 'Missing project-local CS+ project.'}
if(!$Compiler){$Compiler=Join-Path ${env:ProgramFiles(x86)} 'Renesas Electronics/CS+/CC/CubeSuite+.exe'}
if(!(Test-Path -LiteralPath $Compiler -PathType Leaf)){throw 'Installed CS+ is required; supply -Compiler when needed.'}
$logs=Join-Path $root 'tmp/build'
[IO.Directory]::CreateDirectory($logs)|Out-Null
$log=Join-Path $logs 'rebuild.log'
$process=Start-Process -FilePath $Compiler -ArgumentList ('/br "'+$project+'"') -RedirectStandardOutput $log -RedirectStandardError ($log+'.stderr') -WindowStyle Hidden -PassThru -Wait
$result=[IO.File]::ReadAllText($log)
$result -split '\r?\n'|Select-Object -Last 18|Write-Host
if($process.ExitCode -ne 0 -or $result -notmatch 'Build ended\(Error:0, Warning:\d+\)' -or $result -match 'Failed:[1-9]'){throw ('CS+ rebuild failed: '+$log)}
Write-Host 'PASS CS+ rebuild; no hardware operation was requested.'
