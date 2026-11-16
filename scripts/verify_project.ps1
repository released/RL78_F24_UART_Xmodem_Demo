$ErrorActionPreference='Stop'
$root=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..')).TrimEnd('\')
$contract=Get-Content -LiteralPath (Join-Path $PSScriptRoot 'runtime_contract.json') -Raw -Encoding UTF8|ConvertFrom-Json
if($contract.project -cne 'RL78_F24_UART_Xmodem_Demo'){throw 'Unexpected project contract.'}
$sha=[Security.Cryptography.SHA256]::Create()
try {
    foreach($entry in $contract.files) {
        if([IO.Path]::IsPathRooted($entry.path) -or $entry.path -match '(^|[/\\])\.\.([/\\]|$)'){throw 'Non-local contract path.'}
        $path=[IO.Path]::GetFullPath((Join-Path $root $entry.path))
        if(!$path.StartsWith($root+'\',[StringComparison]::OrdinalIgnoreCase)){throw 'Contract path escapes project.'}
        for($parent=$path;$parent;$parent=[IO.Path]::GetDirectoryName($parent)) {
            if((Test-Path -LiteralPath $parent) -and ((Get-Item -LiteralPath $parent -Force).Attributes -band [IO.FileAttributes]::ReparsePoint)){throw 'Linked contract path refused.'}
        }
        $text=[IO.File]::ReadAllText($path,(New-Object Text.UTF8Encoding($false,$true))).Replace("`r`n","`n")
        $hash=[BitConverter]::ToString($sha.ComputeHash([Text.Encoding]::UTF8.GetBytes($text))).Replace('-','').ToLowerInvariant()
        if($hash -cne $entry.sha256){throw ('Runtime/configuration differs from the reviewed rename baseline: '+$entry.path)}
    }
} finally {$sha.Dispose()}
Write-Host 'PASS reviewed runtime, peripheral and project configuration contract.'
