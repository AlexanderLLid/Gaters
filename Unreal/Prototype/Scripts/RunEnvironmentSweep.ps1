param(
    [int[]] $Seeds = @(0, 2, 4, 7),
    [int] $SecondsPerSeed = 12,
    [string] $EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
	[string] $ArchivePath = '',
	[int] $GalleryRadius = 4,
	[int] $GalleryCameraOffset = 26000,
	[int] $GalleryCameraHeight = 40000,
	[switch] $CaptureGallery
)

$ErrorActionPreference = 'Stop'
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$Project = Join-Path $ProjectRoot 'Prototype.uproject'
$EditorCmd = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$LogDir = Join-Path $ProjectRoot 'Saved\Logs\EnvironmentSweep'
$GalleryDir = Join-Path $ProjectRoot 'Saved\EnvironmentGallery'
$RunWriter = Join-Path $PSScriptRoot 'Write-EnvironmentRun.ps1'
$GalleryImageTest = Join-Path $PSScriptRoot 'Test-GalleryImage.ps1'

if (-not (Test-Path -LiteralPath $EditorCmd)) {
    throw "UnrealEditor-Cmd.exe not found under $EngineRoot"
}

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null

foreach ($Seed in $Seeds) {
    $Log = Join-Path $LogDir "seed-$Seed.log"
    if ($CaptureGallery) {
        New-Item -ItemType Directory -Force -Path $GalleryDir | Out-Null
        $Beauty = Join-Path $GalleryDir "seed-$Seed-beauty.png"
        $Traversal = Join-Path $GalleryDir "seed-$Seed-traversal.png"
        Remove-Item -LiteralPath $Beauty, $Traversal -Force -ErrorAction SilentlyContinue
    }
    $Arguments = @(
        $Project, '/Game/Gaters/Maps/Lvl_GateGreybox',
        '-game', '-unattended', '-nop4', '-nosplash', '-NoSound',
        '-benchmark', "-seconds=$SecondsPerSeed", "-GatersSeed=$Seed", "-abslog=$Log"
    )
    if ($CaptureGallery) {
		$Arguments += @('-RenderOffscreen', '-ResX=1280', '-ResY=720', '-windowed',
			'-GatersGallery', "-GatersGalleryRadius=$GalleryRadius",
			"-GatersGalleryCameraOffset=$GalleryCameraOffset",
			"-GatersGalleryCameraHeight=$GalleryCameraHeight")
    } else {
        $Arguments += '-nullrhi'
    }
    & $EditorCmd @Arguments

    if ($LASTEXITCODE -ne 0) {
        throw "Environment seed $Seed failed with exit code $LASTEXITCODE. See $Log"
    }

    if ($CaptureGallery) {
        foreach ($Artifact in @($Beauty, $Traversal)) {
            if (-not (Test-Path -LiteralPath $Artifact) -or (Get-Item -LiteralPath $Artifact).Length -eq 0) {
                throw "Environment seed $Seed produced no gallery artifact: $Artifact"
            }
        }
        & $GalleryImageTest -ImagePath $Beauty
    }

    if (-not [string]::IsNullOrWhiteSpace($ArchivePath)) {
        $EngineVersion = (Split-Path -Leaf $EngineRoot) -replace '^UE_', ''
        & $RunWriter -LogPath $Log -ArchivePath $ArchivePath -EngineVersion $EngineVersion -Platform 'Win64'
    }

    Select-String -LiteralPath $Log -Pattern `
        '\[GatersChunk\] (RECIPE|EVAL|SITE|TRAVERSE|PLAN|PLOTS|BASE|SCATTER|STAMP|GEN ms|PERF(?:_FAIL)?\b)|\[GatersTestSpawner\] gallery_(?:beauty|traversal|stream)' |
        ForEach-Object { $_.Line -replace '^.*\[GatersChunk\] ', '' }
}
