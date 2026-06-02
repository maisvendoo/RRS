#ifndef EDITOR_CHECK_MACRO_H
#define EDITOR_CHECK_MACRO_H

#define CHECK(result, success)    \
    if (!result)                  \
    {                             \
        success = false;          \
        return;                   \
    }

#endif // EDITOR_CHECK_MACRO_H
