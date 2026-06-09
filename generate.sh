#!/bin/sh
# ═══════════════════════════════════════════════════════════════════════════════
#  Haiku Decorator SDK — generate.sh
#  Script principale: legge theme.conf, compila e installa il decorator.
#  Uso: ./generate.sh [--no-install] [--clean]
# ═══════════════════════════════════════════════════════════════════════════════

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CONF="$SCRIPT_DIR/theme.conf"
NO_INSTALL=0
DO_CLEAN=0

# ─── Argomenti ────────────────────────────────────────────────────────────────
for arg in "$@"; do
    case "$arg" in
        --no-install) NO_INSTALL=1 ;;
        --clean)      DO_CLEAN=1   ;;
        --help|-h)
            echo "Uso: ./generate.sh [opzioni]"
            echo ""
            echo "  --no-install   Compila ma non installa il .so"
            echo "  --clean        Rimuove i file compilati prima di procedere"
            echo "  --help         Mostra questo messaggio"
            exit 0
            ;;
    esac
done

# ─── Banner ───────────────────────────────────────────────────────────────────
echo "════════════════════════════════════════════"
echo "  Haiku Decorator SDK — Generate"
echo "════════════════════════════════════════════"

# ─── Verifica prerequisiti ────────────────────────────────────────────────────
if [ ! -f "$CONF" ]; then
    echo "ERRORE: theme.conf non trovato in $SCRIPT_DIR"
    echo "Copia theme.conf nella cartella del tema e ripeti."
    exit 1
fi

if ! command -v g++ > /dev/null 2>&1; then
    echo "ERRORE: g++ non trovato."
    echo "Installa il pacchetto 'haiku_devel' da HaikuDepot."
    exit 1
fi

# ─── Lettura nome dal conf ────────────────────────────────────────────────────
NAME=$(grep '^name' "$CONF" | head -1 | sed 's/.*=[ \t]*"\(.*\)".*/\1/')
if [ -z "$NAME" ]; then
    NAME="MyDecorator"
fi
echo "  Nome decorator : $NAME"
echo "  Configurazione : $CONF"
echo ""

# ─── Pulizia opzionale ────────────────────────────────────────────────────────
if [ "$DO_CLEAN" -eq 1 ]; then
    echo "→ Pulizia file precedenti..."
    make -C "$SCRIPT_DIR" clean > /dev/null 2>&1 || true
fi

# ─── Validazione tema ─────────────────────────────────────────────────────────
echo "→ Validazione theme.conf..."
python3 "$SCRIPT_DIR/tools/validate_theme.py" "$CONF" 2>&1
echo "  ✓ Configurazione valida"

# ─── Compilazione ─────────────────────────────────────────────────────────────
echo "→ Compilazione in corso..."
cd "$SCRIPT_DIR"
make NAME="$NAME" 2>&1

# ─── Installazione ────────────────────────────────────────────────────────────
if [ "$NO_INSTALL" -eq 0 ]; then
    echo "→ Installazione..."
    make NAME="$NAME" install 2>&1
    echo ""
    echo "════════════════════════════════════════════"
    echo "  ✓ Installato: $NAME"
    echo ""
    echo "  Prossimi passi:"
    echo "  1. Apri Preferenze → Aspetto"
    echo "  2. Seleziona '$NAME' nel menu Decorator"
    echo "  (non è necessario riavviare)"
    echo "════════════════════════════════════════════"
else
    echo ""
    echo "✓ Compilato: output/$NAME.so"
    echo "  (non installato — usa 'make install' per installare manualmente)"
fi
