#ifndef SETTINGS_H
#define SETTINGS_H

#include "app_types.h"

void settings_load(
    AppState *state,
    const AppPaths *paths
);

void settings_save(
    const AppState *state,
    const AppPaths *paths
);

#endif
