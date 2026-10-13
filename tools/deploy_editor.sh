#!/bin/bash
# Veroeffentlicht Editor und Helfer ueber GitHub Pages: kopiert den Editor nach docs/editor.html und die Helfer-Dateien nach docs/.
# Danach committen und pushen; Pages liefert https://jppeterson-lab.github.io/DeckPad/editor.html
cd "$(dirname "$0")/.." || exit 1
cp tools/editor/deckpad-editor.html docs/editor.html && echo "docs/editor.html aktualisiert"
cp tools/helper/deckpad-helper.ps1 tools/helper/start-helper.cmd docs/ && echo "Helfer nach docs/ kopiert"
