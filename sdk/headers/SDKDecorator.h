#ifndef SDK_DECORATOR_H
#define SDK_DECORATOR_H

/*
 * Haiku Decorator SDK — SDKDecorator.h
 */

#include "ThemeConfig.h"
#include "ThemeRenderer.h"
#include "TabPainter.h"

#include <SATDecorator.h>
#include <DecorManager.h>
#include <DrawingEngine.h>   // necessario per il tipo completo, presente in includes/
#include <Rect.h>
#include <View.h>
#include <image.h>

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
    virtual void        _DrawFrame(BRect invalid) override;
    virtual void        _DrawTab(Decorator::Tab* tab, BRect invalid) override;
    virtual void        _DrawTitle(Decorator::Tab* tab, BRect r) override;
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
    TabPainter*         fTabPainter;

    void                _PaintTab(Decorator::Tab* tab, const BRect& tabRect,
                                   bool focused, bool stackTile);
    void                _PaintButton(const BRect& rect,
                                      const ButtonDrawSpec& spec);
    void                _PaintButtonIcon(const BRect& rect,
                                          const ButtonDrawSpec& spec);
    void                _PaintBorderEdge(const BRect& rect, bool focused,
                                          bool horizontal, bool isTopOrLeft);
    void                _PaintResizeCorner(const BRect& rect, bool focused);
};

extern "C" DecorAddOn* instantiate_decor_addon(image_id id, const char* name);

#endif // SDK_DECORATOR_H
