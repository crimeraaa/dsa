#ifndef COMMON_H
#define COMMON_H

#ifndef cast
#define cast(Type)  (Type)
#endif // cast

#ifndef unused
#define unused(expression)  cast(void)(expression)
#endif // unused

#endif // COMMON_H
