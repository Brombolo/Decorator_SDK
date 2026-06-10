/*
 * Haiku Decorator SDK — ThemeRenderer.cpp
 *
 * Calcola strutture dati di disegno (TabDrawSpec, ButtonDrawSpec, colori).
 * Nessuna dipendenza da header interni di app_server.
 * Il disegno effettivo è in SDKDecorator.cpp tramite API pubbliche.
 */

#include "ThemeRenderer.h"
#include <InterfaceDefs.h>   // tint_color, B_LIGHTEN_x_TINT, B_DARKEN_x_TINT
#include <algorithm>
#include <cmath>

using std::min;
using std::max;

// ─── Costruttore ──────────────────────────────────────────────────────────────

ThemeRenderer::ThemeRenderer(const ThemeConfig& config)
    : fConfig(config)
{
}

// ─── Utilità colore ───────────────────────────────────────────────────────────

rgb_color ThemeRenderer::Lighten(rgb_color c, float amount) {
    return {
        (uint8)min(255, (int)(c.red   + 255 * amount)),
        (uint8)min(255, (int)(c.green + 255 * amount)),
        (uint8)min(255, (int)(c.blue  + 255 * amount)),
        c.alpha
    };
}

rgb_color ThemeRenderer::Darken(rgb_color c, float amount) {
    return {
        (uint8)max(0, (int)(c.red   - 255 * amount)),
        (uint8)max(0, (int)(c.green - 255 * amount)),
        (uint8)max(0, (int)(c.blue  - 255 * amount)),
        c.alpha
    };
}

rgb_color ThemeRenderer::Lerp(rgb_color a, rgb_color b, float t) {
    return {
        (uint8)(a.red   + (b.red   - a.red)   * t),
        (uint8)(a.green + (b.green - a.green) * t),
        (uint8)(a.blue  + (b.blue  - a.blue)  * t),
        (uint8)(a.alpha + (b.alpha - a.alpha) * t)
    };
}

// ─── Risoluzione colori tab ───────────────────────────────────────────────────

TabColorSet ThemeRenderer::_ResolveTabColors(bool focused, bool stackTile) const {
    if (stackTile) {
        TabColorSet cs;
        cs.color_start  = fConfig.tab.stack_tile.color_start;
        cs.color_end    = fConfig.tab.stack_tile.color_end;
        cs.text_color   = fConfig.tab.stack_tile.text_color;
        cs.border_color = fConfig.tab.active.border_color;
        return cs;
    }
    return focused ? fConfig.tab.active : fConfig.tab.inactive;
}

// ─── CalcTab ─────────────────────────────────────────────────────────────────

TabDrawSpec ThemeRenderer::CalcTab(const BRect& tabRect,
                                    bool focused, bool stackTile) const
{
    TabDrawSpec spec = {};
    const TabColorSet colors = _ResolveTabColors(focused, stackTile);
    const TabEffectConfig& fx = fConfig.tab.effect;

    // Colori bordi
    spec.border_outer        = colors.border_color;
    spec.border_outer_shadow = Darken(colors.border_color, 0.10f);
    spec.bevel_light         = Lighten(colors.color_start, 0.12f);
    spec.bevel_shadow        = Darken(colors.color_start,  0.08f);

    // Fill base (sempre presente come fallback)
    spec.fill_color = colors.color_start;

    // Effetto
    spec.gradient.active = false;
    spec.glass_enabled   = false;
    spec.inner_glow_enabled = false;

    switch (fx.type) {
        case EFFECT_GRADIENT: {
            spec.gradient.active      = true;
            spec.gradient.color_start = colors.color_start;
            spec.gradient.color_end   = colors.color_end;
            BRect fill = tabRect.InsetByCopy(2.0f, 2.0f);
            if (fx.gradient_dir == GRAD_VERTICAL) {
                spec.gradient.start_x = fill.left;
                spec.gradient.start_y = fill.top;
                spec.gradient.end_x   = fill.left;
                spec.gradient.end_y   = fill.bottom;
            } else if (fx.gradient_dir == GRAD_HORIZONTAL) {
                spec.gradient.start_x = fill.left;
                spec.gradient.start_y = fill.top;
                spec.gradient.end_x   = fill.right;
                spec.gradient.end_y   = fill.top;
            } else { // DIAGONAL
                spec.gradient.start_x = fill.left;
                spec.gradient.start_y = fill.top;
                spec.gradient.end_x   = fill.right;
                spec.gradient.end_y   = fill.bottom;
            }
            break;
        }
        case EFFECT_GLASS: {
            spec.glass_enabled = true;
            uint8 alpha = (uint8)(fx.glass_opacity * 255.0f);
            spec.glass_color = {255, 255, 255, alpha};
            BRect fill = tabRect.InsetByCopy(2.0f, 2.0f);
            spec.glass_rect = fill;
            spec.glass_rect.bottom = fill.top + floorf(fill.Height() * 0.5f);
            break;
        }
        case EFFECT_TINT_FROM_SYSTEM:
        case EFFECT_TEXTURE:
        case EFFECT_NONE:
        default:
            break;
    }

    // Inner glow
    if (fx.inner_glow) {
        spec.inner_glow_enabled = true;
        spec.inner_glow_color   = fx.inner_glow_color;
    }

    // Separatore
    spec.separator_enabled = fx.separator_line;
    spec.separator_color   = fx.separator_color;

    // Stack & Tile indicator
    spec.stack_indicator = stackTile;
    if (stackTile) {
        spec.indicator_color = fConfig.tab.stack_tile.indicator_color;
        float cx = tabRect.right - 8.0f;
        float cy = tabRect.top + (tabRect.Height() / 2.0f);
        spec.indicator_rect = BRect(cx - 2, cy - 2, cx + 2, cy + 2);
    }

    return spec;
}

// ─── CalcBorderColors ────────────────────────────────────────────────────────

void ThemeRenderer::CalcBorderColors(rgb_color colors[6], bool focused) const {
    const BorderColorSet& bc = focused
        ? fConfig.border.active
        : fConfig.border.inactive;

    rgb_color base = bc.color_base;

    colors[0] = bc.highlight_auto ? Darken(base,  0.28f) : bc.color_highlight;
    colors[1] = bc.highlight_auto ? Lighten(base, 0.25f) : bc.color_highlight;
    colors[2] = bc.shadow_auto    ? Darken(base,  0.03f) : bc.color_shadow;
    colors[3] = bc.shadow_auto    ? Darken(base,  0.34f) : bc.color_shadow;
    colors[4] = colors[0];
    colors[5] = bc.shadow_auto    ? Darken(base,  0.50f) : bc.color_shadow;
}

// ─── CalcButton ──────────────────────────────────────────────────────────────

ButtonDrawSpec ThemeRenderer::CalcButton(bool isClose,
                                          bool pressed, bool focused) const
{
    ButtonDrawSpec spec = {};
    const ButtonColorSet& bcs = isClose
        ? fConfig.buttons.close
        : fConfig.buttons.zoom;

    // Colore background in base allo stato
    rgb_color bg;
    if (pressed)
        bg = bcs.color_pressed;
    else if (focused)
        bg = bcs.color_normal;
    else
        bg = Darken(bcs.color_normal, 0.15f);

    // Pre-bake hover effect (nel bitmap "normal")
    if (!pressed) {
        switch (fConfig.buttons.hover_effect.type) {
            case HOVER_LIGHTEN: bg = Lighten(bg, 0.06f); break;
            case HOVER_DARKEN:  bg = Darken(bg,  0.06f); break;
            default: break;
        }
    }

    spec.bg_color    = bg;
    spec.highlight   = Lighten(bg, 0.20f);
    spec.shadow      = Darken(bg,  0.25f);
    spec.icon_color  = bcs.icon_color;
    spec.shape       = fConfig.buttons.shape;
    spec.icon_style  = fConfig.buttons.icon.style;
    spec.icon_scale  = fConfig.buttons.icon.scale;
    spec.stroke_width= fConfig.buttons.icon.stroke_width;
    spec.is_close    = isClose;
    spec.pressed     = pressed;

    return spec;
}
