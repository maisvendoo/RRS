#ifndef ANIMATIONS_LIST_H
#define ANIMATIONS_LIST_H

#include <map>
#include <cstddef>

class ProcAnimation;

using animations_t = std::multimap<std::size_t, ProcAnimation*>;

#endif // ANIMATIONS_LIST_H
