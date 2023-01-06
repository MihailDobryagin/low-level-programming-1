#ifndef structures_h
#define structures_h

#include <stdint.h>

#define DEFINE_ARRAY(type)                                               \
  struct array_##type {                                                  \
    type value;                                                         \
    struct list_##type* next;                                           \
  };


DEFINE_LIST(int64_t)
DEFINE_LIST(double)


#endif // !structures_h