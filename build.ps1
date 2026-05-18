$compiler = "g++"
$files = Get-ChildItem -Path src/*.cpp -Recurse | ForEach-Object { $_.FullName }
$includeDir = "include"
$output = "cvm.exe"

& $compiler -std=c++17 -I $includeDir $files -o $output

if ($LASTEXITCODE -eq 0) {
    Write-Host "Build successful: $output"
} else {
    Write-Host "Build failed."
    exit $LASTEXITCODE
}
