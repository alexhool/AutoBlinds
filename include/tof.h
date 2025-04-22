/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright 2025 Alexander Hool
 */

#ifndef TOF_H
#define TOF_H

// Initialize ToF sensor
bool setupTof();

// Detect object presence
bool isTofTriggered();

#endif // TOF_H
