# Script to prepare sample data from Kaggle dataset
# Converts .us.txt files to CSV format compatible with FluxBack

$sourceDir = "C:\Users\golla\Downloads\archive (1)\Data\Stocks"
$destDir = "demo"

# Find a sample file (try common tickers)
$sampleFiles = @("aapl.us.txt", "msft.us.txt", "googl.us.txt", "amzn.us.txt", "tsla.us.txt")

$foundFile = $null
foreach ($file in $sampleFiles) {
    $path = Join-Path $sourceDir $file
    if (Test-Path $path) {
        $foundFile = $path
        Write-Host "Found sample file: $file"
        break
    }
}

if (-not $foundFile) {
    # Try to find any .us.txt or .txt file
    $anyFile = Get-ChildItem $sourceDir -Filter "*.us.txt" | Select-Object -First 1
    if (-not $anyFile) {
        $anyFile = Get-ChildItem $sourceDir -Filter "*.txt" | Select-Object -First 1
    }
    if ($anyFile) {
        $foundFile = $anyFile.FullName
        Write-Host "Using file: $($anyFile.Name)"
    }
}

if (-not $foundFile) {
    Write-Host "Error: No data files found in $sourceDir"
    exit 1
}

# Read lines and convert format
Write-Host "Converting data format..."
$lines = Get-Content $foundFile -TotalCount 200

# Write header
$csvLines = @("timestamp,open,high,low,close,volume")

# Process data lines (skip header, take up to 199 data rows)
$dataLines = $lines[1..199]
foreach ($line in $dataLines) {
    if ($line -match "^(\d{4}-\d{2}-\d{2}),([\d.]+),([\d.]+),([\d.]+),([\d.]+),(\d+),") {
        $date = $matches[1]
        $open = $matches[2]
        $high = $matches[3]
        $low = $matches[4]
        $close = $matches[5]
        $volume = $matches[6]
        
        # Convert date to timestamp format (assume 9:30 AM market open for daily data)
        $timestamp = "$date" + "T09:30:00"
        $csvLines += "$timestamp,$open,$high,$low,$close,$volume"
    }
}

# Write output
$outputPath = Join-Path $destDir "aapl_sample.csv"
$csvLines | Set-Content $outputPath
Write-Host "Created $outputPath with $($csvLines.Count - 1) data rows"
