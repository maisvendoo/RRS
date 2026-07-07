#ifndef EDITOR_CHECK_INITIALIZATION_H
#define EDITOR_CHECK_INITIALIZATION_H

#include <Journal.h>

#include <cstdlib>

#define CHECK_INITIALIZATION(object)                                    \
    if (!object)                                                        \
    {                                                                   \
        Journal::instance()->error("Failed to initialize "#object);     \
        std::exit(EXIT_FAILURE);                                        \
    }                                                                   \
    Journal::instance()->info(#object" is initialized successfully")

#endif // EDITOR_CHECK_INITIALIZATION_H
