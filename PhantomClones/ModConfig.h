#ifndef MODCONFIG_H
#define MODCONFIG_H

#include "GameAPI/Game.h"

typedef struct {
    int32 cloneCount;
    bool32 allowInBosses;
} ModConfig;

extern ModConfig config;

#endif // !MODCONFIG_H
