/*
 * Haiku Decorator SDK — TabPainter.cpp
 * Carica texture PNG e le applica al tab con blend mode software.
 *
 * Nota: Haiku app_server non espone alpha blending composited ai decorator.
 * Il blending viene calcolato pixel per pixel in CPU e scritto direttamente
 * nel DrawingEngine. Per texture di piccole dimensioni (tile ≤ 64x64) questo
 * è accettabile; per texture grandi usa opacity bassa.
 */

#include "TabPainter.h"

#include <TranslationUtils.h>
#include <BitmapStream.h>
#include <File.h>
#include <string.h>
#include <algorithm>

using std::min;
using std::max;

// Haiku internals
#include "DrawingEngine.h"

// ─── Costruttore / Distruttore ────────────────────────────────────────────────

TabPainter::TabPainter(const TabTextureConfig& texConfig, const char* themeDir)
    : fTextureBitmap(nullptr),
      fScaledCache(nullptr),
      fConfig(texConfig)
{
    if (fConfig.file.IsEmpty()) return;

    // Costruisci percorso assoluto relativo alla cartella del tema
    BString path(themeDir);
    path << "/" << fConfig.file;

    _LoadTexture(path.String());
}

TabPainter::~TabPainter() {
    delete fTextureBitmap;
    delete fScaledCache;
}

bool TabPainter::IsValid() const {
    return fTextureBitmap != nullptr;
}

// ─── Caricamento texture ──────────────────────────────────────────────────────

bool TabPainter::_LoadTexture(const char* path) {
    // Usa il Translation Kit di Haiku per caricare qualsiasi formato immagine
    BBitmap* bmp = BTranslationUtils::GetBitmapFile(path);
    if (!bmp || !bmp->IsValid()) {
        delete bmp;
        return false;
    }

    // Converti sempre in B_RGBA32 per semplificare il blending
    if (bmp->ColorSpace() != B_RGBA32) {
        BRect bounds = bmp->Bounds();
        BBitmap* conv = new (std::nothrow) BBitmap(bounds, B_RGBA32);
        if (!conv || !conv->IsValid()) {
            delete bmp;
            delete conv;
            return false;
        }
        // Copia con conversione di spazio colore
        uint8* src = (uint8*)bmp->Bits();
        uint8* dst = (uint8*)conv->Bits();
        int32  w   = bounds.IntegerWidth() + 1;
        int32  h   = bounds.IntegerHeight() + 1;
        int32  src_bpr = bmp->BytesPerRow();
        int32  dst_bpr = conv->BytesPerRow();

        for (int32 y = 0; y < h; y++) {
            uint8* sRow = src + y * src_bpr;
            uint8* dRow = dst + y * dst_bpr;
            for (int32 x = 0; x < w; x++) {
                // Haiku BGRA → RGBA32 (che è sempre BGRA in Haiku)
                dRow[0] = sRow[0];
                dRow[1] = sRow[1];
                dRow[2] = sRow[2];
                dRow[3] = 255;
                sRow += (bmp->ColorSpace() == B_RGB32 ? 4 : 3);
                dRow += 4;
            }
        }
        delete bmp;
        fTextureBitmap = conv;
    } else {
        fTextureBitmap = bmp;
    }
    return true;
}

// ─── Blend pixel ─────────────────────────────────────────────────────────────

uint8 TabPainter::_ClampU8(int v) {
    if (v < 0)   return 0;
    if (v > 255) return 255;
    return (uint8)v;
}

rgb_color TabPainter::_BlendPixel(rgb_color base,
                                   rgb_color tex,
                                   float opacity) const
{
    rgb_color out = base;

    auto lerp8 = [](int a, int b, float t) -> uint8 {
        return (uint8)(a + (b - a) * t);
    };

    switch (fConfig.blend_mode) {
        case BLEND_MULTIPLY: {
            // out = base * tex / 255
            rgb_color mul = {
                (uint8)((base.red   * tex.red)   / 255),
                (uint8)((base.green * tex.green) / 255),
                (uint8)((base.blue  * tex.blue)  / 255),
                255
            };
            out.red   = lerp8(base.red,   mul.red,   opacity);
            out.green = lerp8(base.green, mul.green, opacity);
            out.blue  = lerp8(base.blue,  mul.blue,  opacity);
            break;
        }
        case BLEND_OVERLAY: {
            auto overlay_ch = [](int b, int t) -> int {
                if (b < 128)
                    return (2 * b * t) / 255;
                else
                    return 255 - (2 * (255-b) * (255-t)) / 255;
            };
            rgb_color ov = {
                _ClampU8(overlay_ch(base.red,   tex.red)),
                _ClampU8(overlay_ch(base.green, tex.green)),
                _ClampU8(overlay_ch(base.blue,  tex.blue)),
                255
            };
            out.red   = lerp8(base.red,   ov.red,   opacity);
            out.green = lerp8(base.green, ov.green, opacity);
            out.blue  = lerp8(base.blue,  ov.blue,  opacity);
            break;
        }
        case BLEND_SCREEN: {
            // out = 1 - (1-base)*(1-tex)
            rgb_color sc = {
                _ClampU8(255 - ((255-base.red)  *(255-tex.red)  /255)),
                _ClampU8(255 - ((255-base.green)*(255-tex.green)/255)),
                _ClampU8(255 - ((255-base.blue) *(255-tex.blue) /255)),
                255
            };
            out.red   = lerp8(base.red,   sc.red,   opacity);
            out.green = lerp8(base.green, sc.green, opacity);
            out.blue  = lerp8(base.blue,  sc.blue,  opacity);
            break;
        }
        case BLEND_NORMAL:
        default:
            out.red   = lerp8(base.red,   tex.red,   opacity);
            out.green = lerp8(base.green, tex.green, opacity);
            out.blue  = lerp8(base.blue,  tex.blue,  opacity);
            break;
    }
    return out;
}

// ─── Paint ────────────────────────────────────────────────────────────────────

void TabPainter::Paint(DrawingEngine* engine,
                        const BRect& rect,
                        rgb_color base_color)
{
    if (!IsValid()) return;

    BRect texBounds = fTextureBitmap->Bounds();
    int32 texW = texBounds.IntegerWidth() + 1;
    int32 texH = texBounds.IntegerHeight() + 1;

    int32 rLeft  = (int32)rect.left;
    int32 rTop   = (int32)rect.top;
    int32 rRight = (int32)rect.right;
    int32 rBot   = (int32)rect.bottom;

    uint8* texBits = (uint8*)fTextureBitmap->Bits();
    int32  texBPR  = fTextureBitmap->BytesPerRow();

    // Disegno pixel per pixel con tiling o stretch
    for (int32 y = rTop; y <= rBot; y++) {
        for (int32 x = rLeft; x <= rRight; x++) {
            int32 tx, ty;
            if (fConfig.tile) {
                tx = (x - rLeft) % texW;
                ty = (y - rTop)  % texH;
            } else {
                // Stretch: mappa [rect] → [texture]
                tx = (int32)(((float)(x - rLeft) / (rRight - rLeft)) * (texW - 1));
                ty = (int32)(((float)(y - rTop)  / (rBot   - rTop))  * (texH - 1));
            }

            // Leggi pixel texture (formato BGRA in Haiku B_RGBA32)
            uint8* tp = texBits + ty * texBPR + tx * 4;
            rgb_color tex_pixel = { tp[2], tp[1], tp[0], tp[3] };

            rgb_color blended = _BlendPixel(base_color, tex_pixel, fConfig.opacity);
            engine->StrokePoint(BPoint((float)x, (float)y), blended);
        }
    }
}
