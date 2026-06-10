#ifndef TAB_PAINTER_H
#define TAB_PAINTER_H

/*
 * Haiku Decorator SDK — TabPainter.h
 *
 * Carica una texture PNG e calcola il colore blendato per ogni pixel.
 * NON usa DrawingEngine direttamente: ritorna i pixel in un BBitmap
 * che SDKDecorator disegna con DrawBitmap() tramite l'API pubblica.
 */

#include "ThemeConfig.h"
#include <Bitmap.h>
#include <Rect.h>

class TabPainter {
public:
    explicit        TabPainter(const TabTextureConfig& config,
                                const char* themeDir);
                   ~TabPainter();

    bool            IsValid() const;

    // Ritorna un BBitmap (B_RGB32) con la texture blendato su base_color.
    // Le dimensioni corrispondono a rect.
    // Il caller è responsabile del delete.
    BBitmap*        CreateBlendedBitmap(const BRect& rect,
                                         rgb_color base_color) const;

private:
    BBitmap*            fTexture;
    TabTextureConfig    fConfig;

    bool                _Load(const char* path);
    rgb_color           _BlendPixel(rgb_color base, rgb_color tex,
                                     float opacity) const;
    static uint8        _Clamp(int v);
};

#endif // TAB_PAINTER_H
