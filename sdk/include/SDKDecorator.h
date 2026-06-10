#ifndef SDK_DECORATOR_H
#define SDK_DECORATOR_H

/*
 * Haiku Decorator SDK — SDKDecorator.h
 *
 * Usa solo header pubblici di Haiku:
 *   <Decorator.h>  <SATDecorator.h>  <View.h>  <GraphicsDefs.h>
 *
 * Non include DrawingEngine.h, ServerBitmap.h o altri header privati
 * di app_server — quelli sono wrappati internamente da Decorator.
 */

#include "ThemeConfig.h"
#include "ThemeRenderer.h"

#include <SATDecorator.h>   // /boot/system/develop/headers/private/servers/app/decorator/
#include <DecorManager.h>   // contiene sia DecorAddOn che DecorManager
#include <Rect.h>
#include <Region.h>
#include <View.h>

class SDKDecorAddOn : public DecorAddOn {
public:
                        SDKDecorAddOn(image_id id, const char* name);

protected:
    virtual Decorator*  _AllocateDecorator(DesktopSettings& settings,
                                            BRect rect,
                                            Desktop* desktop) override;
};

class SDKDecorator : public SATDecorator {
public:
                        SDKDecorator(DesktopSettings& settings,
                                     BRect rect,
                                     Desktop* desktop,
                                     const ThemeConfig& config);
    virtual             ~SDKDecorator();

    virtual void        GetComponentColors(Component component,
                                            uint8 highlight,
                                            ComponentColors _colors,
                                            Decorator::Tab* tab) override;

protected:
    // Disegno frame e tab
    virtual void        _DrawFrame(BRect invalid) override;
    virtual void        _DrawTab(Decorator::Tab* tab, BRect invalid) override;
    virtual void        _DrawTitle(Decorator::Tab* tab, BRect r) override;

    // Disegno pulsanti
    virtual void        _DrawClose(Decorator::Tab* tab, bool direct,
                                   BRect rect) override;
    virtual void        _DrawZoom(Decorator::Tab* tab, bool direct,
                                  BRect rect) override;
    virtual void        _DrawMinimize(Decorator::Tab* tab, bool direct,
                                      BRect rect) override;

    virtual void        _GetButtonSizeAndOffset(const BRect& tabRect,
                                                 float* offset,
                                                 float* size,
                                                 float* inset) const override;

private:
    ThemeConfig         fConfig;
    ThemeRenderer       fRenderer;
    TabPainter*         fTabPainter;    // non-null solo se effect.type=texture

    // ─── Disegno tramite DrawingEngine (puntatore già in Decorator) ───────────

    // Disegna il tab usando le specifiche calcolate da ThemeRenderer
    void                _PaintTab(Decorator::Tab* tab, const BRect& tabRect,
                                   bool focused, bool stackTile);

    // Disegna un pulsante (Close o Zoom) direttamente senza bitmap cache
    void                _PaintButton(const BRect& rect,
                                      const ButtonDrawSpec& spec);

    // Disegna l'icona del pulsante (X, □, dot, ecc.)
    void                _PaintButtonIcon(const BRect& rect,
                                          const ButtonDrawSpec& spec);

    // Disegna i bordi del frame con lo stile configurato
    void                _PaintBorderEdge(const BRect& rect,
                                          bool focused, bool horizontal,
                                          bool isTopOrLeft);

    // Disegna l'angolo di resize
    void                _PaintResizeCorner(const BRect& rect, bool focused);
};

extern "C" DecorAddOn* instantiate_decor_addon(image_id id, const char* name);

#endif // SDK_DECORATOR_H
