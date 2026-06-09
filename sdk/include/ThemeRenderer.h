#ifndef THEME_RENDERER_H
#define THEME_RENDERER_H

/*
 * Haiku Decorator SDK — ThemeRenderer.h
 * Traduce ThemeConfig in chiamate concrete a DrawingEngine.
 * Usato internamente da SDKDecorator.
 */

#include "ThemeConfig.h"
#include <Rect.h>
#include <View.h>

// Forward declarations Haiku internals
class DrawingEngine;
class ServerBitmap;
namespace BitmapDrawingEngine_ { class BitmapDrawingEngine; }

class ThemeRenderer {
public:
    explicit        ThemeRenderer(const ThemeConfig& config);

    // ─── Tab ──────────────────────────────────────────────────────────────────

    // Disegna l'intero tab (sfondo + bordi + effetto)
    void            DrawTab(DrawingEngine* engine,
                            const BRect& tabRect,
                            bool focused,
                            bool stackTile);

    // Disegna solo il riempimento del tab (sfondo + eventuale gradiente/texture)
    void            DrawTabFill(DrawingEngine* engine,
                                const BRect& tabRect,
                                bool focused);

    // ─── Bordi ────────────────────────────────────────────────────────────────

    // Ritorna l'array di 6 colori per GetComponentColors() del decorator
    void            GetBorderColors(rgb_color colors[6], bool focused) const;

    // Disegna un bordo con lo stile configurato
    void            DrawBorder(DrawingEngine* engine,
                               const BRect& outerRect,
                               bool focused);

    // ─── Pulsanti ─────────────────────────────────────────────────────────────

    // Crea e ritorna il bitmap pre-renderizzato per un pulsante
    // (compatibile con il meccanismo _GetBitmapForButton del Decorator base)
    ServerBitmap*   CreateButtonBitmap(bool isClose,
                                       bool pressed,
                                       bool focused,
                                       int32 width,
                                       int32 height);

    // ─── Resize corner ────────────────────────────────────────────────────────
    void            DrawResizeCorner(DrawingEngine* engine,
                                     const BRect& rect,
                                     bool focused);

private:
    const ThemeConfig&  fConfig;

    // Calcola i colori effettivi per uno stato del tab
    TabColorSet         _ResolveTabColors(bool focused, bool stackTile) const;

    // Applica l'effetto selezionato (gradiente, glass, ecc.) su tabRect
    void                _ApplyEffect(DrawingEngine* engine,
                                     const BRect& fillRect,
                                     const TabColorSet& colors,
                                     bool focused);

    // Disegna la forma del tab con il clip corretto (flat/rounded/slanted)
    void                _DrawTabShape(DrawingEngine* engine,
                                      const BRect& tabRect,
                                      rgb_color fillColor,
                                      bool focused);

    // Disegna l'icona dentro un pulsante
    void                _DrawButtonIcon(DrawingEngine* engine,
                                        const BRect& rect,
                                        bool isClose,
                                        bool pressed,
                                        rgb_color iconColor);

    // Schiarisce o scurisce un colore di una percentuale
    static rgb_color    _Lighten(rgb_color c, float amount);
    static rgb_color    _Darken(rgb_color c, float amount);

    // Interpola tra due colori (t=0 → a, t=1 → b)
    static rgb_color    _Lerp(rgb_color a, rgb_color b, float t);
};

#endif // THEME_RENDERER_H
