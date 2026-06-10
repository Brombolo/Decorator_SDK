#ifndef THEME_RENDERER_H
#define THEME_RENDERER_H

/*
 * Haiku Decorator SDK — ThemeRenderer.h
 *
 * ThemeRenderer NON include header interni di app_server.
 * Calcola solo dati (colori, rect, strutture) che SDKDecorator
 * usa tramite le API pubbliche di Decorator / DrawingEngine
 * già esposte da <Decorator.h>.
 */

#include "ThemeConfig.h"
#include <Rect.h>
#include <GraphicsDefs.h>   // rgb_color, BGradientLinear
#include <GradientLinear.h>

// ─── Strutture dati di output (nessuna dipendenza da app_server) ──────────────

// Risultato del calcolo di un gradiente
struct GradientSpec {
    bool            active;         // false = usa fill_color piatto
    rgb_color       color_start;
    rgb_color       color_end;
    float           start_x, start_y;
    float           end_x,   end_y;
};

// Tutto quello che serve per disegnare il tab in un singolo passaggio
struct TabDrawSpec {
    // Fill
    rgb_color       fill_color;
    GradientSpec    gradient;

    // Bordi esterni (3 lati: top, left, right — bottom è aperto)
    rgb_color       border_outer;
    rgb_color       border_outer_shadow;

    // Bevel interno
    rgb_color       bevel_light;
    rgb_color       bevel_shadow;

    // Glass overlay (se effect=glass)
    bool            glass_enabled;
    rgb_color       glass_color;
    BRect           glass_rect;

    // Separatore
    bool            separator_enabled;
    rgb_color       separator_color;

    // Stack & Tile indicator
    bool            stack_indicator;
    rgb_color       indicator_color;
    BRect           indicator_rect;

    // Inner glow (bordo superiore)
    bool            inner_glow_enabled;
    rgb_color       inner_glow_color;
};

// Tutto quello che serve per disegnare un pulsante
struct ButtonDrawSpec {
    rgb_color       bg_color;
    rgb_color       highlight;
    rgb_color       shadow;
    rgb_color       icon_color;
    ButtonShape     shape;
    ButtonIconStyle icon_style;
    float           icon_scale;
    float           stroke_width;
    bool            is_close;
    bool            pressed;
};

// ─── ThemeRenderer ────────────────────────────────────────────────────────────

class ThemeRenderer {
public:
    explicit        ThemeRenderer(const ThemeConfig& config);

    // Calcola la specifica di disegno per il tab
    TabDrawSpec     CalcTab(const BRect& tabRect,
                             bool focused,
                             bool stackTile) const;

    // Calcola i 6 colori del bordo (formato Haiku GetComponentColors)
    void            CalcBorderColors(rgb_color colors[6], bool focused) const;

    // Calcola la specifica di disegno per un pulsante
    ButtonDrawSpec  CalcButton(bool isClose, bool pressed, bool focused) const;

    // Utilità colore (pubbliche per uso in SDKDecorator)
    static rgb_color Lighten(rgb_color c, float amount);
    static rgb_color Darken(rgb_color c, float amount);
    static rgb_color Lerp(rgb_color a, rgb_color b, float t);

private:
    const ThemeConfig&  fConfig;

    TabColorSet     _ResolveTabColors(bool focused, bool stackTile) const;
};

#endif // THEME_RENDERER_H
