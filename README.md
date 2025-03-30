# Remote Execution Simulation with OMNeT++

## Overview

This project implements a remote program execution simulation using the OMNeT++ Discrete Event Simulator. The simulation demonstrates a distributed system where each client divides a given task (finding the maximum element in an integer array) into _n_ subtasks. Each subtask is then assigned to _n/2 + 1_ randomly selected server nodes for execution. The client collects the responses from these servers and uses majority voting to determine the correct result for that subtask. Finally, the final task result is computed as the maximum of the individual subtask results.

In addition, the clients are connected to each other (via a gossip protocol) so that they can exchange and average server scores. The server scores reflect the correctness of the responses (1 for an honest result, 0 for a malicious one). In the next round, clients use these scores to assign tasks to the top _n/2 + 1_ servers.

## Project Structure

- **Client.cc / Client.h:**  
  Implements the client module that divides the task into subtasks, sends subtasks to a subset of servers, collects responses, and performs majority voting to compute the final result. It also logs detailed information (subtask responses, score updates, gossip messages) into a client-specific log file (`client_<id>_log.txt`) and to a common output file (`outputfile.txt`).

- **Server.cc / Server.h:**  
  Implements the server module that receives a subtask, computes the maximum element in its sub-array, and sends a response back to the client. Each server is capable of operating in either an **Honest** or **Malicious** mode (controlled via the `isMalicious` parameter). Malicious servers intentionally return an incorrect result (by subtracting 1 from the correct maximum). Each server logs its computation and response details into a server-specific log file (`server_<id>_log.txt`).

- **myNetwork.ned:**  
  Defines the network topology. The network includes:
  - A set of server nodes (e.g., 5 servers)
  - A set of client nodes (e.g., 3 clients)
  
  The topology ensures that:
  - Each client is connected to all servers.
  - Each client is also connected to all other clients to support the gossip protocol.

- **omnetpp.ini:**  
  Contains the simulation configuration. For example, it specifies:
  - `numServers = 5` and `numClients = 3`
  - The `isMalicious` parameter for each server (e.g., one malicious server, others honest).
  
- **topo.txt:**  
  (Optional) A separate topology file that documents the connections between client and server nodes. This file is maintained separately so that it can be edited during evaluation without changing the code.

## How It Works

1. **Task Division:**  
   Each client reads an integer array (provided as a comma-separated string via the `arrayData` parameter) and divides it into _n_ subtasks, where _n_ equals the total number of servers. It is ensured that each subtask contains at least 2 elements.

2. **Subtask Assignment:**  
   For each subtask, the client randomly selects _n/2 + 1_ servers to execute the subtask. The servers compute the maximum of their assigned subarray.

3. **Response Aggregation and Majority Voting:**  
   The client collects responses for each subtask and uses majority voting to determine the correct result. For example, if the honest (correct) result for a subtask is 12 and malicious servers return 11, then the majority vote returns 12.

4. **Score Calculation and Gossip:**  
   Each client assigns a score of 1 to a server if its response is correct, and 0 if not. The consolidated scores are then exchanged among clients using the gossip protocol. This information is later used to assign tasks in the subsequent round (Round 2) to the top _n/2 + 1_ servers.

5. **Two Rounds of Execution:**  
   The simulation runs for two rounds:
   - **Round 1:** Subtasks are assigned randomly.
   - **Round 2:** Tasks are reassigned to the top-scoring servers based on the gossip exchange.

## How to Compile and Run

1. **Compile:**  
   From the project root directory (where the `omnetpp.ini` and `myNetwork.ned` files are located), run:
   ```
   make MODE=release all
   ```
   This will compile the project in release mode.

2. **Run the Simulation:**  
   After compilation, run the simulation using:
   ```
   out/clang-release/cn.exe -m -u Qtenv -n . omnetpp.ini
   ```
   This starts the OMNeT++ simulation environment (Qtenv) using your configuration.

3. **Viewing Logs:**  
   - **Console Logs:** The simulation prints detailed logs (subtask responses, score updates, gossip messages) to the console.
   - **Output File:** The consolidated task results and gossip exchanges are appended to `outputfile.txt`.
   - **Module-Specific Logs:**  
     - Client-specific logs are stored in files named `client_<id>_log.txt` (e.g., `client_5_log.txt`).
     - Server-specific logs are stored in files named `server_<id>_log.txt` (e.g., `server_2_log.txt`).

## Example Configuration (omnetpp.ini)

```ini
[General]
network = Net

# Define the number of servers and clients.
numServers = 5
numClients = 3

# Configure malicious behavior for servers:
**.s[0].isMalicious = true
**.s[1].isMalicious = false
**.s[2].isMalicious = false
**.s[3].isMalicious = false
**.s[4].isMalicious = false

# (Optional) Set debug or log levels if needed.
*.debug = true
```
