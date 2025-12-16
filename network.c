/**
 * Chess Game - Network Module
 * P2P networking implementation using libjuice for ICE/STUN NAT traversal.
 */

#include "network.h"
#include "libjuice/include/juice/juice.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//==============================================================================
// NETWORK STATE
//==============================================================================

NetworkState networkState = NET_DISCONNECTED;
char localOfferCode[NET_CODE_MAX_LEN] = {0};
char localAnswerCode[NET_CODE_MAX_LEN] = {0};
bool isHost = false;

static juice_agent_t *agent = NULL;
static NetworkMoveCallback moveCallback = NULL;

// Buffers for candidate trickling
static char pendingCandidates[NET_CODE_MAX_LEN] = {0};
static int candidateCount = 0;

// Store offer code for guest - will apply AFTER local gathering completes
static char storedOfferCode[NET_CODE_MAX_LEN] = {0};
static bool hasStoredOfferCode = false;

// Forward declaration
static void parse_and_apply_code(const char *code, bool setRemote,
                                 bool signalGatheringDone);

//==============================================================================
// BASE64 URL-SAFE ENCODING/DECODING
//==============================================================================

static const char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

static void base64_encode(const char *input, size_t inLen, char *output,
                          size_t outMaxLen) {
  size_t i, j;
  for (i = 0, j = 0; i < inLen && j < outMaxLen - 4;) {
    unsigned int octet_a = i < inLen ? (unsigned char)input[i++] : 0;
    unsigned int octet_b = i < inLen ? (unsigned char)input[i++] : 0;
    unsigned int octet_c = i < inLen ? (unsigned char)input[i++] : 0;

    unsigned int triple = (octet_a << 16) | (octet_b << 8) | octet_c;

    output[j++] = base64_chars[(triple >> 18) & 0x3F];
    output[j++] = base64_chars[(triple >> 12) & 0x3F];
    output[j++] = base64_chars[(triple >> 6) & 0x3F];
    output[j++] = base64_chars[triple & 0x3F];
  }
  output[j] = '\0';
}

static int base64_char_value(char c) {
  if (c >= 'A' && c <= 'Z')
    return c - 'A';
  if (c >= 'a' && c <= 'z')
    return c - 'a' + 26;
  if (c >= '0' && c <= '9')
    return c - '0' + 52;
  if (c == '-')
    return 62;
  if (c == '_')
    return 63;
  return -1;
}

static void base64_decode(const char *input, char *output, size_t outMaxLen) {
  size_t inLen = strlen(input);
  size_t i, j;

  for (i = 0, j = 0; i < inLen && j < outMaxLen - 1;) {
    int v0 = base64_char_value(input[i++]);

    // Track whether we have actual input data at each position
    bool hasV1 = (i < inLen);
    bool hasV2 = false;
    bool hasV3 = false;

    int v1 = hasV1 ? base64_char_value(input[i++]) : 0;
    hasV2 = (i < inLen);
    int v2 = hasV2 ? base64_char_value(input[i++]) : 0;
    hasV3 = (i < inLen);
    int v3 = hasV3 ? base64_char_value(input[i++]) : 0;

    if (v0 < 0 || v1 < 0)
      break;

    unsigned int triple = (v0 << 18) | (v1 << 12) | (v2 << 6) | v3;

    if (j < outMaxLen - 1)
      output[j++] = (triple >> 16) & 0xFF;
    if (j < outMaxLen - 1 && hasV2 && v2 >= 0)
      output[j++] = (triple >> 8) & 0xFF;
    if (j < outMaxLen - 1 && hasV3 && v3 >= 0)
      output[j++] = triple & 0xFF;
  }
  output[j] = '\0';
}

//==============================================================================
// LIBJUICE CALLBACKS
//==============================================================================

static void on_state_changed(juice_agent_t *agent, juice_state_t state,
                             void *user_ptr) {
  (void)agent;
  (void)user_ptr;

  printf("[Network] State changed: %s\n", juice_state_to_string(state));

  switch (state) {
  case JUICE_STATE_DISCONNECTED:
    networkState = NET_DISCONNECTED;
    break;
  case JUICE_STATE_GATHERING:
    networkState = NET_GATHERING;
    break;
  case JUICE_STATE_CONNECTING:
    networkState = NET_CONNECTING;
    break;
  case JUICE_STATE_CONNECTED:
  case JUICE_STATE_COMPLETED:
    networkState = NET_CONNECTED;
    printf("[Network] Connected!\n");
    break;
  case JUICE_STATE_FAILED:
    networkState = NET_FAILED;
    printf("[Network] Connection failed!\n");
    break;
  }
}

static void on_candidate(juice_agent_t *agent, const char *sdp,
                         void *user_ptr) {
  (void)agent;
  (void)user_ptr;

  printf("[Network] Candidate: %s\n", sdp);

  // Append candidate to pending list
  if (strlen(pendingCandidates) + strlen(sdp) + 2 < NET_CODE_MAX_LEN) {
    if (candidateCount > 0) {
      strcat(pendingCandidates, "\n");
    }
    strcat(pendingCandidates, sdp);
    candidateCount++;
  }
}

static void on_gathering_done(juice_agent_t *agent, void *user_ptr) {
  (void)user_ptr;

  printf("[Network] Gathering done\n");

  // Get local description (SDP)
  char sdp[JUICE_MAX_SDP_STRING_LEN];
  juice_get_local_description(agent, sdp, sizeof(sdp));

  // Combine SDP with candidates - truncate if too long
  char fullDesc[NET_CODE_MAX_LEN];
  size_t sdpLen = strlen(sdp);
  size_t sepLen = strlen("\n---CANDIDATES---\n");
  size_t candLen = strlen(pendingCandidates);
  size_t maxCandLen = sizeof(fullDesc) - sdpLen - sepLen - 1;

  if (candLen > maxCandLen) {
    printf("[Network] Warning: candidates truncated (%zu > %zu)\n", candLen,
           maxCandLen);
    candLen = maxCandLen;
  }

  snprintf(fullDesc, sizeof(fullDesc), "%s\n---CANDIDATES---\n%.*s", sdp,
           (int)candLen, pendingCandidates);

  // Encode as base64 for easy copy/paste
  if (isHost) {
    base64_encode(fullDesc, strlen(fullDesc), localOfferCode,
                  sizeof(localOfferCode));
    networkState = NET_WAITING_ANSWER;
    printf("[Network] Offer code generated, length: %zu\n",
           strlen(localOfferCode));
  } else {
    // Guest: generate our answer code
    base64_encode(fullDesc, strlen(fullDesc), localAnswerCode,
                  sizeof(localAnswerCode));
    printf("[Network] Answer code generated, length: %zu\n",
           strlen(localAnswerCode));

    // DON'T apply offer code here - wait for user to copy answer code first
    // User will click READY button which calls FinalizeGuestConnection()
    networkState = NET_WAITING_CONNECTION;
    printf("[Network] Waiting for user to copy answer code and click READY\n");
  }
}

static void on_recv(juice_agent_t *agent, const char *data, size_t size,
                    void *user_ptr) {
  (void)agent;
  (void)user_ptr;

  if (size >= sizeof(MoveMessage)) {
    MoveMessage *msg = (MoveMessage *)data;

    if (msg->type == MSG_MOVE && moveCallback) {
      printf("[Network] Received move: %d,%d -> %d,%d\n", msg->fromRow,
             msg->fromCol, msg->toRow, msg->toCol);
      moveCallback(msg->fromRow, msg->fromCol, msg->toRow, msg->toCol,
                   msg->promotionPiece);
    }
  }
}

//==============================================================================
// HELPER FUNCTIONS
//==============================================================================

static void parse_and_apply_code(const char *code, bool setRemote,
                                 bool signalGatheringDone) {
  // Decode base64
  char decoded[NET_CODE_MAX_LEN];
  base64_decode(code, decoded, sizeof(decoded));

  printf("[Network] Decoded code length: %zu\n", strlen(decoded));

  // Split SDP and candidates
  char *candidateSep = strstr(decoded, "\n---CANDIDATES---\n");
  if (candidateSep) {
    *candidateSep = '\0';
    char *candidatesStr = candidateSep + strlen("\n---CANDIDATES---\n");

    printf("[Network] SDP length: %zu, Candidates present: %s\n",
           strlen(decoded), strlen(candidatesStr) > 0 ? "yes" : "no");

    // Set remote SDP
    if (setRemote && agent) {
      int result = juice_set_remote_description(agent, decoded);
      printf("[Network] Set remote description result: %d\n", result);

      // Make a copy for strtok since it modifies the string
      char candidatesCopy[NET_CODE_MAX_LEN];
      strncpy(candidatesCopy, candidatesStr, sizeof(candidatesCopy) - 1);
      candidatesCopy[sizeof(candidatesCopy) - 1] = '\0';

      // Add remote candidates
      char *line = strtok(candidatesCopy, "\n");
      while (line) {
        if (strlen(line) > 0) {
          printf("[Network] Adding remote candidate: %s\n", line);
          juice_add_remote_candidate(agent, line);
        }
        line = strtok(NULL, "\n");
      }

      // Only signal remote gathering done if requested
      // For guest, we delay this until AFTER our own gathering is complete
      if (signalGatheringDone) {
        printf("[Network] Signaling remote gathering done\n");
        juice_set_remote_gathering_done(agent);
      }
    }
  } else {
    printf("[Network] No candidate separator found in decoded data\n");
  }
}

static void create_agent(void) {
  juice_config_t config;
  memset(&config, 0, sizeof(config));

  // Use Google's public STUN server
  config.stun_server_host = "stun.l.google.com";
  config.stun_server_port = 19302;

  config.cb_state_changed = on_state_changed;
  config.cb_candidate = on_candidate;
  config.cb_gathering_done = on_gathering_done;
  config.cb_recv = on_recv;
  config.user_ptr = NULL;

  // Use thread mode for simpler integration
  config.concurrency_mode = JUICE_CONCURRENCY_MODE_THREAD;

  agent = juice_create(&config);
  if (!agent) {
    printf("[Network] Failed to create agent!\n");
    networkState = NET_FAILED;
    return;
  }

  printf("[Network] Agent created\n");
}

//==============================================================================
// PUBLIC API
//==============================================================================

void InitNetwork(void) {
  // Enable verbose logging for debugging
  juice_set_log_level(JUICE_LOG_LEVEL_VERBOSE);
  networkState = NET_DISCONNECTED;
  agent = NULL;
  moveCallback = NULL;
  memset(localOfferCode, 0, sizeof(localOfferCode));
  memset(localAnswerCode, 0, sizeof(localAnswerCode));
  memset(pendingCandidates, 0, sizeof(pendingCandidates));
  candidateCount = 0;
  isHost = false;
  printf("[Network] Network module initialized\n");
}

void ShutdownNetwork(void) {
  if (agent) {
    juice_destroy(agent);
    agent = NULL;
  }
  networkState = NET_DISCONNECTED;
}

void CreateHostSession(void) {
  DisconnectNetwork();

  isHost = true;
  memset(pendingCandidates, 0, sizeof(pendingCandidates));
  candidateCount = 0;

  create_agent();
  if (agent) {
    networkState = NET_GATHERING;
    juice_gather_candidates(agent);
  }
}

void JoinSession(const char *offerCode) {
  printf("[Network] JoinSession called, offer code length: %zu\n",
         strlen(offerCode));
  DisconnectNetwork();

  isHost = false;
  memset(pendingCandidates, 0, sizeof(pendingCandidates));
  candidateCount = 0;

  create_agent();
  if (agent) {
    printf("[Network] Agent created for guest\n");

    // IMPORTANT: Store the offer code to apply AFTER our gathering is done
    // If we apply it now, libjuice will start connecting and skip our gathering
    strncpy(storedOfferCode, offerCode, sizeof(storedOfferCode) - 1);
    storedOfferCode[sizeof(storedOfferCode) - 1] = '\0';
    hasStoredOfferCode = true;
    printf("[Network] Offer code stored, will apply after local gathering\n");

    // Start gathering our candidates - offer code will be applied in callback
    networkState = NET_GATHERING;
    printf("[Network] Starting candidate gathering...\n");
    juice_gather_candidates(agent);
  } else {
    printf("[Network] ERROR: Failed to create agent for guest!\n");
    networkState = NET_FAILED;
  }
}

void SetAnswerCode(const char *answerCode) {
  if (agent && isHost) {
    parse_and_apply_code(answerCode, true,
                         true); // true = signal gathering done now
    networkState = NET_CONNECTING;
  }
}

void FinalizeGuestConnection(void) {
  // Called when guest clicks READY after copying their answer code
  if (agent && !isHost && hasStoredOfferCode) {
    printf("[Network] Guest clicked READY, applying stored offer code...\n");
    parse_and_apply_code(storedOfferCode, true, true);
    hasStoredOfferCode = false;
    memset(storedOfferCode, 0, sizeof(storedOfferCode));
    networkState = NET_CONNECTING;
    printf("[Network] Now connecting to host...\n");
  }
}

NetworkState GetNetworkState(void) { return networkState; }

bool IsNetworkConnected(void) { return networkState == NET_CONNECTED; }

void NetworkSendMove(int fromRow, int fromCol, int toRow, int toCol,
                     int promotionPiece) {
  if (!agent || networkState != NET_CONNECTED) {
    return;
  }

  MoveMessage msg;
  msg.type = MSG_MOVE;
  msg.fromRow = (unsigned char)fromRow;
  msg.fromCol = (unsigned char)fromCol;
  msg.toRow = (unsigned char)toRow;
  msg.toCol = (unsigned char)toCol;
  msg.promotionPiece = (unsigned char)promotionPiece;

  juice_send(agent, (const char *)&msg, sizeof(msg));
  printf("[Network] Sent move: %d,%d -> %d,%d\n", fromRow, fromCol, toRow,
         toCol);
}

void PollNetwork(void) {
  // libjuice handles network in background threads
  // Nothing to poll manually
}

void SetMoveCallback(NetworkMoveCallback callback) { moveCallback = callback; }

void DisconnectNetwork(void) {
  if (agent) {
    juice_destroy(agent);
    agent = NULL;
  }
  networkState = NET_DISCONNECTED;
  memset(localOfferCode, 0, sizeof(localOfferCode));
  memset(localAnswerCode, 0, sizeof(localAnswerCode));
  memset(pendingCandidates, 0, sizeof(pendingCandidates));
  candidateCount = 0;
}

const char *GetNetworkStatusString(void) {
  switch (networkState) {
  case NET_DISCONNECTED:
    return "Disconnected";
  case NET_GATHERING:
    return "Gathering candidates...";
  case NET_WAITING_ANSWER:
    return "Waiting for answer code";
  case NET_WAITING_CONNECTION:
    return "Waiting for connection";
  case NET_CONNECTING:
    return "Connecting...";
  case NET_CONNECTED:
    return "Connected";
  case NET_FAILED:
    return "Connection failed";
  default:
    return "Unknown";
  }
}
