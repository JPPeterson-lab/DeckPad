#!/bin/bash
# Veröffentlicht den Editor über GitHub Pages: kopiert tools/editor/deckpad-editor.html nach docs/index.html.
# Danach committen und pushen; Pages liefert https://jppeterson-lab.github.io/DeckPad/
cd "$(dirname "$0")/.." || exit 1
cp tools/editor/deckpad-editor.html docs/index.html && echo "docs/index.html aktualisiert"
