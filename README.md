# Real-Time UDP Transport Protocol

This project implements a custom UDP transport protocol designed to stream 160-byte media frames every 20ms across a hostile network that drops, delays, reorders, and duplicates packets[cite: 11]. 

The main goal was to meet strict performance caps (under 1% packet loss, under 2x bandwidth overhead[cite: 9]) while minimizing playout latency[cite: 11].

## How It Works

* **XOR Parity (FEC):** Instead of using a slow feedback loop for retransmissions or wasting bandwidth on 100% duplication, the sender pairs two consecutive frames and generates an XOR parity packet. This keeps bandwidth overhead down to an efficient ~1.54x.
* **Stateless Receiver:** To avoid getting stuck behind delayed packets (head-of-line blocking), the receiver immediately forwards incoming data to the player. If a frame is missing, it uses the parity packet to mathematically reconstruct it on the fly.

## Performance Results

* **Profile A (Mild Loss & Jitter):** Achieved a valid run with a **60 ms** playout delay at 1.54x overhead.
* **Profile B (Burst Loss & High Jitter):** Achieved a valid run with a **110 ms** playout delay at 1.54x overhead.

## Running the Tests

Compile the binaries using the Makefile[cite: 11]:
```bash
make
