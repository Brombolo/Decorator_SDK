# theme.conf — Riferimento Completo

Tutte le chiavi disponibili con tipo, valori ammessi e default.

## Legenda tipi

| Tipo    | Descrizione                                      | Esempio        |
|---------|--------------------------------------------------|----------------|
| `int`   | Numero intero                                    | `22`           |
| `float` | Numero decimale                                  | `0.5`          |
| `bool`  | Booleano                                         | `true`/`false` |
| `color` | Colore esadecimale `#RRGGBB` o `#RRGGBBAA`       | `"#3a5f8a"`    |
| `enum`  | Una delle stringhe indicate                      | `"flat"`       |
| `str`   | Stringa libera                                   | `"MyTheme"`    |

`""` (stringa vuota) su chiavi `color` o opzionali = **calcolato automaticamente**.

---

## [meta]

| Chiave    | Tipo  | Default        | Note                          |
|-----------|-------|----------------|-------------------------------|
| `name`    | `str` | `"MyDecorator"`| Diventa il nome del file .so  |
| `author`  | `str` | `""`           | Opzionale                     |
| `version` | `str` | `"1.0"`        | Opzionale                     |

---

## [tab]

| Chiave          | Tipo    | Default  | Range/Valori                             |
|-----------------|---------|----------|------------------------------------------|
| `height`        | `int`   | `22`     | 14–48                                    |
| `font_size`     | `int`   | `12`     | 8–18                                     |
| `font_bold`     | `bool`  | `false`  |                                          |
| `shape`         | `enum`  | `"flat"` | `flat` `rounded` `slanted` `wave`        |
| `corner_radius` | `int`   | `0`      | 0–12 (solo se `shape=rounded`)           |
| `slant_angle`   | `float` | `15`     | 5.0–45.0° (solo se `shape=slanted`)      |

---

## [tab.active] e [tab.inactive]

| Chiave         | Tipo    | Default     | Note                          |
|----------------|---------|-------------|-------------------------------|
| `color_start`  | `color` | —           | Colore principale del tab     |
| `color_end`    | `color` | —           | Fine gradiente (= start = piatto) |
| `text_color`   | `color` | —           | Colore testo titolo           |
| `border_color` | `color` | —           | Bordo esterno del tab         |

---

## [tab.stack_tile]

| Chiave            | Tipo    | Default | Note                             |
|-------------------|---------|---------|----------------------------------|
| `color_start`     | `color` | —       | Colore tab in modalità Stack&Tile|
| `color_end`       | `color` | —       |                                  |
| `text_color`      | `color` | —       |                                  |
| `indicator_color` | `color` | —       | Punto indicatore dello stack     |

---

## [tab.effect]

| Chiave              | Tipo    | Default      | Range/Valori                                      |
|---------------------|---------|--------------|---------------------------------------------------|
| `type`              | `enum`  | `"none"`     | `none` `gradient` `glass` `texture` `tint_from_system` |
| `gradient_dir`      | `enum`  | `"vertical"` | `vertical` `horizontal` `diagonal`                |
| `glass_opacity`     | `float` | `0.25`       | 0.0–1.0                                           |
| `tint_amount`       | `float` | `0.15`       | 0.0–1.0                                           |
| `inner_glow`        | `bool`  | `false`      | Luce interna sul bordo superiore                  |
| `inner_glow_color`  | `color` | `"#ffffff44"`|                                                   |
| `separator_line`    | `bool`  | `true`       | Linea di separazione tab/contenuto                |
| `separator_color`   | `color` | `""`         | Vuoto = auto (derivato da `color_start`)          |

---

## [tab.texture]

Attiva solo quando `[tab.effect] type = "texture"`.

| Chiave       | Tipo    | Default      | Valori                              |
|--------------|---------|--------------|-------------------------------------|
| `file`       | `str`   | `""`         | Percorso PNG relativo a `theme.conf`|
| `tile`       | `bool`  | `true`       | `true`=tasselli `false`=stirata     |
| `blend_mode` | `enum`  | `"multiply"` | `normal` `multiply` `overlay` `screen` |
| `opacity`    | `float` | `0.4`        | 0.0–1.0                             |

---

## [border]

| Chiave  | Tipo   | Default     | Range/Valori                       |
|---------|--------|-------------|------------------------------------|
| `width` | `int`  | `5`         | 1–12                               |
| `style` | `enum` | `"beveled"` | `flat` `beveled` `inset` `shadow`  |

---

## [border.active] e [border.inactive]

| Chiave            | Tipo    | Default | Note                                   |
|-------------------|---------|---------|----------------------------------------|
| `color_base`      | `color` | —       | Colore base; deriva auto gli altri     |
| `color_highlight` | `color` | `""`    | Vuoto = auto (tint B_LIGHTEN_2)        |
| `color_shadow`    | `color` | `""`    | Vuoto = auto (tint B_DARKEN_3)         |

---

## [border.corners]

| Chiave      | Tipo   | Default | Range    |
|-------------|--------|---------|----------|
| `radius`    | `int`  | `0`     | 0–8      |
| `anti_alias`| `bool` | `true`  |          |

---

## [buttons]

| Chiave          | Tipo   | Default    | Range/Valori                                  |
|-----------------|--------|------------|-----------------------------------------------|
| `size`          | `int`  | `12`       | 8–20                                          |
| `margin`        | `int`  | `5`        | 1–20                                          |
| `spacing`       | `int`  | `4`        | 0–20                                          |
| `position`      | `enum` | `"left"`   | `left` `right`                                |
| `shape`         | `enum` | `"circle"` | `circle` `square` `rounded_square` `diamond`  |
| `corner_radius` | `int`  | `3`        | 0–8 (solo se `shape=rounded_square`)          |

---

## [buttons.icon]

| Chiave        | Tipo    | Default     | Range/Valori                                       |
|---------------|---------|-------------|----------------------------------------------------|
| `style`       | `enum`  | `"symbol"`  | `symbol` `dot` `cross_x` `classic_be` `none`       |
| `scale`       | `float` | `0.6`       | 0.4–0.9                                            |
| `stroke_width`| `float` | `1.5`       | 0.5–4.0                                            |

---

## [buttons.close] e [buttons.zoom]

| Chiave          | Tipo    | Default | Note                             |
|-----------------|---------|---------|----------------------------------|
| `color_normal`  | `color` | —       | Colore a riposo                  |
| `color_hover`   | `color` | —       | Colore al passaggio del mouse    |
| `color_pressed` | `color` | —       | Colore quando premuto            |
| `icon_color`    | `color` | —       | Colore dell'icona interna        |

---

## [buttons.hover_effect]

| Chiave         | Tipo    | Default     | Range/Valori                          |
|----------------|---------|-------------|---------------------------------------|
| `type`         | `enum`  | `"lighten"` | `lighten` `darken` `glow` `scale` `none` |
| `scale_factor` | `float` | `1.08`      | 0.5–2.0 (solo se `type=scale`)        |

---

## [resize_corner]

| Chiave  | Tipo    | Default  | Range/Valori         |
|---------|---------|----------|----------------------|
| `style` | `enum`  | `"knob"` | `knob` `lines` `none`|
| `size`  | `int`   | `18`     | 8–40                 |
| `color` | `color` | `""`     | Vuoto = eredita da `border.active.color_base` |

---

## [title]

| Chiave        | Tipo    | Default     | Valori                  |
|---------------|---------|-------------|-------------------------|
| `alignment`   | `enum`  | `"center"`  | `left` `center` `right` |
| `shadow`      | `bool`  | `false`     |                         |
| `shadow_color`| `color` | `"#00000044"`|                        |
| `truncation`  | `enum`  | `"end"`     | `end` `middle`          |

---

## [floating]

| Chiave         | Tipo    | Default | Note                                     |
|----------------|---------|---------|------------------------------------------|
| `tab_height`   | `int`   | `14`    | 8–32                                     |
| `border_width` | `int`   | `3`     | 1–8                                      |
| `tab_color`    | `color` | `""`    | Vuoto = usa `tab.inactive.color_start`   |

---

## [rendering]

| Chiave            | Tipo   | Default | Note                                      |
|-------------------|--------|---------|-------------------------------------------|
| `scale_with_dpi`  | `bool` | `true`  | Scala i valori pixel con il DPI del display |
| `min_tab_height`  | `int`  | `16`    | 8–32                                      |

---

## Note generali

**Calcolo automatico dei colori**  
Quando una chiave `color` è lasciata vuota (`""`), l'SDK calcola il valore usando le funzioni di tint di Haiku sul `color_base` più vicino nella gerarchia. Questo garantisce coerenza visiva automatica.

**Stack & Tile**  
Haiku permette di raggruppare finestre con Stack & Tile. La sezione `[tab.stack_tile]` definisce l'aspetto del tab quando una finestra fa parte di un gruppo. Se non specificata, il decorator usa i colori di `[tab.active]` leggermente modificati.

**Dipendenza dalla forma e dall'effetto**  
Alcune chiavi hanno effetto solo in combinazione:

| Chiave                  | Richiede                   |
|-------------------------|----------------------------|
| `tab.corner_radius`     | `tab.shape = "rounded"`    |
| `tab.slant_angle`       | `tab.shape = "slanted"`    |
| `tab.effect.glass_opacity` | `tab.effect.type = "glass"` |
| `tab.texture.*`         | `tab.effect.type = "texture"` |
| `buttons.corner_radius` | `buttons.shape = "rounded_square"` |
| `buttons.hover_effect.scale_factor` | `buttons.hover_effect.type = "scale"` |
