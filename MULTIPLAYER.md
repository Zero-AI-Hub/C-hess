# Multiplayer Guide

This chess game supports P2P multiplayer using NAT traversal technology. Play with a friend over the internet without needing to open router ports!

## How It Works

The game uses ICE (Interactive Connectivity Establishment) with STUN to punch through NATs and establish a direct connection between players. Connection codes are exchanged manually via any messaging app.

## Creating a Game (Host)

1. Launch the game and click **MULTIPLAYER**
2. Click **CREATE GAME**
3. Wait for your **Offer Code** to be generated
4. Click **COPY CODE** to copy it
5. Send the code to your friend (via Discord, WhatsApp, etc.)
6. Wait for your friend to send their **Answer Code**
7. Paste their code into the input field
8. Click **CONNECT**

You will play as **White** (the first player to move).

## Joining a Game (Guest)  

1. Launch the game and click **MULTIPLAYER**
2. Click **JOIN GAME**
3. Paste the host's **Offer Code** into the input field
4. Click **CONNECT**
5. Wait for your **Answer Code** to be generated
6. Click **COPY CODE** to copy it
7. Send the code back to the host

You will play as **Black**.

## Tips

- **Code Exchange**: Use any messaging platform to share codes (Discord, WhatsApp, Telegram, etc.)
- **Copy/Paste**: Use Ctrl+V to paste codes into the input fields
- **Connection Time**: It may take a few seconds to establish the connection
- **Turn Enforcement**: You can only move pieces during your turn

## Troubleshooting

### Connection Failed
- Verify you copied the complete code (they can be long)
- Check your internet connection
- Try creating/joining again

### Very Long Codes
This is normal! The codes contain network information needed to establish the connection through NATs. Just copy the entire code.

### Can't Connect on Certain Networks
Some corporate or strict networks block UDP hole punching. Try from a different network or use a mobile hotspot.

## Technical Details

- Uses [libjuice](https://github.com/paullouisageneau/libjuice) for ICE/STUN
- Connects via Google's public STUN server (stun.l.google.com:19302)
- Direct peer-to-peer connection - no relay server
- No port forwarding required for most residential networks
