param([switch]$DryRun,[switch]$CheckLocal,[switch]$Yes,[string]$ScopeFile='',
      [string]$Message='Maintain standalone UART Xmodem demo metadata')
$ErrorActionPreference='Stop'
try {
    if(($DryRun -and $CheckLocal) -or ($Yes -and ($DryRun -or $CheckLocal))){throw 'Select preview, local check or update separately.'}
    . (Join-Path $PSScriptRoot 'github_publish_core.ps1')
    $root=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..')).TrimEnd('\')
    & (Join-Path $PSScriptRoot 'verify_project.ps1')
    $config=Get-Content -LiteralPath (Join-Path $PSScriptRoot 'github_repository.json') -Raw -Encoding UTF8|ConvertFrom-Json
    if($config.repository -cne 'released/RL78_F24_UART_Xmodem_Demo' -or $config.repository_id -ne 920350497 -or
        $config.visibility -cne 'PUBLIC' -or $config.branch -cne 'main'){throw 'Pinned repository contract differs.'}
    $scope=@($config.metadata_paths)
    if($ScopeFile) {
        $path=Assert-PublishPath $root $ScopeFile
        $scope=Get-Content -LiteralPath $path -Raw -Encoding UTF8|ConvertFrom-Json
        if(!$scope.Count){throw 'Empty task scope.'}
        foreach($relative in $scope){if($relative -isnot [string] -or $relative -notin $config.metadata_paths){throw 'Task path is outside the reviewed metadata scope.'}}
    }
    if(!$CheckLocal) {
        $gh=Get-Command gh -CommandType Application -ErrorAction SilentlyContinue
        if(!$gh){$candidate=Join-Path $env:ProgramFiles 'GitHub CLI/gh.exe';if(!(Test-Path -LiteralPath $candidate)){throw 'GitHub CLI is required.'};$env:PATH=(Split-Path -Parent $candidate)+';'+$env:PATH}
        $live=(Invoke-PublishProcess gh @('api',('repos/'+$config.repository))).Text|ConvertFrom-Json
        if($live.id -ne $config.repository_id -or $live.full_name -cne $config.repository -or $live.private -or
            $live.archived -or $live.default_branch -cne $config.branch){throw 'Live immutable repository identity/visibility/branch differs.'}
    }
    $target=@{Root=$root;Repository=$config.repository;Remote=('https://github.com/'+$config.repository+'.git');Branch=$config.branch;
        Title=$config.title;IdentityFiles=@('README.md','custom_func.c');DeliveryFiles=@();Visibility=$config.visibility;HostOnly=$false;
        PreviousTitle=$config.previous_title;PreviousHead=$config.previous_head;ScopePaths=@($scope)}
    $plan=New-PublishPlan $target -LocalOnly:$CheckLocal
    Show-PublishPlan $plan
    if($DryRun -or $CheckLocal){exit 0}
    if(!$Yes -and !(Test-PublishConfirmation (Read-Host 'Type PUSH to commit and push the reviewed metadata scope'))){Write-Output 'CANCELED: no Git changes.';exit 0}
    Invoke-PublishPlan $plan $Message
    exit 0
} catch {Write-Error $_.Exception.Message -ErrorAction Continue;exit 1}
