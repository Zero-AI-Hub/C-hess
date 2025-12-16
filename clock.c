/**
 * Chess Game - Clock System
 * Chess clock with multiple time control modes.
 */

#include "clock.h"
#include "raylib.h"
#include <stdio.h>

//==============================================================================
// GLOBAL STATE DEFINITIONS
//==============================================================================

ChessClock gameClock = {0};
ClockConfig clockConfig = {0};

//==============================================================================
// CLOCK CONFIGURATION
//==============================================================================

void InitClockConfig(void) {
  // Default: 5 minute blitz with no increment
  clockConfig.selectedType = CLOCK_FISCHER;
  clockConfig.baseMinutes = 5;
  clockConfig.baseSeconds = 0;
  clockConfig.incrementSeconds = 3;
}

//==============================================================================
// CLOCK INITIALIZATION
//==============================================================================

void InitClock(void) {
  float baseTime =
      (float)(clockConfig.baseMinutes * 60 + clockConfig.baseSeconds);

  gameClock.type = clockConfig.selectedType;
  gameClock.baseTimeSeconds = baseTime;
  gameClock.incrementSeconds = (float)clockConfig.incrementSeconds;
  gameClock.whiteTimeRemaining = baseTime;
  gameClock.blackTimeRemaining = baseTime;
  gameClock.delayRemaining = 0.0f;
  gameClock.moveStartTime = 0.0f;
  gameClock.isRunning = false;
  gameClock.whiteFlagged = false;
  gameClock.blackFlagged = false;
}

//==============================================================================
// CLOCK UPDATE
//==============================================================================

void UpdateClock(PieceColor currentTurn) {
  if (!gameClock.isRunning || gameClock.type == CLOCK_NONE) {
    return;
  }

  float dt = GetFrameTime();

  // Handle delay-based clocks
  if (gameClock.type == CLOCK_SIMPLE_DELAY && gameClock.delayRemaining > 0) {
    gameClock.delayRemaining -= dt;
    if (gameClock.delayRemaining < 0) {
      // Delay exhausted, start counting from main time
      dt = -gameClock.delayRemaining;
      gameClock.delayRemaining = 0;
    } else {
      // Still in delay period, don't decrement main time
      return;
    }
  }

  // Decrement active player's time
  if (currentTurn == COLOR_WHITE) {
    gameClock.whiteTimeRemaining -= dt;
    if (gameClock.whiteTimeRemaining <= 0) {
      gameClock.whiteTimeRemaining = 0;
      gameClock.whiteFlagged = true;
      gameClock.isRunning = false;
    }
  } else if (currentTurn == COLOR_BLACK) {
    gameClock.blackTimeRemaining -= dt;
    if (gameClock.blackTimeRemaining <= 0) {
      gameClock.blackTimeRemaining = 0;
      gameClock.blackFlagged = true;
      gameClock.isRunning = false;
    }
  }
}

//==============================================================================
// CLOCK SWITCH (after move)
//==============================================================================

void SwitchClock(PieceColor playerWhoMoved) {
  if (gameClock.type == CLOCK_NONE) {
    return;
  }

  switch (gameClock.type) {
  case CLOCK_FISCHER:
    // Add increment to player who just moved
    if (playerWhoMoved == COLOR_WHITE) {
      gameClock.whiteTimeRemaining += gameClock.incrementSeconds;
    } else {
      gameClock.blackTimeRemaining += gameClock.incrementSeconds;
    }
    break;

  case CLOCK_SIMPLE_DELAY:
    // Reset delay for the next player
    gameClock.delayRemaining = gameClock.incrementSeconds;
    break;

  case CLOCK_BRONSTEIN: {
    // Add back time used, up to the delay amount
    float timeUsed =
        gameClock.moveStartTime - (playerWhoMoved == COLOR_WHITE
                                       ? gameClock.whiteTimeRemaining
                                       : gameClock.blackTimeRemaining);

    // Clamp to increment amount (can't add more than delay)
    float timeToAdd = timeUsed;
    if (timeToAdd > gameClock.incrementSeconds) {
      timeToAdd = gameClock.incrementSeconds;
    }
    if (timeToAdd < 0) {
      timeToAdd = 0;
    }

    if (playerWhoMoved == COLOR_WHITE) {
      gameClock.whiteTimeRemaining += timeToAdd;
    } else {
      gameClock.blackTimeRemaining += timeToAdd;
    }

    // Store current time for next move's Bronstein calculation
    gameClock.moveStartTime =
        (playerWhoMoved == COLOR_WHITE ? gameClock.blackTimeRemaining
                                       : gameClock.whiteTimeRemaining);
    break;
  }

  case CLOCK_SUDDEN_DEATH:
  case CLOCK_NONE:
  default:
    // No increment for sudden death
    break;
  }
}

//==============================================================================
// TIMEOUT CHECK
//==============================================================================

PieceColor CheckTimeout(void) {
  if (gameClock.whiteFlagged) {
    return COLOR_WHITE;
  }
  if (gameClock.blackFlagged) {
    return COLOR_BLACK;
  }
  return COLOR_NONE;
}

//==============================================================================
// TIME FORMATTING
//==============================================================================

void FormatTime(float seconds, char *buffer) {
  if (seconds < 0) {
    seconds = 0;
  }

  if (seconds < 20.0f) {
    // Show tenths of seconds when low on time
    int secs = (int)seconds;
    int tenths = (int)((seconds - secs) * 10);
    sprintf(buffer, "%d.%d", secs, tenths);
  } else {
    // Normal MM:SS format
    int mins = (int)(seconds / 60);
    int secs = (int)seconds % 60;
    sprintf(buffer, "%d:%02d", mins, secs);
  }
}

//==============================================================================
// CLOCK CONTROL
//==============================================================================

void StartClock(void) {
  if (gameClock.type != CLOCK_NONE) {
    gameClock.isRunning = true;

    // Initialize delay for delay-based clocks
    if (gameClock.type == CLOCK_SIMPLE_DELAY) {
      gameClock.delayRemaining = gameClock.incrementSeconds;
    }

    // Initialize move start time for Bronstein
    if (gameClock.type == CLOCK_BRONSTEIN) {
      gameClock.moveStartTime = gameClock.whiteTimeRemaining;
    }
  }
}

void StopClock(void) { gameClock.isRunning = false; }

bool IsClockEnabled(void) { return gameClock.type != CLOCK_NONE; }

float GetPlayerTime(PieceColor color) {
  if (color == COLOR_WHITE) {
    return gameClock.whiteTimeRemaining;
  } else if (color == COLOR_BLACK) {
    return gameClock.blackTimeRemaining;
  }
  return 0.0f;
}
