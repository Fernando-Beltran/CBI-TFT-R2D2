param (
    [string]$FilePath
)

# Si no se pasa el archivo, pedirlo interactivamente
if (-not $FilePath) {
    $FilePath = Read-Host "Arrastra el archivo .gif aqui y presiona Intro"
}

# Limpiar comillas que Windows aÃ±ade al arrastrar
$FilePath = $FilePath.Trim('"')

if (-not (Test-Path $FilePath) -or (Get-Item $FilePath).Extension.ToLower() -ne ".gif") {
    Write-Host "[ERROR] El archivo proporcionado no es un GIF valido." -ForegroundColor Red
    Read-Host "Presiona Intro para salir..."
    exit
}

$outputH = [System.IO.Path]::ChangeExtension($FilePath, ".h")
$variableName = [System.IO.Path]::GetFileNameWithoutExtension($FilePath) -replace '[^a-zA-Z0-9_]', '_'

Write-Host "===================================================" -ForegroundColor Cyan
Write-Host "   PROCESANDO ARCHIVO: $([System.IO.Path]::GetFileName($FilePath))" -ForegroundColor Cyan
Write-Host "===================================================" -ForegroundColor Cyan
Write-Host "Codificando GIF a matriz de bytes (.h)..."

try {
    # Leer todos los bytes del GIF de golpe
    $bytes = [System.IO.File]::ReadAllBytes($FilePath)
    $sb = New-Object System.Text.StringBuilder
    
    # Formatear los bytes en filas de 16 para que quede limpio
    for ($i = 0; $i -lt $bytes.Length; $i++) {
        if ($i -eq 0) {
            [void]$sb.Append("`t")
        } elseif ($i % 16 -eq 0) {
            [void]$sb.AppendLine(",")
            [void]$sb.Append("`t")
        } else {
            [void]$sb.Append(",")
        }
        [void]$sb.Append("0x{0:x2}" -f $bytes[$i])
    }
    
    # Estructura del archivo .h compatible con Arduino / ESP32
    $content = "/* Archivo generado automaticamente a partir de GIF */",
               "#ifndef PROGMEM", 
               "#define PROGMEM", 
               "#endif",
               "const uint8_t ${variableName}[] PROGMEM = {",
               $sb.ToString(), 
               "};"
               
    [System.IO.File]::WriteAllLines($outputH, $content)
    
    Write-Host ""
    Write-Host "===================================================" -ForegroundColor Green
    Write-Host "   Â¡ARCHIVO .H GENERADO CON EXITO!" -ForegroundColor Green
    Write-Host "===================================================" -ForegroundColor Green
    Write-Host "Guardado en: $outputH" -ForegroundColor Yellow

} catch {
    Write-Host "[ERROR] Ocurrio un fallo al procesar el archivo: $_" -ForegroundColor Red
}

Write-Host ""
Read-Host "Presiona Intro para finalizar..."
