#ifndef GUARD_GLOBALS_H
#define GUARD_GLOBALS_H

/*
 * Umbrella header.  Includes every subsystem header so existing TUs that
 * `#include "globals.h"` still see the full project-wide symbol set.
 *
 * New code should prefer the narrower subsystem headers when only one or
 * two subsystems are touched.
 */

#include "global.h"

#include "audio.h"
#include "entity.h"
#include "game.h"
#include "gfx.h"
#include "input.h"
#include "save.h"
#include "ui.h"

#endif /* GUARD_GLOBALS_H */
