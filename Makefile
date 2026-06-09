## ════════════════════════════════════════════════════════════════════════════
##  Haiku Decorator SDK — Makefile
##  Uso: make  (oppure  make NAME=MyTheme  per sovrascrivere il nome)
## ════════════════════════════════════════════════════════════════════════════

## ─── Configurazione ───────────────────────────────────────────────────────────
## Il nome viene letto da theme.conf; può essere sovrascritto da riga di comando.
NAME       ?= $(shell grep '^name' theme.conf | head -1 | sed 's/.*=\s*"\(.*\)".*/\1/')
ifeq ($(NAME),)
NAME       = MyDecorator
endif

CONF_PATH   = $(shell pwd)/theme.conf

## ─── Percorsi ────────────────────────────────────────────────────────────────
SDK_SRC     = sdk/src
SDK_INC     = sdk/include
OUTPUT_DIR  = output
INSTALL_DIR = /boot/home/config/non-packaged/add-ons/decorators

## ─── Sorgenti ────────────────────────────────────────────────────────────────
SRCS = \
    $(SDK_SRC)/ConfigReader.cpp \
    $(SDK_SRC)/ThemeRenderer.cpp \
    $(SDK_SRC)/SDKDecorator.cpp

OBJS = $(SRCS:.cpp=.o)

## ─── Compiler flags ──────────────────────────────────────────────────────────
HAIKU_INCS  = \
    /boot/system/develop/headers/private/app \
    /boot/system/develop/headers/private/interface \
    /boot/system/develop/headers/private/servers/app \
    /boot/system/develop/headers/os \
    /boot/system/develop/headers/os/interface \
    /boot/system/develop/headers/os/support

INCLUDE_FLAGS = $(addprefix -I, $(HAIKU_INCS)) -I$(SDK_INC)

CXXFLAGS = -O2 -std=c++11 \
    -DTHEME_CONF_PATH='"$(CONF_PATH)"' \
    $(INCLUDE_FLAGS)

LDFLAGS  = -shared -Xlinker -soname=$(NAME).so

## ─── Librerie di sistema Haiku ───────────────────────────────────────────────
LIBS = -lbe -lroot

## ─── Target principale ───────────────────────────────────────────────────────
TARGET = $(OUTPUT_DIR)/$(NAME).so

.PHONY: all clean install uninstall help

all: $(TARGET)
	@echo ""
	@echo "╔══════════════════════════════════════════════╗"
	@echo "║  Build completato: $(TARGET)"
	@echo "║  Usa 'make install' per installare."
	@echo "╚══════════════════════════════════════════════╝"

$(TARGET): $(OBJS) | $(OUTPUT_DIR)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

## ─── Installazione ───────────────────────────────────────────────────────────
install: $(TARGET)
	@mkdir -p $(INSTALL_DIR)
	cp $(TARGET) $(INSTALL_DIR)/$(NAME).so
	@echo "Installato in $(INSTALL_DIR)/$(NAME).so"
	@echo "Seleziona '$(NAME)' nelle Preferenze Aspetto di Haiku."

uninstall:
	rm -f $(INSTALL_DIR)/$(NAME).so
	@echo "Rimosso $(INSTALL_DIR)/$(NAME).so"

clean:
	rm -f $(OBJS) $(TARGET)

help:
	@echo "Haiku Decorator SDK — Comandi disponibili:"
	@echo ""
	@echo "  make              Compila il decorator"
	@echo "  make install      Compila e installa in add-ons/decorators/"
	@echo "  make uninstall    Rimuove il decorator installato"
	@echo "  make clean        Rimuove i file compilati"
	@echo "  make NAME=Foo     Usa 'Foo' come nome invece di quello in theme.conf"
