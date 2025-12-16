/**
 * Chess Game - Network Module Header
 * P2P networking using libjuice for ICE/STUN NAT traversal.
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <stdbool.h>
#include <stddef.h>

//==============================================================================
// NETWORK CONSTANTS
//==============================================================================

#define NET_CODE_MAX_LEN 8192
#define NET_MSG_MAX_LEN 256

//==============================================================================
// NETWORK TYPES
//==============================================================================

typedef enum {
  NET_DISCONNECTED = 0,
  NET_GATHERING,
  NET_WAITING_ANSWER,
  NET_WAITING_CONNECTION,
  NET_CONNECTING,
  NET_CONNECTED,
  NET_FAILED
} NetworkState;

typedef enum {
  MSG_MOVE = 1,
  MSG_SYNC,
  MSG_RESIGN,
  MSG_DRAW_OFFER,
  MSG_DRAW_ACCEPT,
  MSG_PING,
  MSG_PONG
} MessageType;

// Move message structure
typedef struct {
  unsigned char type;
  unsigned char fromRow;
  unsigned char fromCol;
  unsigned char toRow;
  unsigned char toCol;
  unsigned char promotionPiece; // 0 if none
} MoveMessage;

// Callback for received moves
typedef void (*NetworkMoveCallback)(int fromRow, int fromCol, int toRow,
                                    int toCol, int promotionPiece);

//==============================================================================
// NETWORK STATE (defined in network.c)
//==============================================================================

extern NetworkState networkState;
extern char localOfferCode[NET_CODE_MAX_LEN];
extern char localAnswerCode[NET_CODE_MAX_LEN];
extern bool isHost;

//==============================================================================
// NETWORK FUNCTIONS
//==============================================================================

/**
 * Initialize the network module.
 */
void InitNetwork(void);

/**
 * Shutdown and cleanup network resources.
 */
void ShutdownNetwork(void);

/**
 * Create a new host session.
 * Generates offer code that can be shared with guest.
 */
void CreateHostSession(void);

/**
 * Join an existing session as guest.
 * @param offerCode The host's offer code
 */
void JoinSession(const char *offerCode);

/**
 * Set the answer code received from guest (host only).
 * @param answerCode The guest's answer code
 */
void SetAnswerCode(const char *answerCode);

/**
 * Get the current network state.
 */
NetworkState GetNetworkState(void);

/**
 * Check if network is connected.
 */
bool IsNetworkConnected(void);

/**
 * Send a move to the remote peer.
 */
void NetworkSendMove(int fromRow, int fromCol, int toRow, int toCol,
                     int promotionPiece);

/**
 * Poll network for incoming messages.
 * Should be called every frame.
 */
void PollNetwork(void);

/**
 * Set callback for received moves.
 */
void SetMoveCallback(NetworkMoveCallback callback);

/**
 * Disconnect and reset network state.
 */
void DisconnectNetwork(void);

/**
 * Get a human-readable status string.
 */
const char *GetNetworkStatusString(void);

/**
 * Finalize guest connection after user has copied answer code.
 * Called when guest clicks READY button.
 */
void FinalizeGuestConnection(void);

#endif // NETWORK_H
