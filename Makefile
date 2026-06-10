## ════════════════════════════════════════════════════════════════════════════
##  Haiku Decorator SDK — Makefile
##  Usa il makefile-engine nativo di Haiku (stesso approccio di DecoratorSDK
##  by CodeforEvolution: github.com/CodeforEvolution/DecoratorSDK)
## ════════════════════════════════════════════════════════════════════════════

## ─── Nome (letto da theme.conf, sovrascrivibile da riga di comando) ───────────
NAME ?= $(shell grep '^name' theme.conf | head -1 | sed 's/.*=[ \t]*"\(.*\)".*/\1/')
ifeq ($(NAME),)
NAME = MyDecorator
endif

TYPE = SHARED

APP_MIME_SIG =

SRCS = \
    sdk/src/ConfigReader.cpp \
    sdk/src/ThemeRenderer.cpp \
    sdk/src/TabPainter.cpp \
    sdk/src/SDKDecorator.cpp

RDEFS = resources.rdef

RSRCS =

LIBS = $(STDCPPLIBS) be translation

LIBPATHS =

## Header privati Haiku necessari per i decorator.
## Vengono cercati prima in sdk/private-headers/ (copia locale distribuita
## con l'SDK), poi nei path standard di haiku_devel come fallback.
SYSTEM_INCLUDE_PATHS = sdk/private-headers sdk/include

LOCAL_INCLUDE_PATHS = sdk/private-headers sdk/include

OPTIMIZE = FULL

LOCALES =

DEFINES = THEME_CONF_PATH='"$(shell pwd)/theme.conf"'

WARNINGS =

SYMBOLS :=

DEBUGGER :=

COMPILER_FLAGS = -std=c++11

LINKER_FLAGS =

APP_VERSION :=

DRIVER_PATH =

## ─── Installazione ───────────────────────────────────────────────────────────
INSTALL_DIR = /boot/home/config/non-packaged/add-ons/decorators

## Il makefile-engine di Haiku produce i .so in objects.<arch>-*/
## Questo target li copia nella cartella output/ e li installa.
OBJECTS_DIR = $(shell ls -d objects.* 2>/dev/null | head -1)

.PHONY: install uninstall help

install: all
	@mkdir -p $(INSTALL_DIR)
	@OBJECTS_DIR=$$(ls -d objects.* 2>/dev/null | head -1); \
	if [ -z "$$OBJECTS_DIR" ]; then \
		echo "ERRORE: nessuna cartella objects.* trovata. Esegui 'make' prima."; exit 1; \
	fi; \
	cp "$$OBJECTS_DIR/$(NAME)" $(INSTALL_DIR)/$(NAME); \
	echo "✓ Installato: $(INSTALL_DIR)/$(NAME)"
	@echo "Seleziona '$(NAME)' in Preferenze → Aspetto."

uninstall:
	rm -f $(INSTALL_DIR)/$(NAME)
	@echo "Rimosso $(INSTALL_DIR)/$(NAME)"

help:
	@echo "Haiku Decorator SDK — Comandi:"
	@echo "  make              Compila il decorator"
	@echo "  make install      Compila e installa"
	@echo "  make uninstall    Rimuove il decorator installato"
	@echo "  make clean        Rimuove i file compilati"

## ─── makefile-engine di Haiku ────────────────────────────────────────────────
DEVEL_DIRECTORY := \
    $(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)

include $(DEVEL_DIRECTORY)/etc/makefile-engine
