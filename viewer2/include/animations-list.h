#ifndef ANIMATIONS_LIST_H
#define ANIMATIONS_LIST_H

#include <QMap>

#include <cstddef>

class ProcAnimation;

using animations_t = QMap<std::size_t, ProcAnimation*>;

#endif // ANIMATIONS_LIST_H
