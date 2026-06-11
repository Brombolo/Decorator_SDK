/*
 * Haiku Decorator SDK — SDKDecorator.cpp
 */

#include "SDKDecorator.h"
#include "ConfigReader.h"
#include "TabPainter.h"

#include <new>
#include <stdio.h>
#include <algorithm>
#include <libgen.h>

#include <InterfaceDefs.h>
#include <GradientLinear.h>
#include <WindowPrivate.h>

using std::min;
using std::max;

// ─── Helper: colore piatto come BGradientLinear ────────────────────────────
// DrawingEngine non ha FillEllipse(BRect, rgb_color).
// Per disegnare ellissi con colore piatto usiamo un gradiente degenere
// (stesso colore a start e end).
static BGradientLinear _SolidGradient(BRect r, rgb_color c) {
    BGradientLinear g;
    g.SetStart(r.LeftTop());
    g.SetEnd(r.LeftBottom());
    g.AddColor(c, 0);
    g.AddColor(c, 255);
    return g;
}

// ─── Entry point ──────────────────────────────────────────────────────────────

extern "C" DecorAddOn* instantiate_decor_addon(image_id id, const char* name) {
    return new (std::nothrow) SDKDecorAddOn(id, name);
}

SDKDecorAddOn::SDKDecorAddOn(image_id id, const char* name)
    : DecorAddOn(id, name) {}

Decorator* SDKDecorAddOn::_AllocateDecorator(DesktopSettings& settings,
                                               BRect rect, Desktop* desktop)
{
    ThemeConfig config;
    status_t err = ConfigReader::Load(THEME_CONF_PATH, config);
    if (err != B_OK)
        fprintf(stderr, "[SDKDecorator] Errore theme.conf: %s\n",
                ConfigReader::LastError());
    return new (std::nothrow) SDKDecorator(settings, rect, desktop, config);
}

// ─── SDKDecorator ─────────────────────────────────────────────────────────────

SDKDecorator::SDKDecorator(DesktopSettings& settings, BRect rect,
                             Desktop* desktop, const ThemeConfig& config)
    : SATDecorator(settings, rect, desktop),
      fConfig(config),
      fRenderer(fConfig),
      fTabPainter(nullptr)
{
    if (fConfig.tab.effect.type == EFFECT_TEXTURE
        && !fConfig.tab.texture.file.IsEmpty())
    {
        char confPath[] = THEME_CONF_PATH;
        fTabPainter = new (std::nothrow) TabPainter(fConfig.tab.texture,
                                                     dirname(confPath));
        if (fTabPainter && !fTabPainter->IsValid()) {
            delete fTabPainter;
            fTabPainter = nullptr;
        }
    }
}

SDKDecorator::~SDKDecorator() {
    delete fTabPainter;
}

// ─── GetComponentColors ───────────────────────────────────────────────────────

void SDKDecorator::GetComponentColors(Component component, uint8 highlight,
                                       ComponentColors _colors,
                                       Decorator::Tab* _tab)
{
    Decorator::Tab* tab = static_cast<Decorator::Tab*>(_tab);
    bool focused   = tab && tab->buttonFocus;
    bool stackTile = (highlight == HIGHLIGHT_STACK_AND_TILE);

    switch (component) {
        case COMPONENT_TAB: {
            const TabColorSet& cs = stackTile
                ? (TabColorSet){
                    fConfig.tab.stack_tile.color_start,
                    fConfig.tab.stack_tile.color_end,
                    fConfig.tab.stack_tile.text_color,
                    fConfig.tab.active.border_color }
                : (focused ? fConfig.tab.active : fConfig.tab.inactive);

            rgb_color base = focused ? fFocusFrameColor : fNonFocusFrameColor;
            _colors[COLOR_TAB_FRAME_LIGHT] = tint_color(base, B_DARKEN_2_TINT);
            _colors[COLOR_TAB_FRAME_DARK]  = tint_color(base, B_DARKEN_3_TINT);
            _colors[COLOR_TAB]        = cs.color_start;
            _colors[COLOR_TAB_LIGHT]  = tint_color(cs.color_start, B_LIGHTEN_1_TINT);
            _colors[COLOR_TAB_BEVEL]  = tint_color(cs.color_start, B_LIGHTEN_2_TINT);
            _colors[COLOR_TAB_SHADOW] = tint_color(cs.color_start, B_DARKEN_2_TINT);
            _colors[COLOR_TAB_TEXT]   = cs.text_color;
            break;
        }
        case COMPONENT_CLOSE_BUTTON:
        case COMPONENT_ZOOM_BUTTON: {
            const ButtonColorSet& bcs = (component == COMPONENT_CLOSE_BUTTON)
                ? fConfig.buttons.close : fConfig.buttons.zoom;
            _colors[COLOR_BUTTON] = focused
                ? bcs.color_normal
                : ThemeRenderer::Darken(bcs.color_normal, 0.15f);
            _colors[COLOR_BUTTON_LIGHT] =
                ThemeRenderer::Lighten(bcs.color_normal, 0.15f);
            break;
        }
        default: {
            rgb_color bc[6];
            fRenderer.CalcBorderColors(bc, focused || stackTile);
            for (int i = 0; i < 6; i++) _colors[i] = bc[i];
            break;
        }
    }
}

// ─── _DrawFrame ───────────────────────────────────────────────────────────────

void SDKDecorator::_DrawFrame(BRect invalid) {
    if (fTopTab->look == B_NO_BORDER_WINDOW_LOOK) return;
    if (fBorderWidth <= 0) return;
    bool focused = IsFocus(fTopTab);
    if (invalid.Intersects(fTopBorder))
        _PaintBorderEdge(fTopBorder, focused, true, true);
    if (invalid.Intersects(fBottomBorder))
        _PaintBorderEdge(fBottomBorder, focused, true, false);
    if (invalid.Intersects(fLeftBorder))
        _PaintBorderEdge(fLeftBorder, focused, false, true);
    if (invalid.Intersects(fRightBorder))
        _PaintBorderEdge(fRightBorder, focused, false, false);
    if (!(fTopTab->flags & B_NOT_RESIZABLE))
        _PaintResizeCorner(fResizeRect, focused);
}

void SDKDecorator::_PaintBorderEdge(const BRect& rect, bool focused,
                                     bool, bool)
{
    rgb_color colors[6];
    fRenderer.CalcBorderColors(colors, focused);
    int32 n = min((int32)fBorderWidth, (int32)5);
    BRect r = rect;
    switch (fConfig.border.style) {
        case BORDER_FLAT:
            fDrawingEngine->FillRect(r, colors[2]);
            break;
        case BORDER_INSET:
            fDrawingEngine->FillRect(r, colors[2]);
            fDrawingEngine->StrokeLine(r.LeftTop(),    r.RightTop(),    colors[3]);
            fDrawingEngine->StrokeLine(r.LeftTop(),    r.LeftBottom(),  colors[3]);
            fDrawingEngine->StrokeLine(r.RightTop(),   r.RightBottom(), colors[1]);
            fDrawingEngine->StrokeLine(r.LeftBottom(), r.RightBottom(), colors[1]);
            break;
        case BORDER_BEVELED:
        default:
            for (int8 i = 0; i < n; i++) {
                fDrawingEngine->StrokeLine(BPoint(r.left,  r.top),
                                            BPoint(r.right, r.top),    colors[i]);
                fDrawingEngine->StrokeLine(BPoint(r.left,  r.top),
                                            BPoint(r.left,  r.bottom), colors[i]);
                rgb_color sc = (n-1-i >= 3) ? colors[5] : colors[3];
                fDrawingEngine->StrokeLine(BPoint(r.right, r.top),
                                            BPoint(r.right, r.bottom), sc);
                fDrawingEngine->StrokeLine(BPoint(r.left,  r.bottom),
                                            BPoint(r.right, r.bottom), sc);
                r.InsetBy(1, 1);
            }
            break;
    }
}

void SDKDecorator::_PaintResizeCorner(const BRect& rect, bool) {
    if (fConfig.resize_corner.style == RESIZE_NONE) return;
    rgb_color c     = fConfig.resize_corner.color;
    rgb_color light = ThemeRenderer::Lighten(c, 0.20f);
    if (fConfig.resize_corner.style == RESIZE_LINES) {
        int32 sz = fConfig.resize_corner.size;
        fDrawingEngine->StrokeLine(BPoint(rect.right-sz, rect.bottom),
                                    BPoint(rect.right,    rect.bottom), c);
        fDrawingEngine->StrokeLine(BPoint(rect.right, rect.bottom-sz),
                                    BPoint(rect.right, rect.bottom),    c);
        return;
    }
    float x = rect.right - 2.0f, y = rect.bottom - 2.0f;
    for (int8 i = 1; i <= 3; i++)
        for (int8 j = 1; j <= i; j++) {
            fDrawingEngine->StrokePoint(
                BPoint(x - 3.0f*j,        y - 3.0f*(4-i)),        c);
            fDrawingEngine->StrokePoint(
                BPoint(x - 3.0f*j + 1.0f, y - 3.0f*(4-i) + 1.0f), light);
        }
}

// ─── _DrawTab ─────────────────────────────────────────────────────────────────

void SDKDecorator::_DrawTab(Decorator::Tab* _tab, BRect invalid) {
    Decorator::Tab* tab = static_cast<Decorator::Tab*>(_tab);
    const BRect& tabRect = tab->tabRect;
    if (!tabRect.IsValid() || !invalid.Intersects(tabRect)) return;

    bool focused   = tab->buttonFocus;
    // Stack&Tile: il decorator riceve highlight come parametro in _DrawTab.
    // Usiamo isHighlighted come proxy — il valore esatto è gestito da
    // GetComponentColors che riceve uint8 highlight.
    bool stackTile = tab->isHighlighted;

    _PaintTab(tab, tabRect, focused, stackTile);
    _DrawTitle(tab, tabRect);
    if (tab->closeRect.IsValid()) _DrawClose(tab, true, tab->closeRect);
    if (tab->zoomRect.IsValid())  _DrawZoom(tab,  true, tab->zoomRect);
}

void SDKDecorator::_PaintTab(Decorator::Tab*, const BRect& tabRect,
                               bool focused, bool stackTile)
{
    TabDrawSpec spec = fRenderer.CalcTab(tabRect, focused, stackTile);
    BRect fill = tabRect.InsetByCopy(2.0f, 2.0f);

    // Bordi
    fDrawingEngine->StrokeLine(tabRect.LeftTop(),  tabRect.LeftBottom(),
                                spec.border_outer);
    fDrawingEngine->StrokeLine(tabRect.LeftTop(),  tabRect.RightTop(),
                                spec.border_outer);
    fDrawingEngine->StrokeLine(tabRect.RightTop(), tabRect.RightBottom(),
                                spec.border_outer_shadow);
    BRect inner = tabRect.InsetByCopy(1.0f, 1.0f);
    fDrawingEngine->StrokeLine(inner.LeftTop(),  inner.LeftBottom(),  spec.bevel_light);
    fDrawingEngine->StrokeLine(inner.LeftTop(),  inner.RightTop(),    spec.bevel_light);
    fDrawingEngine->StrokeLine(inner.RightTop(), inner.RightBottom(), spec.bevel_shadow);

    // Fill
    if (spec.gradient.active) {
        BGradientLinear gradient;
        gradient.SetStart(BPoint(spec.gradient.start_x, spec.gradient.start_y));
        gradient.SetEnd(BPoint(spec.gradient.end_x,     spec.gradient.end_y));
        gradient.AddColor(spec.gradient.color_start, 0);
        gradient.AddColor(spec.gradient.color_end,   255);
        fDrawingEngine->FillRect(fill, gradient);
    } else {
        fDrawingEngine->FillRect(fill, spec.fill_color);
    }

    // Texture (BBitmap→ non supportato da DrawingEngine, skip per ora)
    // DrawingEngine::DrawBitmap richiede ServerBitmap* — TabPainter
    // sarà integrato in una versione futura tramite ServerBitmap.

    // Glass
    if (spec.glass_enabled) {
        drawing_mode old;
        fDrawingEngine->SetDrawingMode(B_OP_ALPHA, old);
        fDrawingEngine->FillRect(spec.glass_rect, spec.glass_color);
        fDrawingEngine->SetDrawingMode(old);
    }

    // Inner glow
    if (spec.inner_glow_enabled)
        fDrawingEngine->StrokeLine(BPoint(fill.left, fill.top),
                                    BPoint(fill.right, fill.top),
                                    spec.inner_glow_color);

    // Separatore
    if (spec.separator_enabled)
        fDrawingEngine->StrokeLine(
            BPoint(tabRect.left+2,  tabRect.bottom+1),
            BPoint(tabRect.right-2, tabRect.bottom+1),
            spec.separator_color);

    // Stack&Tile indicator: piccolo rettangolo (FillEllipse non accetta rgb_color)
    if (spec.stack_indicator)
        fDrawingEngine->FillRect(spec.indicator_rect, spec.indicator_color);
}

// ─── _DrawTitle ───────────────────────────────────────────────────────────────

void SDKDecorator::_DrawTitle(Decorator::Tab* _tab, BRect) {
    Decorator::Tab* tab = static_cast<Decorator::Tab*>(_tab);
    const BRect& tabRect   = tab->tabRect;
    const BRect& closeRect = tab->closeRect;
    const BRect& zoomRect  = tab->zoomRect;

    // highlight=0 per finestre non in Stack&Tile
    uint8 hl = tab->isHighlighted ? HIGHLIGHT_STACK_AND_TILE : 0;
    ComponentColors colors;
    GetComponentColors(COMPONENT_TAB, hl, colors, tab);

    fDrawingEngine->SetDrawingMode(B_OP_OVER);
    fDrawingEngine->SetHighColor(colors[COLOR_TAB_TEXT]);
    fDrawingEngine->SetLowColor(colors[COLOR_TAB]);
    fDrawingEngine->SetFont(fDrawState.Font());

    font_height fh;
    fDrawState.Font().GetHeight(fh);

    float startX = closeRect.IsValid()
        ? closeRect.right + tab->textOffset
        : tabRect.left    + tab->textOffset;
    float titleWidth = fDrawState.Font().StringWidth(
        tab->truncatedTitle.String(), tab->truncatedTitleLength);

    float titleX = startX;
    switch (fConfig.title.alignment) {
        case TITLE_CENTER: {
            float endX = zoomRect.IsValid() ? zoomRect.left : tabRect.right;
            titleX = startX + (endX - startX - titleWidth) * 0.5f;
            break;
        }
        case TITLE_RIGHT: {
            float endX = zoomRect.IsValid()
                ? zoomRect.left - tab->textOffset
                : tabRect.right - tab->textOffset;
            titleX = endX - titleWidth;
            break;
        }
        default: break;
    }

    float titleY = floorf(
        ((tabRect.top + 2.0f) + tabRect.bottom + fh.ascent + fh.descent)
        / 2.0f - fh.descent + 0.5f);

    if (fConfig.title.shadow) {
        fDrawingEngine->SetHighColor(fConfig.title.shadow_color);
        fDrawingEngine->DrawString(tab->truncatedTitle.String(),
                                    tab->truncatedTitleLength,
                                    BPoint(titleX+1, titleY+1));
        fDrawingEngine->SetHighColor(colors[COLOR_TAB_TEXT]);
    }
    fDrawingEngine->DrawString(tab->truncatedTitle.String(),
                                tab->truncatedTitleLength,
                                BPoint(titleX, titleY));
    fDrawingEngine->SetDrawingMode(B_OP_COPY);
}

// ─── Pulsanti ─────────────────────────────────────────────────────────────────

void SDKDecorator::_PaintButton(const BRect& rect, const ButtonDrawSpec& spec) {
    fDrawingEngine->FillRect(rect, spec.bg_color);
    switch (spec.shape) {
        case BTN_CIRCLE: {
            // Cerchio approssimato: FillRect interno + bordo con StrokeLine
            // DrawingEngine non ha FillEllipse(BRect, rgb_color)
            BRect inner = rect.InsetByCopy(1.0f, 1.0f);
            fDrawingEngine->FillRect(inner, spec.bg_color);
            // Bordo simulato con 4 linee
            fDrawingEngine->StrokeLine(rect.LeftTop(),    rect.RightTop(),    spec.highlight);
            fDrawingEngine->StrokeLine(rect.LeftTop(),    rect.LeftBottom(),  spec.highlight);
            fDrawingEngine->StrokeLine(rect.RightTop(),   rect.RightBottom(), spec.shadow);
            fDrawingEngine->StrokeLine(rect.LeftBottom(), rect.RightBottom(), spec.shadow);
            break;
        }
        case BTN_ROUNDED_SQUARE:
        case BTN_SQUARE:
        default:
            fDrawingEngine->StrokeLine(rect.LeftTop(),    rect.RightTop(),    spec.highlight);
            fDrawingEngine->StrokeLine(rect.LeftTop(),    rect.LeftBottom(),  spec.highlight);
            fDrawingEngine->StrokeLine(rect.RightTop(),   rect.RightBottom(), spec.shadow);
            fDrawingEngine->StrokeLine(rect.LeftBottom(), rect.RightBottom(), spec.shadow);
            break;
        case BTN_DIAMOND:
            fDrawingEngine->StrokeLine(rect.LeftTop(),  rect.RightBottom(), spec.shadow);
            fDrawingEngine->StrokeLine(rect.RightTop(), rect.LeftBottom(),  spec.shadow);
            break;
    }
    _PaintButtonIcon(rect.InsetByCopy(2.0f, 2.0f), spec);
}

void SDKDecorator::_PaintButtonIcon(const BRect& rect, const ButtonDrawSpec& spec) {
    if (spec.icon_style == ICON_NONE) return;
    float cx = rect.left + rect.Width()  / 2.0f;
    float cy = rect.top  + rect.Height() / 2.0f;
    float r  = (min(rect.Width(), rect.Height()) * spec.icon_scale) * 0.5f;
    if (spec.pressed) { cx += 0.5f; cy += 0.5f; }
    rgb_color ic = spec.icon_color;

    switch (spec.icon_style) {
        case ICON_DOT:
            // Piccolo rettangolo al posto del cerchio (DrawingEngine limita)
            fDrawingEngine->FillRect(
                BRect(cx-r*0.35f, cy-r*0.35f, cx+r*0.35f, cy+r*0.35f), ic);
            break;
        case ICON_CLASSIC_BE:
            if (spec.is_close) {
                fDrawingEngine->StrokeLine(BPoint(cx-r,cy-r),BPoint(cx+r,cy+r),ic);
                fDrawingEngine->StrokeLine(BPoint(cx+r,cy-r),BPoint(cx-r,cy+r),ic);
            } else {
                fDrawingEngine->StrokeLine(BPoint(cx-r,cy),  BPoint(cx+r,cy),  ic);
                fDrawingEngine->StrokeLine(BPoint(cx,  cy-r),BPoint(cx,  cy+r),ic);
            }
            break;
        case ICON_CROSS_X:
            fDrawingEngine->StrokeLine(BPoint(cx-r,cy-r),BPoint(cx+r,cy+r),ic);
            fDrawingEngine->StrokeLine(BPoint(cx+r,cy-r),BPoint(cx-r,cy+r),ic);
            break;
        case ICON_SYMBOL:
        default:
            if (spec.is_close) {
                fDrawingEngine->StrokeLine(BPoint(cx-r,cy-r),BPoint(cx+r,cy+r),ic);
                fDrawingEngine->StrokeLine(BPoint(cx+r,cy-r),BPoint(cx-r,cy+r),ic);
            } else {
                fDrawingEngine->StrokeRect(BRect(cx-r,cy-r,cx+r,cy+r),ic);
            }
            break;
    }
}

void SDKDecorator::_DrawClose(Decorator::Tab* _tab, bool direct, BRect rect) {
    Decorator::Tab* tab = static_cast<Decorator::Tab*>(_tab);
    bool prev = fDrawingEngine->CopyToFrontEnabled();
    fDrawingEngine->SetCopyToFrontEnabled(direct);
    _PaintButton(rect, fRenderer.CalcButton(true,  tab->closePressed, tab->buttonFocus));
    fDrawingEngine->SetCopyToFrontEnabled(prev);
}

void SDKDecorator::_DrawZoom(Decorator::Tab* _tab, bool direct, BRect rect) {
    if (rect.IntegerWidth() < 1) return;
    Decorator::Tab* tab = static_cast<Decorator::Tab*>(_tab);
    bool prev = fDrawingEngine->CopyToFrontEnabled();
    fDrawingEngine->SetCopyToFrontEnabled(direct);
    _PaintButton(rect, fRenderer.CalcButton(false, tab->zoomPressed,  tab->buttonFocus));
    fDrawingEngine->SetCopyToFrontEnabled(prev);
}

void SDKDecorator::_DrawMinimize(Decorator::Tab*, bool, BRect) {}

void SDKDecorator::_GetButtonSizeAndOffset(const BRect& tabRect,
                                            float* offset, float* size,
                                            float* inset) const
{
    *offset = (float)fConfig.buttons.margin;
    *inset  = 0.0f;
    *size   = tabRect.Height() - (float)fConfig.buttons.margin * 2.0f;
    if (*size > (float)fConfig.buttons.size) *size = (float)fConfig.buttons.size;
    if (*size < 0) *size = 0;
}
