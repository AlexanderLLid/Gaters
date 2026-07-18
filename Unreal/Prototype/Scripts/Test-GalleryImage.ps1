param(
    [Parameter(Mandatory = $true)]
    [string] $ImagePath,
    [double] $MaxDarkFraction = 0.01,
    [double] $MinMeanLuminance = 65
)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing

$Bitmap = [Drawing.Bitmap]::new((Resolve-Path -LiteralPath $ImagePath).Path)
try {
    $Dark = 0
    $Pixels = 0
    $Luminance = 0.0
    $Brightness = 0.0
    for ($Y = [int]($Bitmap.Height * 0.55); $Y -lt $Bitmap.Height; $Y += 2) {
        for ($X = 0; $X -lt $Bitmap.Width; $X += 2) {
            $Color = $Bitmap.GetPixel($X, $Y)
            $Pixels++
            $Luminance += 0.2126 * $Color.R + 0.7152 * $Color.G + 0.0722 * $Color.B
            $Brightness += ([Math]::Max($Color.R, [Math]::Max($Color.G, $Color.B)) +
                [Math]::Min($Color.R, [Math]::Min($Color.G, $Color.B))) * 0.5
            if ($Color.R -lt 16 -and $Color.G -lt 16 -and $Color.B -lt 16) {
                $Dark++
            }
        }
    }
} finally {
    $Bitmap.Dispose()
}

$DarkFraction = $Dark / $Pixels
$MeanLuminance = $Luminance / $Pixels
$MeanBrightness = $Brightness / $Pixels
$MeanSignal = [Math]::Max($MeanLuminance, $MeanBrightness)
if ($DarkFraction -gt $MaxDarkFraction) {
    throw "Gallery terrain is excessively dark: fraction=$DarkFraction max=$MaxDarkFraction"
}
if ($MeanSignal -lt $MinMeanLuminance) {
    throw "Gallery terrain is underexposed: signal=$MeanSignal luminance=$MeanLuminance brightness=$MeanBrightness min=$MinMeanLuminance"
}

"PASS gallery_dark_fraction=$DarkFraction mean_signal=$MeanSignal mean_luminance=$MeanLuminance mean_brightness=$MeanBrightness"
