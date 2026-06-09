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

### Limitazioni note
- Shape `wave` non implementata (alias di `flat`)
- Shape `slanted` parziale (clip trapezoidale non ancora attivo)
- Nessun hot-reload di theme.conf
- Texture grandi su schermi HiDPI possono essere lente (blending software)
- Nessun alpha compositing verso il contenuto della finestra (limitazione Haiku)
