#ifndef ZEPHIO_EXPORT_H
#define ZEPHIO_EXPORT_H

#ifdef ZEPHIO_BUILDING_SO
  #define ZEPHIO_API __attribute__((visibility("default")))
#else
  #define ZEPHIO_API
#endif

#endif