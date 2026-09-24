#ifndef PATHS_H
#define PATHS_H

#include <windows.h>
#include <stddef.h>

#include "app_types.h"

BOOL paths_get_config_file(
    char *outPath,
    size_t bufferSize
);

void paths_build_config_fallback(
    AppPaths *paths
);

void paths_build_startup(
    AppPaths *paths
);

void paths_build(
    AppPaths *paths
);

void micctl_ensure(AppPaths *paths);

#endif