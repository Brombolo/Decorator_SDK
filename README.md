# Haiku Decorator SDK

**Crea un decorator personalizzato per Haiku OS modificando un solo file di testo.**

Nessuna conoscenza di C++ richiesta per l'uso base.

---

## Requisiti

- Haiku OS (nightly o versione stabile recente)
- Package `haiku_devel` installato (da HaikuDepot o `pkgman install haiku_devel`)
- Python 3 (incluso in Haiku di default)

---

## Iniziare in 4 passi

### 1. Copia gli header privati di Haiku (una tantum)

Gli header `SATDecorator.h`, `Decorator.h` ecc. non fanno parte di
`haiku_devel`. Li trovi già nel tuo repository `haiku_darkstyle`,
nella cartella `FlatDecorator/includes/`.

```sh
cp -r /percorso/haiku_darkstyle/FlatDecorator/includes/* \
      sdk/private-headers/
```

Serve farlo **una sola volta**. Vedi `sdk/private-headers/README.txt` per l'elenco esatto dei file necessari.

### 2. Copia il tema di partenza

```sh
cp themes/DarkFlat.conf theme.conf
```

Puoi scegliere tra i temi inclusi nella cartella `themes/`:

| File              | Descrizione                              |
|-------------------|------------------------------------------|
| `DarkFlat.conf`   | Scuro, minimale, ispirato a FlatDecorator|
| `ClassicBeOS.conf`| Tab giallo, stile R4/R5 originale        |
| `ArcLight.conf`   | Chiaro, moderno, tab con effetto glass   |

### 3. Modifica `theme.conf`

Apri `theme.conf` con qualsiasi editor di testo e cambia i valori che vuoi.  
Le sezioni principali sono:

- **`[tab]`** — altezza, forma, colori e effetti della barra del titolo
- **`[border]`** — spessore e stile dei bordi della finestra
- **`[buttons]`** — forma, colori e icone dei pulsanti Close/Zoom
- **`[title]`** — allineamento e ombra del testo del titolo
- **`[resize_corner]`** — stile dell'angolo di ridimensionamento

### 4. Genera e installa

```sh
./generate.sh
```

Lo script:
1. Valida `theme.conf` e segnala eventuali errori con indicazione della riga
2. Compila il decorator come libreria condivisa (`.so`)
3. Installa in `/boot/home/config/non-packaged/add-ons/decorators/`

Poi apri **Preferenze → Aspetto**, scegli il tuo decorator dal menu e premi *Applica*. Non è necessario riavviare.

---

## Guida alle impostazioni

### [tab] — Barra del titolo

```toml
[tab]
height        = 22        # Altezza in pixel (14–48)
font_size     = 12        # Dimensione testo (8–18)
font_bold     = false     # true = grassetto
shape         = "flat"    # flat | rounded | slanted
corner_radius = 0         # Arrotondamento angoli (0–12, solo se shape=rounded)
slant_angle   = 15        # Inclinazione laterale (5–45°, solo se shape=slanted)
```

**Forme del tab:**

| Valore     | Aspetto                                      |
|------------|----------------------------------------------|
| `flat`     | Rettangolo con angoli netti (default)        |
| `rounded`  | Angoli arrotondati (usa `corner_radius`)     |
| `slanted`  | Trapezio inclinato, stile BeOS R4            |

---

### [tab.active] e [tab.inactive]

I colori del tab quando la finestra è in focus o non lo è.

```toml
[tab.active]
color_start   = "#3a5f8a"  # Colore principale
color_end     = "#2b4a6f"  # Fine gradiente (= color_start → colore piatto)
text_color    = "#ffffff"
border_color  = "#1a3a5a"
```

> **Suggerimento:** per un colore piatto senza gradiente, imposta `color_start` e `color_end` allo stesso valore e usa `effect.type = "none"`.

---

### [tab.effect] — Effetto visivo

```toml
[tab.effect]
type = "gradient"    # none | gradient | glass | texture | tint_from_system
```

| Tipo              | Descrizione                                              |
|-------------------|----------------------------------------------------------|
| `none`            | Colore piatto                                            |
| `gradient`        | Sfumatura da `color_start` a `color_end`                 |
| `glass`           | Overlay semitrasparente chiaro nella metà superiore      |
| `texture`         | Immagine PNG in `[tab.texture]` (tiled o stretched)      |
| `tint_from_system`| Eredita il colore di accento dalle preferenze di sistema |

Per `gradient`, puoi scegliere la direzione:
```toml
gradient_dir = "vertical"    # vertical | horizontal | diagonal
```

---

### [tab.texture] — Texture personalizzata

Valido solo quando `effect.type = "texture"`.

```toml
[tab.texture]
file       = "textures/mia_texture.png"  # PNG relativo a theme.conf
tile       = true       # true = ripetuta a tasselli, false = stirata
blend_mode = "multiply" # normal | multiply | overlay | screen
opacity    = 0.4        # Intensità della texture (0.0–1.0)
```

**Modalità di blending:**

| Modalità   | Effetto                                         |
|------------|-------------------------------------------------|
| `normal`   | La texture sovrascrive il colore di base        |
| `multiply` | Scurisce il colore di base con la texture       |
| `overlay`  | Contrasto: scurisce le ombre, schiarisce le luci|
| `screen`   | Schiarisce il colore di base con la texture     |

---

### [border] — Bordi della finestra

```toml
[border]
width = 5           # Spessore in pixel (1–12)
style = "beveled"   # flat | beveled | inset | shadow
```

| Stile      | Descrizione                                   |
|------------|-----------------------------------------------|
| `flat`     | Bordo monocolore sottile                      |
| `beveled`  | Effetto rilievo a più livelli (default Haiku) |
| `inset`    | Effetto incassato (highlight/shadow invertiti)|
| `shadow`   | Singolo bordo con ombra esterna               |

Per i colori del bordo, basta specificare un `color_base`: il sistema calcola automaticamente highlight e shadow derivandoli tramite tint Haiku.

```toml
[border.active]
color_base = "#3a5f8a"   # Tutto il resto è automatico
```

Per un controllo completo puoi sovrascrivere manualmente:
```toml
color_highlight = "#6a8fba"
color_shadow    = "#1a3a5a"
```

---

### [buttons] — Pulsanti Close e Zoom

```toml
[buttons]
size     = 12       # Dimensione quadrata in pixel (8–20)
margin   = 5        # Distanza dal bordo del tab
spacing  = 4        # Spazio tra i pulsanti
position = "left"   # left | right
shape    = "circle" # circle | square | rounded_square | diamond
```

**Icone dei pulsanti:**

```toml
[buttons.icon]
style        = "symbol"   # symbol | dot | cross_x | classic_be | none
scale        = 0.6        # Dimensione relativa al pulsante (0.4–0.9)
stroke_width = 1.5        # Spessore linea
```

| Stile        | Descrizione                           |
|--------------|---------------------------------------|
| `symbol`     | X per close, □ per zoom (stile moderno)|
| `dot`        | Solo un punto                         |
| `cross_x`    | X sottile per entrambi                |
| `classic_be` | Stile iconico BeOS R4/R5              |
| `none`       | Solo il colore, nessuna icona         |

**Effetto hover:**
```toml
[buttons.hover_effect]
type = "lighten"   # lighten | darken | glow | scale | none
```

---

### [title] — Testo del titolo

```toml
[title]
alignment  = "center"   # left | center | right
shadow     = false       # true = ombra sottile sotto il testo
truncation = "end"       # end | middle  (come tagliare i titoli lunghi)
```

---

### [resize_corner] — Angolo di ridimensionamento

```toml
[resize_corner]
style = "knob"    # knob | lines | none
size  = 18        # Area in pixel
color = ""        # Vuoto = eredita da border.active.color_base
```

---

### [floating] — Finestre floating

Le finestre di tipo pannello (palette strumenti, finestre di dialogo flottanti) usano queste impostazioni separate per avere un aspetto più discreto.

```toml
[floating]
tab_height   = 14   # Tab più sottile
border_width = 3
tab_color    = ""   # Vuoto = usa colori di tab.inactive
```

---

## Usare una texture

1. Metti il file PNG nella cartella `textures/` accanto a `theme.conf`
2. Imposta nel conf:
   ```toml
   [tab.effect]
   type = "texture"

   [tab.texture]
   file       = "textures/tuo_file.png"
   tile       = true
   blend_mode = "multiply"
   opacity    = 0.35
   ```
3. Esegui `./generate.sh`

**Consigli per le texture:**
- Per il tiling usa immagini ≤ 64×64 px per prestazioni ottimali
- PNG in scala di grigi funzionano bene con `blend_mode = "multiply"` o `"overlay"`
- Riduci `opacity` se la texture risulta troppo invadente
- Haiku non ha compositing hardware nei decorator: texture grandi su schermi ad alta risoluzione possono risultare lente

---

## Struttura del progetto

```
HaikuDecoratorSDK/
│
├── theme.conf              ← MODIFICA QUESTO FILE
├── generate.sh             ← Compila e installa
├── Makefile
│
├── themes/                 ← Temi preset di esempio
│   ├── DarkFlat.conf
│   ├── ClassicBeOS.conf
│   └── ArcLight.conf
│
├── textures/               ← Metti qui i tuoi PNG per le texture
│
├── sdk/                    ← Codice dell'SDK (non modificare)
│   ├── include/
│   │   ├── ThemeConfig.h   — Strutture dati della configurazione
│   │   ├── ConfigReader.h  — Parser di theme.conf
│   │   ├── ThemeRenderer.h — Disegno di tab, bordi, pulsanti
│   │   ├── TabPainter.h    — Gestione texture PNG
│   │   └── SDKDecorator.h  — Classe decorator principale
│   └── src/
│       ├── ConfigReader.cpp
│       ├── ThemeRenderer.cpp
│       ├── TabPainter.cpp
│       └── SDKDecorator.cpp
│
├── tools/
│   └── validate_theme.py   ← Validatore del conf (eseguito da generate.sh)
│
└── output/                 ← Il .so compilato appare qui
```

---

## Risoluzione dei problemi

**`generate.sh` segnala errori di configurazione**  
Leggi il messaggio: indica la riga esatta in `theme.conf` e il valore corretto. Correggi e riesegui.

**Errore di compilazione "header not found"**  
Installa `haiku_devel` con:
```sh
pkgman install haiku_devel
```

**Il decorator non appare nelle Preferenze Aspetto**  
Verifica che il file `.so` sia in `/boot/home/config/non-packaged/add-ons/decorators/`:
```sh
ls /boot/home/config/non-packaged/add-ons/decorators/
```
Se la cartella non esiste, viene creata automaticamente da `make install`.

**Il decorator appare ma la finestra ha l'aspetto sbagliato**  
Controlla i colori in `theme.conf`: un colore malformato viene silenziosamente ignorato e sostituito con il default. Usa `./tools/validate_theme.py theme.conf` per una diagnosi rapida.

**Vuoi tornare al decorator predefinito**  
Apri Preferenze → Aspetto e seleziona "Default".

---

## Personalizzazione avanzata (per chi conosce il C++)

Se vuoi comportamenti non esposti da `theme.conf`, puoi modificare direttamente `sdk/src/SDKDecorator.cpp`. I metodi rilevanti sono:

- `_DrawFrame()` — disegno dei bordi
- `_DrawTab()` — disegno del tab completo
- `_DrawTitle()` — rendering del titolo
- `_DrawClose()` / `_DrawZoom()` — pulsanti
- `GetComponentColors()` — array di colori per ogni componente

La classe `ThemeRenderer` è il punto di estensione principale: aggiungi nuovi metodi lì e chiamali da `SDKDecorator`.

---

## Licenza

MIT. Puoi usare, modificare e distribuire liberamente questo SDK,  
incluso distribuire i decorator che crei con esso.
