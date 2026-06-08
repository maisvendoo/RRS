#ifndef CORE_GET_MODULE_H
#define CORE_GET_MODULE_H


#define GET_MODULE(ClassName)                      \
    extern "C" void* get_module()    \
    {                                              \
        return (void*)(new (ClassName));           \
    }

#endif // CORE_GET_MODULE_H
