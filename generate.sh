#!/bin/sh
# ═══════════════════════════════════════════════════════════════════════════════
#  Haiku Decorator SDK — generate.sh
# ═══════════════════════════════════════════════════════════════════════════════

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CONF="$SCRIPT_DIR/theme.conf"
NO_INSTALL=0
DO_CLEAN=0

for arg in "$@"; do
    case "$arg" in
        --no-install) NO_INSTALL=1 ;;
        --clean)      DO_CLEAN=1   ;;
        --help|-h)
            echo "Uso: ./generate.sh [--no-install] [--clean]"
            exit 0
            ;;
    esac
done

echo "════════════════════════════════════════════"
echo "  Haiku Decorator SDK — Generate"
echo "════════════════════════════════════════════"

# ─── Verifica prerequisiti ────────────────────────────────────────────────────
if [ ! -f "$CONF" ]; then
    echo "ERRORE: theme.conf non trovato in $SCRIPT_DIR"
    exit 1
fi

if ! command -v findpaths > /dev/null 2>&1; then
    echo "ERRORE: questo script deve essere eseguito su Haiku OS."
    exit 1
fi

MAKEFILE_ENGINE=$(findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY 2>/dev/null)
if [ -z "$MAKEFILE_ENGINE" ]; then
    echo "ERRORE: makefile-engine non trovato."
    echo "Installa haiku_devel: pkgman install haiku_devel"
    exit 1
fi

# ─── Verifica header privati ──────────────────────────────────────────────────
if [ ! -f "$SCRIPT_DIR/includes/SATDecorator.h" ]; then
    echo ""
    echo "⚠  Header privati non trovati in includes/"
    echo ""
    echo "   Copia la cartella includes/ dal tuo repository haiku_darkstyle:"
    echo "   cp -r /percorso/haiku_darkstyle/FlatDecorator/includes/* includes/"
    echo ""
    exit 1
fi

# ─── Nome dal conf ────────────────────────────────────────────────────────────
NAME=$(grep '^name' "$CONF" | head -1 | sed 's/.*=[ \t]*"\(.*\)".*/\1/')
if [ -z "$NAME" ]; then NAME="MyDecorator"; fi

echo "  Nome decorator : $NAME"
echo "  Configurazione : $CONF"
echo ""

# ─── Pulizia opzionale ────────────────────────────────────────────────────────
if [ "$DO_CLEAN" -eq 1 ]; then
    echo "→ Pulizia..."
    make -C "$SCRIPT_DIR" clean > /dev/null 2>&1 || true
fi

# ─── Validazione ─────────────────────────────────────────────────────────────
echo "→ Validazione theme.conf..."
python3 "$SCRIPT_DIR/tools/validate_theme.py" "$CONF"
echo "  ✓ Configurazione valida"

# ─── Compilazione ─────────────────────────────────────────────────────────────
echo "→ Compilazione in corso..."
cd "$SCRIPT_DIR"
make NAME="$NAME" 2>&1

# ─── Installazione ────────────────────────────────────────────────────────────
if [ "$NO_INSTALL" -eq 0 ]; then
    echo "→ Installazione..."
    make NAME="$NAME" deploy 2>&1
    echo ""
    echo "════════════════════════════════════════════"
    echo "  ✓ Installato: $NAME"
    echo ""
    echo "  1. Apri Preferenze → Aspetto"
    echo "  2. Seleziona '$NAME' nel menu Decorator"
    echo "════════════════════════════════════════════"
else
    echo ""
    echo "✓ Compilato (non installato)"
fi
