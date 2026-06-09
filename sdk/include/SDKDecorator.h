#ifndef SDK_DECORATOR_H
#define SDK_DECORATOR_H

/*
 * Haiku Decorator SDK — SDKDecorator.h
 * Classe principale del decorator, estende SATDecorator.
 * Il codice utente non deve modificare questo file;
 * tutte le personalizzazioni passano da theme.conf.
 */

#include "ThemeConfig.h"
#include "ThemeRenderer.h"

#include <SATDecorator.h>   // Haiku internal

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

    // ─── Override obbligatori ─────────────────────────────────────────────────

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

    void                _DrawButtonBitmap(ServerBitmap* bitmap,
                                          bool direct, BRect rect);

    ServerBitmap*       _GetButtonBitmap(Decorator::Tab* tab,
                                          Component item,
                                          bool pressed,
                                          int32 width, int32 height);
};

// ─── Entry point richiesto da Haiku ──────────────────────────────────────────

extern "C" DecorAddOn* instantiate_decor_addon(image_id id, const char* name);

#endif // SDK_DECORATOR_H
