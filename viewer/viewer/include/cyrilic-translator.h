#ifndef     CYRILIC_TRANSLATOR_H
#define     CYRILIC_TRANSLATOR_H

#include    "keys-translator.h"

class CyrilicTranslator : public KeysTranslator
{
public:

    CyrilicTranslator(QObject *parent = Q_NULLPTR);

    ~CyrilicTranslator();
};

#endif // CYRILIC_TRANSLATOR_H
