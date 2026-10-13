# DeckPad-Helfer fuer Windows. Zwei Aufgaben (beide am DeckPad einstellbar):
#  1. Automatischer Profilwechsel: meldet dem DeckPad das Programm im Vordergrund (ACTIVE <programm.exe>).
#  2. "Tasten am PC ausfuehren": fuehrt Tasten, Medientasten, Text und Programmstarts aus, die das DeckPad schickt
#     (DO media|keys|text|run <angabe>), statt dass das Board als USB-Tastatur auftritt.
#
# Start (ohne Installation):  start-helper.cmd  doppelklicken
# Autostart:  Verknuepfung auf start-helper.cmd in den Ordner  shell:startup  legen
# Mit sichtbarem Fenster zum Testen:  powershell -ExecutionPolicy Bypass -File deckpad-helper.ps1
#
# Wichtig: Der COM-Port kann immer nur von EINEM Programm geoeffnet sein. Der Helfer gibt ihn deshalb frei,
# solange ein Fenster mit "DeckPad Editor" im Titel im Vordergrund ist, damit sich der Editor verbinden kann.
# Bleibt der Editor mit dem Geraet verbunden, den Knopf "Trennen" im Editor benutzen.
param(
  [string]$Port = "",          # z. B. COM6; leer = automatisch (USB-Geraet 303A:1001, das DeckPad)
  [int]$IntervalMs = 600,      # wie oft das Vordergrundfenster geprueft wird
  [int]$RepeatSec = 3          # unveraendertes Programm so oft erneut melden (Lebenszeichen fuer die Anzeige am Geraet)
)

Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
public static class DeckFg {
  [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll", CharSet = CharSet.Unicode)] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
}
public static class DeckKeys {
  [StructLayout(LayoutKind.Sequential)] struct MOUSEINPUT { public int dx, dy; public uint mouseData, dwFlags, time; public IntPtr dwExtraInfo; }
  [StructLayout(LayoutKind.Sequential)] struct KEYBDINPUT { public ushort wVk, wScan; public uint dwFlags, time; public IntPtr dwExtraInfo; }
  [StructLayout(LayoutKind.Explicit)] struct INPUTUNION { [FieldOffset(0)] public MOUSEINPUT mi; [FieldOffset(0)] public KEYBDINPUT ki; }
  [StructLayout(LayoutKind.Sequential)] struct INPUT { public uint type; public INPUTUNION u; }
  [DllImport("user32.dll", SetLastError = true)] static extern uint SendInput(uint n, INPUT[] p, int size);
  static void Send(ushort vk, ushort scan, uint flags) {
    INPUT[] a = new INPUT[1];
    a[0].type = 1; a[0].u.ki.wVk = vk; a[0].u.ki.wScan = scan; a[0].u.ki.dwFlags = flags;
    SendInput(1, a, Marshal.SizeOf(typeof(INPUT)));
  }
  static bool Ext(ushort vk) { return (vk >= 0x21 && vk <= 0x28) || vk == 0x2D || vk == 0x2E; }   // Bild/Pos1/Ende/Pfeile/Einfg/Entf
  public static void Chord(ushort[] vks) {
    foreach (ushort v in vks) Send(v, 0, Ext(v) ? 1u : 0u);
    Thread.Sleep(30);
    for (int i = vks.Length - 1; i >= 0; i--) Send(vks[i], 0, (Ext(vks[i]) ? 1u : 0u) | 2u);
  }
  public static void Text(string s) {                    // Unicode-Zeichen, unabhaengig vom Tastaturlayout
    foreach (char c in s) { Send(0, c, 4); Send(0, c, 6); }
  }
}
"@

# -- Tastentabellen (virtuelle Tastencodes, folgen dem Windows-Tastaturlayout) --
$MediaVk = @{ vol_up=0xAF; vol_down=0xAE; mute=0xAD; play_pause=0xB3; next=0xB0; prev=0xB1; stop=0xB2; calculator=0xB7; browser=0xAC; mail=0xB4 }
$NamedVk = @{
  ctrl=0x11; shift=0x10; alt=0x12; win=0x5B; gui=0x5B; rctrl=0xA3; rshift=0xA1; rwin=0x5C
  enter=0x0D; esc=0x1B; tab=0x09; space=0x20; backspace=0x08; delete=0x2E; insert=0x2D; home=0x24; end=0x23
  pageup=0x21; pagedown=0x22; up=0x26; down=0x28; left=0x25; right=0x27
  printscreen=0x2C; scrolllock=0x91; pause=0x13; capslock=0x14
}

function Convert-Keys([string]$spec) {
  $out = New-Object System.Collections.Generic.List[uint16]
  foreach ($part in $spec.ToLower().Split('+')) {
    if ($part -eq 'altgr') { $out.Add(0x11); $out.Add(0x12) }
    elseif ($NamedVk.ContainsKey($part)) { $out.Add([uint16]$NamedVk[$part]) }
    elseif ($part -match '^[a-z0-9]$') { $out.Add([uint16][char]$part.ToUpper()) }
    elseif ($part -match '^f(\d{1,2})$' -and [int]$Matches[1] -ge 1 -and [int]$Matches[1] -le 24) { $out.Add([uint16](0x70 + [int]$Matches[1] - 1)) }
    else { return $null }
  }
  if ($out.Count -eq 0) { return $null }
  return ,$out.ToArray()
}

function Invoke-DeckAction([string]$type, [string]$spec) {
  Write-Host ("Aktion: {0} {1}" -f $type, $spec)
  switch ($type) {
    'media' { if ($MediaVk.ContainsKey($spec)) { [DeckKeys]::Chord([uint16[]]@([uint16]$MediaVk[$spec])) } }
    'keys'  { $vks = Convert-Keys $spec; if ($vks) { [DeckKeys]::Chord($vks) } else { Write-Host "  unbekannte Taste in '$spec'" } }
    'text'  { [DeckKeys]::Text($spec) }
    'run'   { try { Start-Process -FilePath $spec } catch { Write-Host "  Start fehlgeschlagen: $($_.Exception.Message)" } }
  }
}

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
$buf = ""
$lastExe = ""
$lastSent = [datetime]::MinValue
$lastFg = [datetime]::MinValue
$editorOpen = $false
Write-Host "DeckPad-Helfer laeuft. Beenden mit Strg+C oder Fenster schliessen."

while ($true) {
  try {
    $now = Get-Date

    # Vordergrundfenster pruefen (seltener als die Leitung)
    $fg = $null
    if (($now - $lastFg).TotalMilliseconds -ge $IntervalMs) {
      $lastFg = $now
      $fg = Get-Foreground
      $editorOpen = [bool]($fg -and $fg.Title -like '*DeckPad Editor*')
      if ($editorOpen -and $sp) { $sp.Close(); $sp.Dispose(); $sp = $null; Write-Host "Editor im Vordergrund: Port freigegeben." }
    }

    if (-not $editorOpen) {
      if (-not $sp) {
        $name = Find-DeckPadPort
        if ($name) {
          $s = New-Object System.IO.Ports.SerialPort $name, 115200
          $s.DtrEnable = $true; $s.RtsEnable = $true; $s.NewLine = "`n"; $s.WriteTimeout = 1000
          $s.Open()
          $sp = $s; $lastExe = ""; $buf = ""
          Write-Host "Verbunden mit DeckPad an $name."
        }
      }
      if ($sp) {
        # Meldung des Vordergrundprogramms (bei Wechsel sofort, sonst alle $RepeatSec Sekunden als Lebenszeichen)
        if ($fg -and ($fg.Exe -ne $lastExe -or ($now - $lastSent).TotalSeconds -ge $RepeatSec)) {
          $sp.WriteLine("ACTIVE " + $fg.Exe)
          $lastExe = $fg.Exe; $lastSent = $now
        } elseif (-not $fg -and ($now - $lastSent).TotalSeconds -ge $RepeatSec -and $lastExe) {
          $sp.WriteLine("ACTIVE " + $lastExe); $lastSent = $now
        }
        # Eingehende Zeilen vom Geraet: Aktionen ausfuehren, Antworten verwerfen
        if ($sp.BytesToRead -gt 0) {
          $buf += $sp.ReadExisting()
          while (($i = $buf.IndexOf("`n")) -ge 0) {
            $line = $buf.Substring(0, $i).TrimEnd("`r"); $buf = $buf.Substring($i + 1)
            if ($line -match '^DO (\w+) (.*)$') { Invoke-DeckAction $Matches[1] $Matches[2] }
          }
          if ($buf.Length -gt 4096) { $buf = "" }
        }
      }
    }
  } catch {
    if ($sp) { try { $sp.Close(); $sp.Dispose() } catch {} ; $sp = $null }   # Kabel ab, Port belegt ...: spaeter neu versuchen
    Start-Sleep -Seconds 2
  }
  Start-Sleep -Milliseconds 40
}
