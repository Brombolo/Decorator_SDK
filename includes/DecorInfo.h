/*
 * DecorInfo.h — stub minimale per la compilazione del decorator SDK.
 *
 * Il file originale è in haiku/headers/private/interface/DecorInfo.h
 * Questo stub contiene solo la dichiarazione di BDecorInfo necessaria
 * a DecorManager.h. Non serve nient'altro al nostro SDK.
 *
 * Licenza originale: MIT (Haiku Project)
 */

#ifndef _DECOR_INFO_H
#define _DECOR_INFO_H

#include <Entry.h>
#include <String.h>
#include <SupportDefs.h>

class BDecorInfo {
public:
                        BDecorInfo();
                        BDecorInfo(const entry_ref& ref);
    virtual             ~BDecorInfo();

    status_t            InitCheck() const;

    const entry_ref&    Ref() const         { return fRef; }
    const char*         Name() const        { return fName.String(); }
    const char*         ShortcutSymbol() const { return fShortcut.String(); }

    bool                IsDefault() const   { return fIsDefault; }

private:
    entry_ref           fRef;
    BString             fName;
    BString             fShortcut;
    bool                fIsDefault;
    status_t            fStatus;
};

#endif // _DECOR_INFO_H
