/*
 * Haiku Decorator SDK — SDKDecorator.cpp
 * Classe principale del decorator. Collega ThemeConfig e ThemeRenderer
 * all'API app_server di Haiku.
 *
 * Al momento della compilazione, THEME_CONF_PATH viene definito da generate.sh
 * con il percorso assoluto di theme.conf. Il file viene letto UNA SOLA VOLTA
 * al momento dell'istanziazione del DecorAddOn.
 */

#include "SDKDecorator.h"
#include "ConfigReader.h"

#include <new>
#include <stdio.h>
#include <WindowPrivate.h>
#include <Autolock.h>

// Haiku internals
#include "Desktop.h"
#include "DesktopSettings.h"
#include "DrawingEngine.h"
#include "DrawState.h"
#include "FontManager.h"
#include "RGBColor.h"
#include "ServerBitmap.h"

#ifndef THEME_CONF_PATH
#define THEME_CONF_PATH "theme.conf"
#endif

// ─── Entry point ──────────────────────────────────────────────────────────────

extern "C" DecorAddOn* instantiate_decor_addon(image_id id, const char* name) {
    return new (std::nothrow) SDKDecorAddOn(id, name);
}

// ─── SDKDecorAddOn ────────────────────────────────────────────────────────────

SDKDecorAddOn::SDKDecorAddOn(image_id id, const char* name)
    : DecorAddOn(id, name)
{
}

Decorator* SDKDecorAddOn::_AllocateDecorator(DesktopSettings& settings,
                                               BRect rect, Desktop* desktop)
{
    ThemeConfig config;
    status_t err = ConfigReader::Load(THEME_CONF_PATH, config);
    if (err != B_OK) {
        fprintf(stderr, "[SDKDecorator] Errore caricamento theme.conf: %s\n",
                ConfigReader::LastError());
        // Ritorna comunque un decorator con valori di default
    }
    return new (std::nothrow) SDKDecorator(settings, rect, desktop, config);
}

// ─── SDKDecorator ─────────────────────────────────────────────────────────────

SDKDecorator::SDKDecorator(DesktopSettings& settings,
                             BRect rect,
                             Desktop* desktop,
                             const ThemeConfig& config)
    : SATDecorator(settings, rect, desktop),
      fConfig(config),
      fRenderer(fConfig)
{
}

SDKDecorator::~SDKDecorator() {
}

// ─── GetComponentColors ───────────────────────────────────────────────────────

void SDKDecorator::GetComponentColors(Component component,
                                       uint8 highlight,
                                       ComponentColors _colors,
                                       Decorator::Tab* _tab)
{
    Decorator::Tab* tab = static_cast<Decorator::Tab*>(_tab);
    bool focused    = tab && tab->buttonFocus;
    bool stackTile  = (highlight == HIGHLIGHT_STACK_AND_TILE);

    switch (component) {
        case COMPONENT_TAB: {
            const TabColorSet& cs = stackTile
                ? (TabColorSet){ fConfig.tab.stack_tile.color_start,
                                 fConfig.tab.stack_tile.color_end,
                                 fConfig.tab.stack_tile.text_color,
                                 fConfig.tab.active.border_color }
                : (focused ? fConfig.tab.active : fConfig.tab.inactive);

            _colors[COLOR_TAB_FRAME_LIGHT] = tint_color(
                focused ? fFocusFrameColor : fNonFocusFrameColor, B_DARKEN_2_TINT);
            _colors[COLOR_TAB_FRAME_DARK]  = tint_color(
                focused ? fFocusFrameColor : fNonFocusFrameColor, B_DARKEN_3_TINT);
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
                ? fConfig.buttons.close
                : fConfig.buttons.zoom;
            _colors[COLOR_BUTTON]       = focused ? bcs.color_normal
                                                  : tint_color(bcs.color_normal, B_DARKEN_1_TINT);
            _colors[COLOR_BUTTON_LIGHT] = tint_color(bcs.color_normal, B_LIGHTEN_1_TINT);
            break;
        }

        case COMPONENT_LEFT_BORDER:
        case COMPONENT_RIGHT_BORDER:
        case COMPONENT_TOP_BORDER:
        case COMPONENT_BOTTOM_BORDER:
        case COMPONENT_RESIZE_CORNER:
        default:
            fRenderer.GetBorderColors(_colors, focused || stackTile);
            break;
    }
}

// ─── _DrawFrame ───────────────────────────────────────────────────────────────

void SDKDecorator::_DrawFrame(BRect invalid) {
    if (fTopTab->look == B_NO_BORDER_WINDOW_LOOK) return;
    if (fBorderWidth <= 0) return;

    BRect r = BRect(fTopBorder.LeftTop(), fBottomBorder.RightBottom());
    bool  focused = IsFocus(fTopTab);

    ComponentColors colors;

    // Top
    if (invalid.Intersects(fTopBorder)) {
        _GetComponentColors(COMPONENT_TOP_BORDER, colors, fTopTab);
        int32 n = min(fBorderWidth, 5);
        for (int8 i = 0; i < n; i++)
            fDrawingEngine->StrokeLine(
                BPoint(r.left + i, r.top + i),
                BPoint(r.right - i, r.top + i), colors[i]);
    }
    // Left
    if (invalid.Intersects(fLeftBorder.InsetByCopy(0, -fBorderWidth))) {
        _GetComponentColors(COMPONENT_LEFT_BORDER, colors, fTopTab);
        int32 n = min(fBorderWidth, 5);
        for (int8 i = 0; i < n; i++)
            fDrawingEngine->StrokeLine(
                BPoint(r.left + i, r.top + i),
                BPoint(r.left + i, r.bottom - i), colors[i]);
    }
    // Bottom
    if (invalid.Intersects(fBottomBorder)) {
        _GetComponentColors(COMPONENT_BOTTOM_BORDER, colors, fTopTab);
        int32 n = min(fBorderWidth, 5);
        for (int8 i = 0; i < n; i++)
            fDrawingEngine->StrokeLine(
                BPoint(r.left + i, r.bottom - i),
                BPoint(r.right - i, r.bottom - i),
                colors[(n-1-i) == (n-1) ? 5 : (n-1-i)]);
    }
    // Right
    if (invalid.Intersects(fRightBorder.InsetByCopy(0, -fBorderWidth))) {
        _GetComponentColors(COMPONENT_RIGHT_BORDER, colors, fTopTab);
        int32 n = min(fBorderWidth, 5);
        for (int8 i = 0; i < n; i++)
            fDrawingEngine->StrokeLine(
                BPoint(r.right - i, r.top + i),
                BPoint(r.right - i, r.bottom - i),
                colors[(n-1-i) == (n-1) ? 5 : (n-1-i)]);
    }

    // Resize corner
    if (!(fTopTab->flags & B_NOT_RESIZABLE)) {
        fRenderer.DrawResizeCorner(fDrawingEngine, fResizeRect, focused);
    }
}

// ─── _DrawTab ─────────────────────────────────────────────────────────────────

void SDKDecorator::_DrawTab(Decorator::Tab* _tab, BRect invalid) {
    Decorator::Tab* tab = static_cast<Decorator::Tab*>(_tab);
    const BRect& tabRect = tab->tabRect;

    if (!tabRect.IsValid() || !invalid.Intersects(tabRect)) return;

    bool focused   = tab->buttonFocus;
    bool stackTile = false; // TODO: rilevare highlight STACK_AND_TILE

    fRenderer.DrawTab(fDrawingEngine, tabRect, focused, stackTile);
    _DrawTitle(tab, tabRect);
    _DrawButtons(tab, invalid);
}

// ─── _DrawTitle ───────────────────────────────────────────────────────────────

void SDKDecorator::_DrawTitle(Decorator::Tab* _tab, BRect r) {
    Decorator::Tab* tab = static_cast<Decorator::Tab*>(_tab);
    const BRect& tabRect  = tab->tabRect;
    const BRect& closeRect = tab->closeRect;
    const BRect& zoomRect  = tab->zoomRect;

    ComponentColors colors;
    _GetComponentColors(COMPONENT_TAB, colors, tab);

    fDrawingEngine->SetDrawingMode(B_OP_OVER);
    fDrawingEngine->SetHighColor(colors[COLOR_TAB_TEXT]);
    fDrawingEngine->SetLowColor(colors[COLOR_TAB]);
    fDrawingEngine->SetFont(fDrawState.Font());

    font_height fh;
    fDrawState.Font().GetHeight(fh);

    BPoint titlePos;
    titlePos.x = closeRect.IsValid()
        ? closeRect.right + tab->textOffset
        : tabRect.left   + tab->textOffset;

    // Allineamento titolo
    switch (fConfig.title.alignment) {
        case TITLE_CENTER: {
            float available = zoomRect.IsValid()
                ? zoomRect.left - titlePos.x
                : tabRect.right - titlePos.x;
            titlePos.x += (available - fDrawState.Font().StringWidth(
                               tab->truncatedTitle.String(),
                               tab->truncatedTitleLength)) / 2.0f;
            break;
        }
        case TITLE_RIGHT:
            titlePos.x = zoomRect.IsValid()
                ? zoomRect.left - tab->textOffset
                  - fDrawState.Font().StringWidth(tab->truncatedTitle.String(),
                                                   tab->truncatedTitleLength)
                : tabRect.right - tab->textOffset
                  - fDrawState.Font().StringWidth(tab->truncatedTitle.String(),
                                                   tab->truncatedTitleLength);
            break;
        case TITLE_LEFT:
        default:
            break;
    }

    titlePos.y = floorf(((tabRect.top + 2.0f) + tabRect.bottom
                          + fh.ascent + fh.descent) / 2.0f
                         - fh.descent + 0.5f);

    // Ombra testo (opzionale)
    if (fConfig.title.shadow) {
        fDrawingEngine->SetHighColor(fConfig.title.shadow_color);
        BPoint shadowPos = titlePos;
        shadowPos.x += 1; shadowPos.y += 1;
        fDrawingEngine->DrawString(tab->truncatedTitle.String(),
                                    tab->truncatedTitleLength, shadowPos);
        fDrawingEngine->SetHighColor(colors[COLOR_TAB_TEXT]);
    }

    fDrawingEngine->DrawString(tab->truncatedTitle.String(),
                                tab->truncatedTitleLength, titlePos);
    fDrawingEngine->SetDrawingMode(B_OP_COPY);
}

// ─── Pulsanti ─────────────────────────────────────────────────────────────────

void SDKDecorator::_DrawButtonBitmap(ServerBitmap* bitmap, bool direct, BRect rect) {
    if (!bitmap) return;
    bool prev = fDrawingEngine->CopyToFrontEnabled();
    fDrawingEngine->SetCopyToFrontEnabled(direct);
    drawing_mode old;
    fDrawingEngine->SetDrawingMode(B_OP_OVER, old);
    fDrawingEngine->DrawBitmap(bitmap, rect.OffsetToCopy(0, 0), rect);
    fDrawingEngine->SetDrawingMode(old);
    fDrawingEngine->SetCopyToFrontEnabled(prev);
}

ServerBitmap* SDKDecorator::_GetButtonBitmap(Decorator::Tab* tab,
                                               Component item, bool pressed,
                                               int32 width, int32 height)
{
    bool isClose  = (item == COMPONENT_CLOSE_BUTTON);
    bool focused  = tab && tab->buttonFocus;
    return fRenderer.CreateButtonBitmap(isClose, pressed, focused, width, height);
}

void SDKDecorator::_DrawClose(Decorator::Tab* _tab, bool direct, BRect rect) {
    Decorator::Tab* tab = static_cast<Decorator::Tab*>(_tab);
    int32 index = (tab->buttonFocus ? 0 : 1) + (tab->closePressed ? 0 : 2);
    ServerBitmap* bmp = tab->closeBitmaps[index];
    if (!bmp) {
        bmp = _GetButtonBitmap(tab, COMPONENT_CLOSE_BUTTON,
                                tab->closePressed,
                                rect.IntegerWidth(), rect.IntegerHeight());
        tab->closeBitmaps[index] = bmp;
    }
    _DrawButtonBitmap(bmp, direct, rect);
}

void SDKDecorator::_DrawZoom(Decorator::Tab* _tab, bool direct, BRect rect) {
    if (rect.IntegerWidth() < 1) return;
    Decorator::Tab* tab = static_cast<Decorator::Tab*>(_tab);
    int32 index = (tab->buttonFocus ? 0 : 1) + (tab->zoomPressed ? 0 : 2);
    ServerBitmap* bmp = tab->zoomBitmaps[index];
    if (!bmp) {
        bmp = _GetButtonBitmap(tab, COMPONENT_ZOOM_BUTTON,
                                tab->zoomPressed,
                                rect.IntegerWidth(), rect.IntegerHeight());
        tab->zoomBitmaps[index] = bmp;
    }
    _DrawButtonBitmap(bmp, direct, rect);
}

void SDKDecorator::_DrawMinimize(Decorator::Tab* /*tab*/, bool /*direct*/, BRect /*rect*/) {
    // Non implementato nel decorator base FlatDecorator
}

void SDKDecorator::_GetButtonSizeAndOffset(const BRect& tabRect,
                                            float* offset,
                                            float* size,
                                            float* inset) const
{
    float tabHeight = tabRect.Height();
    *offset = (float)fConfig.buttons.margin;
    *inset  = 0.0f;
    *size   = std::max(0.0f, tabHeight - (float)fConfig.buttons.margin * 2.0f);
    // Limita alla dimensione configurata
    if (*size > (float)fConfig.buttons.size)
        *size = (float)fConfig.buttons.size;
}
