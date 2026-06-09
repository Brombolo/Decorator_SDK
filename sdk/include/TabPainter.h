#ifndef TAB_PAINTER_H
#define TAB_PAINTER_H

/*
 * Haiku Decorator SDK — TabPainter.h
 * Gestisce il caricamento di texture PNG e la loro applicazione
 * sul tab con blend mode configurabile.
 * Usato da ThemeRenderer quando effect.type = "texture".
 */

#include "ThemeConfig.h"
#include <Rect.h>
#include <Bitmap.h>

class DrawingEngine;

class TabPainter {
public:
    explicit        TabPainter(const TabTextureConfig& texConfig,
                                const char* themeDir);
                   ~TabPainter();

    // Ritorna true se la texture è stata caricata con successo
    bool            IsValid() const;

    // Disegna la texture su rect applicando blend_mode e opacity
    // base_color = colore di sfondo sul quale blend (da TabColorSet)
    void            Paint(DrawingEngine* engine,
                          const BRect& rect,
                          rgb_color base_color);

private:
    BBitmap*        fTextureBitmap;     // texture originale caricata
    BBitmap*        fScaledCache;       // cache del tile pre-scalato
    TabTextureConfig fConfig;

    bool            _LoadTexture(const char* path);

    // Applica il blend mode pixel per pixel
    rgb_color       _BlendPixel(rgb_color base,
                                 rgb_color tex,
                                 float opacity) const;

    static uint8    _ClampU8(int v);
};

#endif // TAB_PAINTER_H
