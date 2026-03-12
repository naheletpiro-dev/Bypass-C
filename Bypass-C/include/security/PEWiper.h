#pragma once
#include <windows.h>

class PEWiper {
public:
    /**
     * @param level 1: Wipe básico (MZ/PE), 2: Wipe agresivo (Headers completos)
     */
    static void Initialize(int level);

private:
    static void WipeHeaders();
    static void AggressiveWipe();
};