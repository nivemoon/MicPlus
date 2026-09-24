#ifndef STARTUP_H
#define STARTUP_H

#include <windows.h>

#include "app_types.h"

BOOL startup_is_enabled(
    const AppPaths *paths
);

BOOL startup_create(
    const AppPaths *paths
);

void startup_remove(
    const AppPaths *paths
);

void startup_set(
    const AppPaths *paths,
    BOOL enable
);

#endif
