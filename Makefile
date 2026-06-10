## ════════════════════════════════════════════════════════════════════════════
##  Haiku Decorator SDK — Makefile
## ════════════════════════════════════════════════════════════════════════════

## ─── Nome (letto da theme.conf, sovrascrivibile da riga di comando) ───────────
NAME ?= $(shell grep '^name' theme.conf | head -1 | sed 's/.*=[ \t]*"\(.*\)".*/\1/')
ifeq ($(NAME),)
NAME = MyDecorator
endif

TYPE        = SHARED
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

## Header privati Haiku (cartella includes/ copiata da haiku_darkstyle)
SYSTEM_INCLUDE_PATHS = includes sdk/include
LOCAL_INCLUDE_PATHS  = includes sdk/include

OPTIMIZE        = FULL
LOCALES         =
DEFINES         = THEME_CONF_PATH='"$(shell pwd)/theme.conf"'
WARNINGS        =
SYMBOLS        :=
DEBUGGER       :=
COMPILER_FLAGS  = -std=c++11
LINKER_FLAGS    =
APP_VERSION    :=
DRIVER_PATH     =

## ─── makefile-engine di Haiku (deve venire PRIMA dei target personalizzati) ──
DEVEL_DIRECTORY := \
    $(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)

include $(DEVEL_DIRECTORY)/etc/makefile-engine

## ─── Target personalizzati (dopo l'include, nessun conflitto) ────────────────
INSTALL_DIR = /boot/home/config/non-packaged/add-ons/decorators

.PHONY: deploy undeploy

deploy: all
	@mkdir -p $(INSTALL_DIR)
	@OBJECTS_DIR=$$(ls -d objects.* 2>/dev/null | head -1); \
	if [ -z "$$OBJECTS_DIR" ]; then \
		echo "ERRORE: cartella objects.* non trovata."; exit 1; \
	fi; \
	cp "$$OBJECTS_DIR/$(NAME)" $(INSTALL_DIR)/$(NAME); \
	echo "✓ Installato: $(INSTALL_DIR)/$(NAME)"; \
	echo "  Seleziona '$(NAME)' in Preferenze → Aspetto."

undeploy:
	rm -f $(INSTALL_DIR)/$(NAME)
	@echo "Rimosso $(INSTALL_DIR)/$(NAME)"
