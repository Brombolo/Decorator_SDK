# Changelog

## v1.0.0 — Prima release

### Funzionalità principali
- Configurazione completa tramite `theme.conf` (formato TOML)
- Supporto tab: forme flat/rounded/slanted, altezza, font, colori active/inactive
- Effetti tab: none, gradient (V/H/diagonal), glass, texture PNG, tint_from_system
- Texture PNG con blending software: normal, multiply, overlay, screen
- Bordi: stili flat/beveled/inset, calcolo automatico highlight/shadow da color_base
- Pulsanti: forme circle/square/rounded_square/diamond, icone symbol/dot/cross_x/classic_be
- Hover effect: lighten, darken, glow, scale
- Supporto Stack & Tile con colori e indicatore visivo dedicati
- Finestre floating con tab e bordi separati
- Angolo resize: stili knob/lines/none
- Titolo: allineamento left/center/right, ombra, truncation
- Validatore Python (`tools/validate_theme.py`) con messaggi di errore precisi
- Script `generate.sh` per build e install in un comando
- Tre temi preset: DarkFlat, ClassicBeOS, ArcLight
- Documentazione completa: README, QUICKSTART, THEME_REFERENCE, SDK_INTERNALS

### Vincoli strutturali di Haiku (non risolvibili nell'SDK)

Questi non sono bug da correggere in futuro, ma confini dell'architettura
di Haiku che definiscono lo spazio di design possibile per qualsiasi decorator:

- **Nessun alpha compositing verso il contenuto della finestra.** app_server
  non espone un compositing layer ai decorator. Il tab non può essere
  semitrasparente verso l'interno della finestra. L'effetto `glass` è
  un overlay bianco opaco — non vera trasparenza.
- **Blending texture in software.** Per lo stesso motivo non esiste
  accelerazione hardware per il blending. Le texture funzionano, ma il costo
  è CPU. Texture ≤ 64×64 px in tiling hanno impatto trascurabile.
- **Forma `wave` non inclusa.** Richiederebbe path drawing con curve di Bézier
  su ogni ridisegno del tab — incompatibile con le performance di app_server
  che ridisegna i decorator in modo sincrono.
- **Forma `slanted` semplificata.** Il clip trapezoidale completo richiede
  di sovrascrivere `GetTabRect()` modificando la geometria di layout dell'intero
  decorator. Fuori dallo scope della v1.

### Da fare (versioni future)
- Hot-reload di theme.conf senza ricompilare (via BNodeMonitor)
- Forma `slanted` completa con override di `GetTabRect()`
- GUI configurator nativa Haiku per modificare theme.conf visualmente
