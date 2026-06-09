# SDK Internals — Guida per sviluppatori

Questo documento descrive l'architettura interna dell'SDK per chi vuole
estenderlo, modificare il comportamento di default o contribuire al progetto.

---

## Architettura generale

```
theme.conf
    │
    ▼
ConfigReader::Load()
    │  Popola
    ▼
ThemeConfig  (struct aggregata in ThemeConfig.h)
    │
    ├──► ThemeRenderer  (disegno tab, bordi, pulsanti)
    │         │
    │         └──► TabPainter  (texture PNG, solo se effect.type=texture)
    │
    └──► SDKDecorator  (classe Haiku, estende SATDecorator)
              │
              └──► app_server  (DrawingEngine, ServerBitmap, ecc.)
```

Il flusso di vita è:

1. `instantiate_decor_addon()` viene chiamata da `app_server` al caricamento del `.so`
2. `SDKDecorAddOn::_AllocateDecorator()` chiama `ConfigReader::Load()` → ottiene `ThemeConfig`
3. Viene creato un `SDKDecorator` che tiene in memoria `ThemeConfig` e un `ThemeRenderer`
4. Per ogni finestra, `app_server` chiama i metodi `_Draw*` dell'istanza del decorator

---

## ThemeConfig.h — Le strutture dati

`ThemeConfig` è una struct pura (nessun metodo), composta da sotto-struct:

```
ThemeConfig
├── BString name, author, version
├── TabConfig tab
│     ├── int32 height, font_size; bool font_bold
│     ├── TabShape shape; int32 corner_radius; float slant_angle
│     ├── TabColorSet active, inactive
│     ├── struct { rgb_color ... } stack_tile
│     ├── TabEffectConfig effect
│     └── TabTextureConfig texture
├── BorderConfig border
│     ├── int32 width; BorderStyle style
│     ├── BorderColorSet active, inactive
│     └── struct { int32 radius; bool anti_alias } corners
├── ButtonsConfig buttons
│     ├── int32 size, margin, spacing, corner_radius
│     ├── ButtonPosition position; ButtonShape shape
│     ├── struct { ButtonIconStyle style; float scale, stroke_width } icon
│     ├── ButtonColorSet close, zoom
│     └── struct { ButtonHoverEffect type; float scale_factor } hover_effect
├── ResizeCornerConfig resize_corner
├── TitleConfig title
├── FloatingConfig floating
└── RenderingConfig rendering
```

Tutte le `rgb_color` usano il tipo nativo Haiku (`<GraphicsDefs.h>`).  
I flag `*_auto` (es. `border.active.highlight_auto`) indicano se il colore
corrispondente deve essere calcolato automaticamente da `ConfigReader::Load()`
nella fase di post-processing.

---

## ConfigReader.cpp — Il parser TOML

Il parser è volutamente minimale: non implementa la specifica TOML completa,
solo il sottoinsieme usato da `theme.conf`:

- Sezioni `[nome]` e `[nome.sotto]`
- Coppie `chiave = valore`
- Stringhe tra virgolette doppie
- Commenti `#` (anche inline)
- Nessun supporto per array, tabelle inline, multiriga

### Algoritmo di parsing

```
per ogni riga:
  1. trim() + rimuovi commento inline
  2. se [sezione] → aggiorna section corrente
  3. se key = value → costruisci "section.key" e dispatcha
```

Il dispatch è una sequenza lineare di confronti stringa (`strcmp`).
Non è elegante ma è trasparente, zero-dipendenze e facile da estendere:
per aggiungere una nuova chiave basta aggiungere un blocco `if`.

### Post-processing

Dopo il parsing di tutte le righe, `Load()` esegue la risoluzione dei colori
automatici:

```cpp
// Esempio: separator_color auto
if (cfg.tab.effect.separator_auto)
    cfg.tab.effect.separator_color =
        tint_color(cfg.tab.active.color_start, B_DARKEN_2_TINT);
```

I tint usati seguono la convenzione Haiku (`<InterfaceDefs.h>`):
- `B_LIGHTEN_1_TINT` / `B_LIGHTEN_2_TINT` — schiarisce
- `B_DARKEN_1_TINT` .. `B_DARKEN_4_TINT` — scurisce

---

## ThemeRenderer.cpp — Il motore di disegno

### DrawTab()

```
DrawTab(engine, tabRect, focused, stackTile)
  │
  ├── StrokeLine ×3  → bordo esterno (3 lati, escluso bottom)
  ├── StrokeLine ×3  → bevel interno 1px
  ├── _DrawTabShape()  → riempimento con la forma corretta
  ├── _ApplyEffect()   → gradiente / glass / texture sopra il fill
  ├── StrokeLine       → separator_line (se abilitata)
  └── FillEllipse      → indicator Stack&Tile (se stackTile)
```

### _DrawTabShape()

Gestisce il clip geometrico in base a `tab.shape`:

| Shape      | Implementazione                                      |
|------------|------------------------------------------------------|
| `flat`     | `FillRect` semplice                                  |
| `rounded`  | `FillRect` + `FillEllipse` agli angoli               |
| `slanted`  | `FillRect` (il clip trapezoidale è responsabilità del caller — vedere nota) |
| `wave`     | Alias di `flat` nella v1 (implementazione futura)    |

> **Nota `slanted`:** la geometria del clip trapezoidale richiede di sovrascrivere
> `GetTabRect()` in `SDKDecorator` per restituire una `BRect` con offset laterale.
> Questa è un'estensione pianificata per la v1.1.

### _ApplyEffect()

Lavora sempre su `fillRect = tabRect.InsetByCopy(2, 2)` (dentro i bordi).

- `EFFECT_GRADIENT` → usa `BGradientLinear` di Haiku, il modo più efficiente
- `EFFECT_GLASS` → due passate: fill base + fill bianco semitrasparente sulla metà superiore
- `EFFECT_TEXTURE` → delega a `TabPainter::Paint()` (implementazione separata)
- `EFFECT_TINT_FROM_SYSTEM` → risolto già in `_ResolveTabColors()`, qui solo fill

### GetBorderColors()

Popola un array `rgb_color[6]` compatibile con il formato interno di `GetComponentColors()`.
L'ordinamento degli slot segue la convenzione del decorator `Default` di Haiku:

```
[0] = bordo esterno scuro
[1] = highlight esterno
[2] = fill interno
[3] = shadow interna
[4] = = [0]
[5] = bordo più scuro (angolo)
```

### CreateButtonBitmap()

Usa `BitmapDrawingEngine` per disegnare il pulsante in un bitmap off-screen,
che viene poi ritornato al decorator e messo in cache in `tab->closeBitmaps[]` /
`tab->zoomBitmaps[]`.

La cache ha 4 slot per pulsante:
```
index = (focused ? 0 : 1) + (pressed ? 0 : 2)
  0 = focused + normal
  1 = unfocused + normal
  2 = focused + pressed
  3 = unfocused + pressed
```

Il bitmap viene generato una sola volta e riusato finché non cambia il focus
(il decorator invalida i bitmap quando necessario tramite il meccanismo standard
di Haiku).

---

## TabPainter.cpp — Texture PNG

### Caricamento

Usa `BTranslationUtils::GetBitmapFile()` — supporta tutti i formati per cui
Haiku ha un translator installato (PNG, JPEG, BMP, TIFF...).

Il bitmap viene convertito in `B_RGBA32` al caricamento per uniformare il
loop di blending.

### Loop di blending

Il blending è software, pixel per pixel. Non è veloce, ma:

1. Haiku non espone compositing hardware ai decorator
2. Per texture tilizzate piccole (≤ 64×64) e tab di altezza normale (≤ 32px)
   il costo è trascurabile
3. Per casi critici di performance, usa `blend_mode = "normal"` con `opacity` alta
   che esegue solo un lerp per canale

Le formule implementate:

```
multiply:  out = lerp(base, base*tex/255, opacity)
overlay:   out = lerp(base, overlay(base,tex), opacity)
           dove overlay(b,t) = 2bt/255 se b<128, 255-2(1-b)(1-t) altrimenti
screen:    out = lerp(base, 1-(1-b)(1-t), opacity)
normal:    out = lerp(base, tex, opacity)
```

---

## SDKDecorator.cpp — Integrazione con Haiku

### Gerarchia di ereditarietà

```
Decorator  (Haiku base)
  └── SATDecorator  (aggiunge supporto Stack & Tile)
        └── SDKDecorator  (questo SDK)
```

`SATDecorator` è il punto di partenza raccomandato per i decorator Haiku moderni
perché gestisce automaticamente la logica di evidenziazione dello stack.

### Metodi override

| Metodo                    | Cosa fa nell'SDK                                    |
|---------------------------|-----------------------------------------------------|
| `GetComponentColors()`    | Mappa `ThemeConfig` → array `rgb_color[6]` per Haiku|
| `_DrawFrame()`            | Disegna i 4 bordi + resize corner                   |
| `_DrawTab()`              | Chiama `ThemeRenderer::DrawTab()` + title + buttons |
| `_DrawTitle()`            | Testo con allineamento e ombra opzionale             |
| `_DrawClose()`            | Cache bitmap → `ThemeRenderer::CreateButtonBitmap()`|
| `_DrawZoom()`             | Idem per zoom                                       |
| `_GetButtonSizeAndOffset()`| Calcola dimensione/posizione dai valori del conf    |

### Gestione focus

Il focus della finestra è determinato da `tab->buttonFocus` (campo della struct
`Decorator::Tab` di Haiku). Il decorator non gestisce direttamente le transizioni
di focus — Haiku invalida la regione del tab e chiama `_DrawTab()` quando cambia.

### THEME_CONF_PATH

Il percorso del file di configurazione è baked-in al momento della compilazione:

```cpp
#ifndef THEME_CONF_PATH
#define THEME_CONF_PATH "theme.conf"
#endif
```

`generate.sh` passa il percorso assoluto tramite:
```makefile
CXXFLAGS = ... -DTHEME_CONF_PATH='"$(CONF_PATH)"'
```

Questo significa che il `.so` compilato punta sempre al `theme.conf` originale.
Se sposti la cartella del tema dopo la compilazione, devi ricompilare.

---

## Aggiungere una nuova chiave di configurazione

Esempio: aggiungere `tab.active.gradient_midpoint` (punto centrale del gradiente).

**1. ThemeConfig.h** — aggiungi il campo alla struct:
```cpp
struct TabEffectConfig {
    // ... campi esistenti ...
    float gradient_midpoint = 0.5f;   // ← nuovo campo
};
```

**2. ConfigReader.cpp** — aggiungi il parsing nel dispatch:
```cpp
FLOAT_KEY(cfg.tab.effect.gradient_midpoint, "tab.effect.gradient_midpoint")
```

**3. ThemeRenderer.cpp** — usa il nuovo valore in `_ApplyEffect()`:
```cpp
case EFFECT_GRADIENT: {
    gradient.AddColor(colors.color_start, 0);
    gradient.AddColor(_Lerp(colors.color_start, colors.color_end,
                             fx.gradient_midpoint), 128);   // ← midpoint
    gradient.AddColor(colors.color_end, 255);
    ...
}
```

**4. tools/validate_theme.py** — aggiungi la regola di validazione:
```python
"tab.effect.gradient_midpoint": ("float", 0.0, 1.0),
```

**5. docs/THEME_REFERENCE.md** — documenta la nuova chiave nella tabella.

**6. theme.conf** — aggiungi la chiave con il valore di default commentato.

---

## Aggiungere una nuova forma di tab

Esempio: `shape = "pill"` (capsule con entrambi gli angoli arrotondati).

**1. ThemeConfig.h:**
```cpp
enum TabShape {
    TAB_SHAPE_FLAT = 0,
    TAB_SHAPE_ROUNDED,
    TAB_SHAPE_SLANTED,
    TAB_SHAPE_WAVE,
    TAB_SHAPE_PILL       // ← nuovo
};
```

**2. ConfigReader.cpp:**
```cpp
TabShape ConfigReader::_ParseTabShape(const char* v) {
    // ... casi esistenti ...
    if (strcmp(v, "pill") == 0) return TAB_SHAPE_PILL;
    return TAB_SHAPE_FLAT;
}
```

**3. ThemeRenderer.cpp** — aggiungi il caso in `_DrawTabShape()`:
```cpp
case TAB_SHAPE_PILL: {
    float r = fill.Height() / 2.0f;
    // Semicerchio sinistro
    engine->FillEllipse(BRect(fill.left, fill.top,
                               fill.left + r*2, fill.bottom), fillColor);
    // Rettangolo centrale
    engine->FillRect(BRect(fill.left + r, fill.top,
                            fill.right - r, fill.bottom), fillColor);
    // Semicerchio destro
    engine->FillEllipse(BRect(fill.right - r*2, fill.top,
                               fill.right, fill.bottom), fillColor);
    break;
}
```

---

## Limitazioni note e note tecniche

### Nessun alpha compositing
`app_server` non espone un compositing layer ai decorator. Tutto il disegno
è opaco (B_OP_COPY) tranne il rendering del titolo che usa B_OP_OVER.
L'effetto `glass` è simulato con un rettangolo bianco con colore semi-opaco
disegnato sopra il fill — questo non è trasparenza vera verso il contenuto
della finestra.

### Thread safety
`SDKDecorator` è istanziato per ogni finestra. I `BitmapDrawingEngine` nei
bitmap dei pulsanti usano una variabile `static` in `CreateButtonBitmap()`.
Questo è safe perché `app_server` garantisce che i metodi del decorator siano
chiamati in modo serializzato per ciascuna istanza. Per istanze multiple
su thread concorrenti, considera di spostare il `BitmapDrawingEngine` come
membro di istanza.

### Texture e reload
`TabPainter` carica la texture una volta al momento della creazione del
decorator (per ogni finestra). Non c'è hot-reload: per applicare una texture
modificata è necessario cambiare decorator e ritornare a quello corrente
dalle Preferenze Aspetto, oppure fare logout/login.

### Floating window look
`B_FLOATING_WINDOW_LOOK` usa i valori della sezione `[floating]`. Il rilevamento
avviene in `_DrawTab()` tramite `tab->look == B_FLOATING_WINDOW_LOOK`.
Questa distinzione è già presente nel codice di `SDKDecorator` ma richiede
di leggere `fTopTab->look` — verifica che il tuo decorator base (`SATDecorator`)
esponga questo campo.

---

## Roadmap futura

| Funzionalità                        | Complessità | Note                                     |
|-------------------------------------|-------------|------------------------------------------|
| Shape `slanted` con clip trapezoidale| Media      | Override di `GetTabRect()` necessario    |
| Shape `wave`                        | Alta        | Richiede path drawing con curve di Bezier|
| Animated focus transition           | Alta        | Richiede timer e invalidation esplicita  |
| Hot-reload di theme.conf            | Bassa       | Aggiungere inotify/node monitor su conf  |
| GUI configurator (app Haiku nativa) | Alta        | App separata che scrive theme.conf       |
| Preset browser integrato            | Media       | Lista temi in `~/config/settings/decorators/` |
