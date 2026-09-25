// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

struct Voice
{
    int note;
    int velocity;

    void reset()
    {
        note = -1;
        velocity = 0;
    }
};