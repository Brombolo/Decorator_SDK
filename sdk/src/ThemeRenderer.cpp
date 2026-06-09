/*
 * Haiku Decorator SDK — ThemeRenderer.cpp
 * Implementazione del rendering: tab, bordi, pulsanti.
 */

#include "ThemeRenderer.h"

#include <algorithm>
#include <cmath>
#include <InterfaceDefs.h>
#include <Region.h>

// Haiku internals (disponibili nell'ambiente di build del decorator)
#include "DrawingEngine.h"
#include "BitmapDrawingEngine.h"
#include "ServerBitmap.h"
#include "RGBColor.h"

using std::min;
using std::max;

// ─── Costruttore ──────────────────────────────────────────────────────────────

ThemeRenderer::ThemeRenderer(const ThemeConfig& config)
    : fConfig(config)
{
}

// ─── Utilità colore ───────────────────────────────────────────────────────────

rgb_color ThemeRenderer::_Lighten(rgb_color c, float amount) {
    return {
        (uint8)min(255, (int)(c.red   + 255 * amount)),
        (uint8)min(255, (int)(c.green + 255 * amount)),
        (uint8)min(255, (int)(c.blue  + 255 * amount)),
        c.alpha
    };
}

rgb_color ThemeRenderer::_Darken(rgb_color c, float amount) {
    return {
        (uint8)max(0, (int)(c.red   - 255 * amount)),
        (uint8)max(0, (int)(c.green - 255 * amount)),
        (uint8)max(0, (int)(c.blue  - 255 * amount)),
        c.alpha
    };
}

rgb_color ThemeRenderer::_Lerp(rgb_color a, rgb_color b, float t) {
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
        cs.border_color = fConfig.tab.active.border_color;  // eredita
        return cs;
    }
    return focused ? fConfig.tab.active : fConfig.tab.inactive;
}

// ─── Disegno tab ─────────────────────────────────────────────────────────────

void ThemeRenderer::DrawTab(DrawingEngine* engine,
                             const BRect& tabRect,
                             bool focused, bool stackTile)
{
    const TabColorSet colors = _ResolveTabColors(focused, stackTile);
    const TabConfig&  tab    = fConfig.tab;

    // Bordo esterno del tab
    engine->StrokeLine(tabRect.LeftTop(),   tabRect.LeftBottom(),  colors.border_color);
    engine->StrokeLine(tabRect.LeftTop(),   tabRect.RightTop(),    colors.border_color);
    engine->StrokeLine(tabRect.RightTop(),  tabRect.RightBottom(), tint_color(colors.border_color, B_DARKEN_1_TINT));

    // Bevel interno (1px dentro il bordo)
    BRect inner = tabRect.InsetByCopy(1.0f, 1.0f);
    rgb_color bevel  = _Lighten(colors.color_start, 0.12f);
    rgb_color shadow = _Darken(colors.color_start, 0.08f);
    engine->StrokeLine(inner.LeftTop(),  inner.LeftBottom(),  bevel);
    engine->StrokeLine(inner.LeftTop(),  inner.RightTop(),    bevel);
    engine->StrokeLine(inner.RightTop(), inner.RightBottom(), shadow);

    // Riempimento
    _DrawTabShape(engine, tabRect, colors.color_start, focused);
    _ApplyEffect(engine, tabRect.InsetByCopy(2.0f, 2.0f), colors, focused);

    // Linea separatrice tab/contenuto
    if (tab.effect.separator_line) {
        engine->StrokeLine(
            BPoint(tabRect.left + 2,  tabRect.bottom + 1),
            BPoint(tabRect.right - 2, tabRect.bottom + 1),
            tab.effect.separator_color);
    }

    // Indicatore Stack & Tile
    if (stackTile) {
        rgb_color dot = fConfig.tab.stack_tile.indicator_color;
        float dotX = tabRect.right - 8.0f;
        float dotY = tabRect.top   + (tabRect.Height() / 2.0f);
        engine->FillEllipse(BRect(dotX - 2, dotY - 2, dotX + 2, dotY + 2), dot);
    }
}

void ThemeRenderer::_DrawTabShape(DrawingEngine* engine,
                                   const BRect& tabRect,
                                   rgb_color fillColor,
                                   bool /*focused*/)
{
    BRect fill = tabRect.InsetByCopy(2.0f, 2.0f);

    switch (fConfig.tab.shape) {
        case TAB_SHAPE_ROUNDED: {
            // Riempimento con angoli arrotondati (approssimazione con rettangolo)
            float r = (float)fConfig.tab.corner_radius;
            engine->FillRect(fill, fillColor);
            // Angoli arrotondati: disegna piccoli cerchi agli angoli
            if (r > 0) {
                engine->FillEllipse(BRect(fill.left,      fill.top,
                                          fill.left + r*2, fill.top + r*2), fillColor);
                engine->FillEllipse(BRect(fill.right-r*2, fill.top,
                                          fill.right,      fill.top + r*2), fillColor);
            }
            break;
        }
        case TAB_SHAPE_SLANTED: {
            // Tab trapezoidale: la geometria viene gestita tramite clipping
            // Approssimazione con rettangolo pieno (il clip è applicato a livello
            // di regione nel decorator padre)
            engine->FillRect(fill, fillColor);
            break;
        }
        case TAB_SHAPE_WAVE:
        case TAB_SHAPE_FLAT:
        default:
            engine->FillRect(fill, fillColor);
            break;
    }
}

void ThemeRenderer::_ApplyEffect(DrawingEngine* engine,
                                  const BRect& fillRect,
                                  const TabColorSet& colors,
                                  bool /*focused*/)
{
    const TabEffectConfig& fx = fConfig.tab.effect;

    switch (fx.type) {
        case EFFECT_GRADIENT: {
            BGradientLinear gradient;
            if (fx.gradient_dir == GRAD_VERTICAL) {
                gradient.SetStart(fillRect.LeftTop());
                gradient.SetEnd(fillRect.LeftBottom());
            } else if (fx.gradient_dir == GRAD_HORIZONTAL) {
                gradient.SetStart(fillRect.LeftTop());
                gradient.SetEnd(fillRect.RightTop());
            } else { // DIAGONAL
                gradient.SetStart(fillRect.LeftTop());
                gradient.SetEnd(fillRect.RightBottom());
            }
            gradient.AddColor(colors.color_start, 0);
            gradient.AddColor(colors.color_end,   255);
            engine->FillRect(fillRect, gradient);
            break;
        }
        case EFFECT_GLASS: {
            // Riempimento base + overlay chiaro nella metà superiore
            engine->FillRect(fillRect, colors.color_start);
            BRect glassRect = fillRect;
            glassRect.bottom = fillRect.top + fillRect.Height() * 0.5f;
            rgb_color glassColor = {255, 255, 255, (uint8)(fx.glass_opacity * 255)};
            engine->FillRect(glassRect, glassColor);
            break;
        }
        case EFFECT_TINT_FROM_SYSTEM:
            // Tint dal colore di sistema: il ThemeRenderer usa il colore già
            // risolto in _ResolveTabColors. Nessuna azione aggiuntiva.
            engine->FillRect(fillRect, colors.color_start);
            break;
        case EFFECT_TEXTURE:
            // La texture è gestita da TabPainter con bitmap pre-caricato.
            // Qui fallback a colore piatto.
            engine->FillRect(fillRect, colors.color_start);
            break;
        case EFFECT_NONE:
        default:
            engine->FillRect(fillRect, colors.color_start);
            break;
    }
}

// ─── Bordi ────────────────────────────────────────────────────────────────────

void ThemeRenderer::GetBorderColors(rgb_color colors[6], bool focused) const {
    const BorderColorSet& bc = focused
        ? fConfig.border.active
        : fConfig.border.inactive;

    rgb_color base = bc.color_base;

    // Array compatibile con il formato Haiku (6 colori per la cornice a 5 linee)
    colors[0] = bc.highlight_auto ? _Darken(base, 0.28f)  : bc.color_highlight;
    colors[1] = bc.highlight_auto ? _Lighten(base, 0.25f) : bc.color_highlight;
    colors[2] = bc.shadow_auto    ? _Darken(base, 0.03f)  : bc.color_shadow;
    colors[3] = bc.shadow_auto    ? _Darken(base, 0.34f)  : bc.color_shadow;
    colors[4] = colors[0];
    colors[5] = bc.shadow_auto    ? _Darken(base, 0.50f)  : bc.color_shadow;
}

void ThemeRenderer::DrawBorder(DrawingEngine* engine,
                                const BRect& outerRect, bool focused)
{
    rgb_color colors[6];
    GetBorderColors(colors, focused);

    int32 width = fConfig.border.width;

    switch (fConfig.border.style) {
        case BORDER_FLAT: {
            engine->StrokeRect(outerRect, colors[2]);
            break;
        }
        case BORDER_INSET: {
            // Bordo singolo con inversione highlight/shadow
            engine->StrokeLine(outerRect.LeftTop(),    outerRect.RightTop(),    colors[3]);
            engine->StrokeLine(outerRect.LeftTop(),    outerRect.LeftBottom(),  colors[3]);
            engine->StrokeLine(outerRect.RightTop(),   outerRect.RightBottom(), colors[1]);
            engine->StrokeLine(outerRect.LeftBottom(), outerRect.RightBottom(), colors[1]);
            break;
        }
        case BORDER_BEVELED:
        default: {
            int32 n = min(width, 5);
            BRect r = outerRect;
            for (int8 i = 0; i < n; i++) {
                engine->StrokeLine(BPoint(r.left,  r.top),    BPoint(r.right, r.top),    colors[i]);
                engine->StrokeLine(BPoint(r.left,  r.top),    BPoint(r.left,  r.bottom), colors[i]);
                engine->StrokeLine(BPoint(r.right, r.top),    BPoint(r.right, r.bottom),
                                   colors[(n-1-i) == (n-1) ? 5 : (n-1-i)]);
                engine->StrokeLine(BPoint(r.left,  r.bottom), BPoint(r.right, r.bottom),
                                   colors[(n-1-i) == (n-1) ? 5 : (n-1-i)]);
                r.InsetBy(1, 1);
            }
            break;
        }
    }
}

// ─── Pulsanti ─────────────────────────────────────────────────────────────────

void ThemeRenderer::_DrawButtonIcon(DrawingEngine* engine,
                                     const BRect& rect,
                                     bool isClose, bool pressed,
                                     rgb_color iconColor)
{
    float cx = rect.left + rect.Width()  / 2.0f;
    float cy = rect.top  + rect.Height() / 2.0f;
    float r  = rect.Width() * fConfig.buttons.icon.scale * 0.5f;
    float sw = fConfig.buttons.icon.stroke_width;

    // Offset verso destra/basso se premuto (press feedback)
    if (pressed) { cx += 0.5f; cy += 0.5f; }

    switch (fConfig.buttons.icon.style) {
        case ICON_DOT:
            engine->FillEllipse(BRect(cx - r*0.3f, cy - r*0.3f,
                                      cx + r*0.3f, cy + r*0.3f), iconColor);
            break;

        case ICON_CROSS_X:
        case ICON_SYMBOL:
            if (isClose) {
                // X
                engine->StrokeLine(BPoint(cx - r, cy - r), BPoint(cx + r, cy + r), iconColor);
                engine->StrokeLine(BPoint(cx + r, cy - r), BPoint(cx - r, cy + r), iconColor);
            } else {
                // Quadrato (zoom/maximize)
                BRect box(cx - r, cy - r, cx + r, cy + r);
                engine->StrokeRect(box, iconColor);
            }
            (void)sw;
            break;

        case ICON_CLASSIC_BE:
            // Stile R4: X per close, linee per zoom
            if (isClose) {
                engine->StrokeLine(BPoint(cx - r, cy - r), BPoint(cx + r, cy + r), iconColor);
                engine->StrokeLine(BPoint(cx + r, cy - r), BPoint(cx - r, cy + r), iconColor);
            } else {
                engine->StrokeLine(BPoint(cx - r, cy),     BPoint(cx + r, cy),     iconColor);
                engine->StrokeLine(BPoint(cx,     cy - r), BPoint(cx,     cy + r), iconColor);
            }
            break;

        case ICON_NONE:
        default:
            break;
    }
}

ServerBitmap* ThemeRenderer::CreateButtonBitmap(bool isClose,
                                                  bool pressed,
                                                  bool focused,
                                                  int32 width,
                                                  int32 height)
{
    static BitmapDrawingEngine* sEngine = nullptr;
    if (sEngine == nullptr)
        sEngine = new (std::nothrow) BitmapDrawingEngine();
    if (sEngine == nullptr || sEngine->SetSize(width, height) != B_OK)
        return nullptr;

    BRect rect(0, 0, width - 1, height - 1);

    const ButtonColorSet& bcs = isClose
        ? fConfig.buttons.close
        : fConfig.buttons.zoom;

    // Scegli il colore in base allo stato
    rgb_color bg;
    if (pressed)
        bg = bcs.color_pressed;
    else if (focused)
        bg = bcs.color_normal;
    else
        bg = _Darken(bcs.color_normal, 0.15f);

    // Hover effect pre-baked nel bitmap "normal" (non premuto)
    if (!pressed && fConfig.buttons.hover_effect.type == HOVER_LIGHTEN)
        bg = _Lighten(bg, 0.06f);
    else if (!pressed && fConfig.buttons.hover_effect.type == HOVER_DARKEN)
        bg = _Darken(bg, 0.06f);

    // ─── Disegno forma pulsante ───────────────────────────────────────────────
    sEngine->FillRect(rect, bg);

    switch (fConfig.buttons.shape) {
        case BTN_CIRCLE:
            sEngine->FillEllipse(rect, bg);
            sEngine->StrokeEllipse(rect, _Darken(bg, 0.25f));
            break;

        case BTN_ROUNDED_SQUARE: {
            rgb_color highlight = _Lighten(bg, 0.20f);
            rgb_color shadow    = _Darken(bg, 0.25f);
            // Bevel
            sEngine->StrokeLine(rect.LeftTop(),    rect.RightTop(),    highlight);
            sEngine->StrokeLine(rect.LeftTop(),    rect.LeftBottom(),  highlight);
            sEngine->StrokeLine(rect.RightTop(),   rect.RightBottom(), shadow);
            sEngine->StrokeLine(rect.LeftBottom(), rect.RightBottom(), shadow);
            break;
        }

        case BTN_DIAMOND: {
            float cx = width  / 2.0f;
            float cy = height / 2.0f;
            BPoint pts[4] = {
                BPoint(cx,       rect.top),
                BPoint(rect.right, cy),
                BPoint(cx,       rect.bottom),
                BPoint(rect.left,  cy)
            };
            BRegion region;
            // Disegno come 4 triangoli
            sEngine->FillRect(rect, bg);
            break;
        }

        case BTN_SQUARE:
        default:
            sEngine->StrokeRect(rect, _Darken(bg, 0.25f));
            break;
    }

    // ─── Icona ────────────────────────────────────────────────────────────────
    _DrawButtonIcon(sEngine, rect.InsetByCopy(2.0f, 2.0f),
                   isClose, pressed, bcs.icon_color);

    return sEngine->ExportToBitmap(width, height, B_RGBA32);
}

// ─── Resize corner ────────────────────────────────────────────────────────────

void ThemeRenderer::DrawResizeCorner(DrawingEngine* engine,
                                      const BRect& rect, bool focused)
{
    if (fConfig.resize_corner.style == RESIZE_NONE) return;

    rgb_color c = fConfig.resize_corner.color;
    rgb_color light = _Lighten(c, 0.20f);

    float x = rect.right - 3.0f;
    float y = rect.bottom - 3.0f;

    if (fConfig.resize_corner.style == RESIZE_KNOB) {
        // Punti a griglia 3x3 stile classico Haiku
        for (int8 i = 1; i <= 3; i++) {
            for (int8 j = 1; j <= i; j++) {
                BPoint p1(x - 3.0f * j + 1, y - 3.0f * (4 - i) + 1);
                BPoint p2(x - 3.0f * j + 2, y - 3.0f * (4 - i) + 2);
                engine->StrokePoint(p1, c);
                engine->StrokePoint(p2, light);
            }
        }
    } else { // RESIZE_LINES
        engine->StrokeLine(
            BPoint(rect.left,  rect.bottom - fConfig.resize_corner.size),
            BPoint(rect.right, rect.bottom - fConfig.resize_corner.size), c);
        engine->StrokeLine(
            BPoint(rect.right - fConfig.resize_corner.size, rect.top),
            BPoint(rect.right - fConfig.resize_corner.size, rect.bottom), c);
    }
    (void)focused;
}
