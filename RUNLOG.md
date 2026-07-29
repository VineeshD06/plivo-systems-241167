# Experiment Log

**Experiment 1: Baseline Test**
* Profile: A.json
* Delay: 60ms
* Miss %: 2.60%
* Overhead: 1.02x
* Notes: Ran the naive C baseline. Failed due to the network's natural packet drops causing missed deadlines. 

**Experiment 2: 100% Duplication**
* Profile: A.json
* Delay: 60ms
* Miss %: 2.07%
* Overhead: 2.05x
* Notes: Altered sender to buffer packets and send a duplicate 80ms later. Reduced misses slightly but failed the strict 2.00x bandwidth cap.

**Experiment 3: XOR Parity with Ordered Buffer**
* Profile: A.json
* Delay: 60ms
* Miss %: > 98.00%
* Overhead: 1.54x
* Notes: Implemented XOR FEC (1 parity packet per 2 data packets). Bandwidth is perfect, but receiver locked up. The strict ordered while-loop caused head-of-line blocking on delayed packets.

**Experiment 4: XOR Parity + Stateless Receiver**
* Profile: A.json
* Delay: 60ms
* Miss %: 0.93%
* Overhead: 1.54x
* Notes: Removed sequence-waiting loop. Receiver now forwards instantly and recovers dropped packets via XOR on the fly. VALID run!

**Experiment 5: Profile B Stress Test**
* Profile: B.json
* Delay: 120ms
* Miss %: 0.80%
* Overhead: 1.54x
* Notes: Tested the harsh profile with burst losses and 80ms max jitter. VALID run!

**Experiment 6: Profile B Optimization (Finding the Floor)**
* Profile: B.json
* Delay: 110ms
* Miss %: < 1.00%
* Overhead: 1.54x
* Notes: Walked the delay down. 100ms failed (parity packets arrived right as the deadline expired). 110ms is the optimal mathematical floor for our offset strategy. VALID run!