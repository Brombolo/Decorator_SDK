════════════════════════════════════════════════════════════════════
 Haiku Decorator SDK — Header privati necessari
════════════════════════════════════════════════════════════════════

Questa cartella deve contenere gli header privati di Haiku che non
fanno parte di haiku_devel.

LA FONTE: il tuo repository haiku_darkstyle
────────────────────────────────────────────
Trovi la cartella in:

  haiku_darkstyle/FlatDecorator/includes/

Copia l'intera cartella qui dentro:

  cp -r /percorso/haiku_darkstyle/FlatDecorator/includes/* \
        /percorso/Decorator_SDK/sdk/private-headers/

QUALI FILE SERVONO AL NOSTRO SDK
──────────────────────────────────
  SATDecorator.h     ← la classe base che estendiamo
  Decorator.h        ← incluso da SATDecorator.h
  DecorManager.h     ← contiene ANCHE DecorAddOn (non è un file separato)
  Desktop.h          ← parametro del costruttore
  DesktopSettings.h  ← parametro del costruttore
  DrawState.h        ← per fDrawState.Font() nel DrawTitle
  RGBColor.h         ← tipo colore interno

NOTA: DecorAddOn.h NON ESISTE come file separato.
La classe DecorAddOn è dichiarata dentro DecorManager.h.

File che il FlatDecorator usa ma NOI NON usiamo (non servono ma
non causano problemi se presenti):
  DrawingEngine.h
  PatternHandler.h
  BitmapDrawingEngine.h
  ServerBitmap.h

════════════════════════════════════════════════════════════════════
