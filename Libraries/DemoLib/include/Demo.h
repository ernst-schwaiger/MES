#ifndef DEMO_H
#define DEMO_H

#include <stdint.h>

// add extern "C" header footer if the API shall be available from C++ modules as well
#ifdef __cplusplus
extern "C" {
#endif

uint32_t getValue();

#ifdef __cplusplus
}
#endif

#endif // DEMO_H