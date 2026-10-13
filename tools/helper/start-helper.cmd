@echo off
rem Startet den DeckPad-Helfer ohne sichtbares Fenster. Fuer Autostart eine Verknuepfung auf diese Datei in "shell:startup" legen.
rem Zum Testen mit sichtbarem Fenster:  powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0deckpad-helper.ps1"
start "" /min powershell -NoProfile -WindowStyle Hidden -ExecutionPolicy Bypass -File "%~dp0deckpad-helper.ps1"
