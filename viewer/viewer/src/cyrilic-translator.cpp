#include    "cyrilic-translator.h"

#include    "cyrilic-keys.h"

#include    <osgGA/GUIEventAdapter>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CyrilicTranslator::CyrilicTranslator(QObject *parent) : KeysTranslator(parent)
{
    keymap.insert(Cyrillic_yu,          osgGA::GUIEventAdapter::KEY_Period);
    keymap.insert(Cyrillic_a,           osgGA::GUIEventAdapter::KEY_F);
    keymap.insert(Cyrillic_be,          osgGA::GUIEventAdapter::KEY_Comma);
    keymap.insert(Cyrillic_tse,         osgGA::GUIEventAdapter::KEY_W);
    keymap.insert(Cyrillic_de,          osgGA::GUIEventAdapter::KEY_L);
    keymap.insert(Cyrillic_ie,          osgGA::GUIEventAdapter::KEY_T);
    keymap.insert(Cyrillic_ef,          osgGA::GUIEventAdapter::KEY_A);
    keymap.insert(Cyrillic_ghe,         osgGA::GUIEventAdapter::KEY_U);
    keymap.insert(Cyrillic_ha,          osgGA::GUIEventAdapter::KEY_Leftbracket);
    keymap.insert(Cyrillic_i,           osgGA::GUIEventAdapter::KEY_B);
    keymap.insert(Cyrillic_shorti,      osgGA::GUIEventAdapter::KEY_Q);
    keymap.insert(Cyrillic_ka,          osgGA::GUIEventAdapter::KEY_R);
    keymap.insert(Cyrillic_el,          osgGA::GUIEventAdapter::KEY_K);
    keymap.insert(Cyrillic_em,          osgGA::GUIEventAdapter::KEY_V);
    keymap.insert(Cyrillic_en,          osgGA::GUIEventAdapter::KEY_Y);
    keymap.insert(Cyrillic_o,           osgGA::GUIEventAdapter::KEY_J);
    keymap.insert(Cyrillic_pe,          osgGA::GUIEventAdapter::KEY_G);
    keymap.insert(Cyrillic_ya,          osgGA::GUIEventAdapter::KEY_Z);
    keymap.insert(Cyrillic_er,          osgGA::GUIEventAdapter::KEY_H);
    keymap.insert(Cyrillic_es,          osgGA::GUIEventAdapter::KEY_C);
    keymap.insert(Cyrillic_te,          osgGA::GUIEventAdapter::KEY_N);
    keymap.insert(Cyrillic_u,           osgGA::GUIEventAdapter::KEY_E);
    keymap.insert(Cyrillic_zhe,         osgGA::GUIEventAdapter::KEY_Semicolon);
    keymap.insert(Cyrillic_ve,          osgGA::GUIEventAdapter::KEY_D);
    keymap.insert(Cyrillic_softsign,    osgGA::GUIEventAdapter::KEY_M);
    keymap.insert(Cyrillic_yeru,        osgGA::GUIEventAdapter::KEY_S);
    keymap.insert(Cyrillic_ze,          osgGA::GUIEventAdapter::KEY_P);
    keymap.insert(Cyrillic_sha,         osgGA::GUIEventAdapter::KEY_I);
    keymap.insert(Cyrillic_e,           osgGA::GUIEventAdapter::KEY_Quote);
    keymap.insert(Cyrillic_shcha,       osgGA::GUIEventAdapter::KEY_O);
    keymap.insert(Cyrillic_che,         osgGA::GUIEventAdapter::KEY_X);
    keymap.insert(Cyrillic_hardsign,    osgGA::GUIEventAdapter::KEY_Rightbracket);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CyrilicTranslator::~CyrilicTranslator()
{

}
