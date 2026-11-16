$ErrorActionPreference='Stop'
$root=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..')).TrimEnd('\')
$build=[IO.Path]::GetFullPath((Join-Path $root 'DefaultBuild'))
if([IO.Path]::GetDirectoryName($build) -ine $root){throw 'Invalid intermediate boundary.'}
for($parent=$build;$parent;$parent=[IO.Path]::GetDirectoryName($parent)) {
    if((Test-Path -LiteralPath $parent) -and ((Get-Item -LiteralPath $parent -Force).Attributes -band [IO.FileAttributes]::ReparsePoint)){throw 'Linked intermediate ancestor refused.'}
}
if(!(Test-Path -LiteralPath $build)){Write-Host 'No build intermediates.';exit 0}
$queue=New-Object 'Collections.Generic.Stack[string]';$queue.Push($build)
$files=New-Object 'Collections.Generic.List[object]'
while($queue.Count) {
    $item=Get-Item -LiteralPath $queue.Pop() -Force
    if($item.Attributes -band [IO.FileAttributes]::ReparsePoint){throw 'Linked intermediate child refused.'}
    if($item.PSIsContainer){foreach($child in Get-ChildItem -LiteralPath $item.FullName -Force){$queue.Push($child.FullName)}}
    else {$files.Add($item)}
}
$count=0
foreach($file in $files) {
    if($file.Extension.ToLowerInvariant() -in @('.obj','.ud','.udm','.clnk')){Remove-Item -LiteralPath $file.FullName -Force -ErrorAction Stop;$count++}
}
Write-Host ('Removed '+$count+' allowlisted compiler intermediates; firmware/maps/libraries/INI/logs/unknown files preserved.')
