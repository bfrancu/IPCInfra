#ifndef IOINFRA_GLOBAL_H
#define IOINFRA_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(IOINFRA_LIBRARY)
#  define IOINFRA_EXPORT Q_DECL_EXPORT
#else
#  define IOINFRA_EXPORT Q_DECL_IMPORT
#endif

#endif // IOINFRA_GLOBAL_H
