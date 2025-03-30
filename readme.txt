====================================================================
Remote Execution Simulation with OMNeT++
====================================================================

Team Members:
   - Viswanadhapalli Sujay (B22CS063)
   - Kandrathi Sai Aiswarya (B22CS028)

--------------------------------------------------------------------
Project Description:
--------------------------------------------------------------------
This project simulates a remote program execution system using the OMNeT++ 
Discrete Event Simulator. The main objective is to mimic the execution 
of a simple task – finding the maximum element in an integer array – 
in a distributed environment. 

Key Features:
   1. **Task Division:** 
      Each client divides a given array (provided as a comma-separated 
      string via a parameter) into n equal subtasks (where n is the total 
      number of servers). It is ensured that each subtask contains at least 
      2 elements.
      
   2. **Subtask Assignment:** 
      Each subtask is dispatched to (n/2 + 1) randomly selected server nodes.
      
   3. **Server Behavior:** 
      Server nodes can operate in either an Honest or Malicious mode. 
      Malicious servers deliberately compute an incorrect result (by 
      subtracting 1 from the correct maximum), while honest servers compute 
      the correct maximum.
      
   4. **Response Aggregation and Majority Voting:** 
      Clients collect responses from the servers for each subtask. Using 
      majority voting, the correct result for each subtask is determined. 
      For example, if the honest result is 12 and the malicious response is 11, 
      the majority (the correct value 12) is chosen.
      
   5. **Score Assignment and Gossip Protocol:** 
      Each client assigns a score of 1 to a server if its response is correct, 
      and 0 if not. These scores are consolidated and exchanged among clients 
      using a gossip protocol. In the second round, clients use these scores 
      to assign tasks to the top (n/2 + 1) servers.
      
   6. **Two Rounds of Execution:** 
      The simulation runs in two rounds:
         - Round 1: Subtasks are distributed randomly.
         - Round 2: Subtasks are redistributed to top scoring servers based 
           on the gossip exchange.
      
--------------------------------------------------------------------
Files and Deliverables:
--------------------------------------------------------------------
1. **Source Code Files:**
   - Client.cc / Client.h: Implements client-side functionality, including
     task division, response aggregation, majority voting, score calculation,
     and gossip protocol.
   - Server.cc / Server.h: Implements server-side functionality to process
     subtasks. Servers can be configured to be honest or malicious.
   - myNetwork.ned: Contains the network topology, specifying connections
     between clients and servers as well as between clients (for gossip).
   - omnetpp.ini: Configuration file that sets parameters (e.g., number of 
     servers and clients, and which server is malicious).
   
2. **Topology File (topo.txt):**
   (Optional) A separate file documenting the network topology (e.g., listing
   which client connects to which servers). 

3. **Output Files:**
   - **outputfile.txt:** A common output file that records the final consolidated 
     results and key gossip messages.
   - **Client Log Files:** Files named `client_<id>_log.txt` (e.g., 
     client_5_log.txt, client_6_log.txt, etc.) that capture detailed logs for each 
     client, including task assignment, responses received, score updates, and 
     round transition information.
   - **Server Log Files:** Files named `server_<id>_log.txt` (e.g., server_2_log.txt, 
     server_3_log.txt, etc.) that capture each server's computed subtask result, 
     and whether it acted honestly or maliciously.

--------------------------------------------------------------------
How the Code Fulfills the Assignment Requirements:
--------------------------------------------------------------------
- **Network Setup:**  
  Clients establish connections with all server nodes as defined in myNetwork.ned.
  Additionally, clients are connected to each other to support the gossip protocol.
  
- **Task Division and Distribution:**  
  Clients divide the input array into n subtasks (with each subtask having at least 2 elements) 
  and dispatch each subtask to (n/2 + 1) randomly selected servers.
  
- **Subtask Execution:**  
  Each server processes its assigned subtask by computing the maximum element. 
  Servers are configured to operate honestly (returning the correct maximum) or maliciously 
  (returning one less than the correct maximum).
  
- **Aggregation and Majority Voting:**  
  Clients collect responses for each subtask, determine the majority outcome, and then compute 
  the final result as the maximum of all subtask results.
  
- **Scoring and Gossip:**  
  Clients assign scores (1 for correct, 0 for incorrect) to servers and broadcast these scores 
  via a gossip protocol. In the next round, tasks are assigned based on these scores.
  
- **Round Transition:**  
  The simulation runs for two rounds. The first round uses random server selection, and the second 
  round reassigns tasks to top scoring servers as determined by the gossip protocol.
  
--------------------------------------------------------------------
How to Compile and Run:
--------------------------------------------------------------------
1. **Compilation:**
   - Open a command prompt and navigate to the project directory (where omnetpp.ini and myNetwork.ned are located).
   - Build the project
     
2. **Running the Simulation:**
   - After successful compilation, run the simulation 
     ```
   - **Important:** Adjust your run configuration to point to the directory containing the generated .exe file.
     
3. **Viewing Output:**
   - Console output: Detailed logs (responses, score updates, gossip messages, round transitions) are displayed on the console.
   - Output file: `outputfile.txt` contains the consolidated task results and key messages.
   - Client logs: Individual client logs (e.g., client_5_log.txt, client_6_log.txt, etc.) are created in the project directory.
   - Server logs: Individual server logs (e.g., server_2_log.txt, server_3_log.txt, etc.) are created in the project directory.
   
--------------------------------------------------------------------
Configuration Example:
--------------------------------------------------------------------
The provided configuration in `omnetpp.ini` uses 5 servers and 3 clients, with 1 malicious server:
  
```ini
[General]
network = Net

numServers = 5
numClients = 3

**.s[0].isMalicious = true
**.s[1].isMalicious = false
**.s[2].isMalicious = false
**.s[3].isMalicious = false
**.s[4].isMalicious = false

*.debug = true
