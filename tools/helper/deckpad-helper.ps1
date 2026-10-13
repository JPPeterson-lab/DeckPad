# DeckPad-Helfer fuer Windows: meldet dem DeckPad das Programm im Vordergrund (fuer den automatischen Profilwechsel).
# Das DeckPad wechselt nur, wenn der Schalter "Automatischer Profilwechsel" in seinen Einstellungen an ist.
#
# Start (ohne Installation):  start-helper.cmd  doppelklicken
# Autostart:  Verknuepfung auf start-helper.cmd in den Ordner  shell:startup  legen
#
# Wichtig: Der COM-Port kann immer nur von EINEM Programm geoeffnet sein. Der Helfer gibt ihn deshalb frei,
# solange ein Fenster mit "DeckPad Editor" im Titel im Vordergrund ist, damit sich der Editor verbinden kann.
# Bleibt der Editor mit dem Gerat verbunden, den Knopf "Trennen" im Editor benutzen.
param(
  [string]$Port = "",          # z. B. COM6; leer = automatisch (USB-Geraet 303A:1001, das DeckPad)
  [int]$IntervalMs = 600,      # wie oft das Vordergrundfenster geprueft wird
  [int]$RepeatSec = 3          # unveraendertes Programm so oft erneut melden (Lebenszeichen fuer die Anzeige am Geraet)
)

Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;
using System.Text;
public static class DeckFg {
  [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll", CharSet = CharSet.Unicode)] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
}
"@

function Get-Foreground {
  $h = [DeckFg]::GetForegroundWindow()
  if ($h -eq [IntPtr]::Zero) { return $null }
  $procId = 0
  [void][DeckFg]::GetWindowThreadProcessId($h, [ref]$procId)
  $sb = New-Object System.Text.StringBuilder 256
  [void][DeckFg]::GetWindowText($h, $sb, 256)
  try { $name = (Get-Process -Id $procId -ErrorAction Stop).ProcessName } catch { return $null }
  [pscustomobject]@{ Exe = "$name.exe"; Title = $sb.ToString() }
}

function Find-DeckPadPort {
  if ($Port) { return $Port }
  $dev = Get-CimInstance Win32_PnPEntity -ErrorAction SilentlyContinue |
         Where-Object { $_.PNPDeviceID -like 'USB\VID_303A&PID_1001*' -and $_.Name -match '\((COM\d+)\)' } |
         Select-Object -First 1
  if ($dev) { [void]($dev.Name -match '\((COM\d+)\)'); return $Matches[1] }
  return $null
}

$sp = $null
$lastExe = ""
$lastSent = [datetime]::MinValue
Write-Host "DeckPad-Helfer laeuft. Beenden mit Strg+C oder Fenster schliessen."

while ($true) {
  try {
    $fg = Get-Foreground
    $editorOpen = $fg -and $fg.Title -like '*DeckPad Editor*'

    if ($editorOpen) {
      if ($sp) { $sp.Close(); $sp.Dispose(); $sp = $null; Write-Host "Editor im Vordergrund: Port freigegeben." }
    } else {
      if (-not $sp) {
        $name = Find-DeckPadPort
        if ($name) {
          $s = New-Object System.IO.Ports.SerialPort $name, 115200
          $s.DtrEnable = $true; $s.RtsEnable = $true; $s.NewLine = "`n"; $s.ReadTimeout = 200; $s.WriteTimeout = 1000
          $s.Open()
          $sp = $s; $lastExe = ""
          Write-Host "Verbunden mit DeckPad an $name."
        }
      }
      if ($sp -and $fg) {
        $now = Get-Date
        if ($fg.Exe -ne $lastExe -or ($now - $lastSent).TotalSeconds -ge $RepeatSec) {
          $sp.WriteLine("ACTIVE " + $fg.Exe)
          $lastExe = $fg.Exe; $lastSent = $now
        }
        [void]$sp.ReadExisting()      # Antworten des Geraets verwerfen
      }
    }
  } catch {
    if ($sp) { try { $sp.Close(); $sp.Dispose() } catch {} ; $sp = $null }   # Kabel ab, Port belegt ...: spaeter neu versuchen
    Start-Sleep -Seconds 2
  }
  Start-Sleep -Milliseconds $IntervalMs
}
