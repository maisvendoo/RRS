#ifndef ANIMATIONS_LIST_H
#define ANIMATIONS_LIST_H

#include <cstddef>
#include <map>

class ProcAnimation;

using animations_t = std::multimap<std::size_t, ProcAnimation*>;

#endif // ANIMATIONS_LIST_H
