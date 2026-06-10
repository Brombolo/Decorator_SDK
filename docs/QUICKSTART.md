# Guida Rapida — 5 minuti per il tuo primo decorator

Questa guida ti porta dal download all'installazione in meno di 5 minuti,
senza scrivere una sola riga di codice.

---

## Passo 1 — Prepara l'ambiente (una tantum)

**Installa haiku_devel:**

```sh
pkgman install haiku_devel
```

**Copia gli header privati:**

Gli header `SATDecorator.h` e gli altri non fanno parte di `haiku_devel`.
Li trovi già nel tuo repository `haiku_darkstyle`, nella cartella
`FlatDecorator/includes/`. Copia quella cartella qui:

```sh
cp -r /percorso/haiku_darkstyle/FlatDecorator/includes/* \
      sdk/private-headers/
```

Serve farlo solo la prima volta.

---

## Passo 2 — Scegli un tema di partenza

Nella cartella `themes/` trovi tre temi pronti:

| Tema           | Aspetto                                   |
|----------------|-------------------------------------------|
| `DarkFlat`     | 🌑 Scuro, bordi sottili, minimalista      |
| `ClassicBeOS`  | 🟡 Tab giallo, look BeOS originale        |
| `ArcLight`     | ☀️ Chiaro, moderno, effetto glass          |

Copia il tema che preferisci come punto di partenza:

```sh
cp themes/DarkFlat.conf theme.conf
```

---

## Passo 3 — Personalizza (opzionale)

Apri `theme.conf` con **StyledEdit** o qualsiasi editor.

Cambia i valori che vuoi. Ecco le cose più facili da modificare:

**Cambiare il colore del tab:**
```toml
[tab.active]
color_start = "#5294e2"   # ← cambia questo colore
color_end   = "#3a7bd5"   # ← e questo (o mettilo uguale a color_start)
```

**Cambiare l'altezza del tab:**
```toml
[tab]
height = 26   # ← prova valori tra 18 e 32
```

**Cambiare la forma del tab:**
```toml
[tab]
shape = "rounded"     # flat | rounded | slanted
corner_radius = 6     # aggiunge se shape=rounded
```

**Cambiare i colori dei pulsanti:**
```toml
[buttons.close]
color_normal = "#e05050"   # rosso personalizzato

[buttons.zoom]
color_normal = "#50b050"   # verde personalizzato
```

**Attivare il gradiente:**
```toml
[tab.effect]
type         = "gradient"
gradient_dir = "vertical"
```

> Per l'elenco completo di tutte le impostazioni, vedi **docs/THEME_REFERENCE.md**

---

## Passo 4 — Compila e installa

Nel Terminale, dalla cartella del SDK:

```sh
./generate.sh
```

Se ci sono errori in `theme.conf`, lo script ti indica esattamente la riga
e cosa correggere. Correggi e riesegui.

Se tutto va bene vedrai:
```
╔══════════════════════════════════════════════╗
║  ✓ Installato: NomeDelTuoTema
║
║  Prossimi passi:
║  1. Apri Preferenze → Aspetto
║  2. Seleziona 'NomeDelTuoTema' nel menu Decorator
║  (non è necessario riavviare)
╚══════════════════════════════════════════════╝
```

---

## Passo 5 — Attiva il decorator

1. Apri **Preferenze** → **Aspetto**
2. Nel menu **Decorator** seleziona il nome del tuo tema
3. Clicca **Applica**

Le finestre si aggiornano immediatamente.

---

## Iterazione rapida

Il ciclo di lavoro tipico è:

```
modifica theme.conf  →  ./generate.sh  →  guarda il risultato
```

Non devi fare nulla nelle Preferenze dopo la prima installazione: `generate.sh`
sovrascrive il `.so` esistente e il decorator si aggiorna al prossimo ridisegno
delle finestre (basta muovere una finestra).

---

## Usare una texture sul tab

1. Metti un file PNG nella cartella `textures/`
   (consigliato: PNG in scala di grigi, ≤ 64×64 px per il tiling)

2. In `theme.conf` modifica:
   ```toml
   [tab.effect]
   type = "texture"

   [tab.texture]
   file       = "textures/tuo_file.png"
   tile       = true
   blend_mode = "multiply"
   opacity    = 0.30
   ```

3. Esegui `./generate.sh`

---

## Qualcosa non funziona?

**Lo script dice "errore di configurazione"**
→ Leggi il messaggio: ti dice la riga esatta e cosa correggere.

**Lo script dice "g++ non trovato"**
→ Esegui `pkgman install haiku_devel` e riprova.

**Il decorator è installato ma non appare nelle Preferenze**
→ Controlla con:
```sh
ls /boot/home/config/non-packaged/add-ons/decorators/
```

**Vuoi tornare al decorator originale**
→ Preferenze → Aspetto → Decorator → seleziona "Default" → Applica.
