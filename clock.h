/**
 * Chess Game - Clock System
 * Chess clock with multiple time control modes.
 */

#ifndef CLOCK_H
#define CLOCK_H

#include "types.h"
#include <stdbool.h>

//==============================================================================
// CLOCK TYPES
//==============================================================================

typedef enum {
  CLOCK_NONE = 0,     // No clock (unlimited time)
  CLOCK_SUDDEN_DEATH, // Time only, no increment
  CLOCK_FISCHER,      // Add increment after each move
  CLOCK_SIMPLE_DELAY, // Delay before clock starts counting
  CLOCK_BRONSTEIN     // Add time up to delay based on move speed
} ClockType;

//==============================================================================
// CLOCK STATE
//==============================================================================

typedef struct {
  ClockType type;
  float baseTimeSeconds;    // Initial time per player
  float incrementSeconds;   // Increment/delay in seconds
  float whiteTimeRemaining; // White's remaining time
  float blackTimeRemaining; // Black's remaining time
  float delayRemaining;     // For delay-based clocks
  float moveStartTime;      // For Bronstein: time when move started
  bool isRunning;           // Whether clock is actively counting
  bool whiteFlagged;        // White ran out of time
  bool blackFlagged;        // Black ran out of time
} ChessClock;

//==============================================================================
// CLOCK CONFIGURATION (for setup screen)
//==============================================================================

typedef struct {
  ClockType selectedType;
  int baseMinutes;
  int baseSeconds;
  int incrementSeconds;
} ClockConfig;

//==============================================================================
// GLOBAL CLOCK STATE (defined in clock.c)
//==============================================================================

extern ChessClock gameClock;
extern ClockConfig clockConfig;

//==============================================================================
// CLOCK FUNCTIONS
//==============================================================================

/**
 * Initialize clock configuration to defaults.
 */
void InitClockConfig(void);

/**
 * Initialize the game clock with current configuration.
 * Called when starting a new game.
 */
void InitClock(void);

/**
 * Update the clock each frame.
 * Decrements active player's time based on delta time.
 * @param currentTurn The color whose clock should be running
 */
void UpdateClock(PieceColor currentTurn);

/**
 * Switch the clock after a move is made.
 * Applies increment/delay based on clock type.
 * @param playerWhoMoved The color that just completed their move
 */
void SwitchClock(PieceColor playerWhoMoved);

/**
 * Check if a player has flagged (run out of time).
 * @return COLOR_WHITE if white flagged, COLOR_BLACK if black flagged,
 *         COLOR_NONE if neither
 */
PieceColor CheckTimeout(void);

/**
 * Format time for display.
 * Shows MM:SS normally, SS.t when under 20 seconds.
 * @param seconds Time in seconds
 * @param buffer Output buffer (should be at least 10 chars)
 */
void FormatTime(float seconds, char *buffer);

/**
 * Start the clock running (called when game begins).
 */
void StartClock(void);

/**
 * Stop the clock (for pause, game over, etc).
 */
void StopClock(void);

/**
 * Check if the clock is enabled for this game.
 */
bool IsClockEnabled(void);

/**
 * Get remaining time for a player.
 */
float GetPlayerTime(PieceColor color);

#endif // CLOCK_H
