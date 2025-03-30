
/*
 * Roll Numbers: B22CS063, B22CS028
 */


#include "Client.h"
#include <omnetpp.h>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <fstream>

using namespace omnetpp;
using namespace std;

Define_Module(Client);

/*
 * Function: responsesProcessed
 * ----------------------------
 *   Iterates through the responses for each subtask and counts how many subtasks have received
 *   the required number of responses (which is totalServers/2 + 1). This function helps determine
 *   if a client has received all necessary server responses before proceeding with majority voting.
 *
 *   Returns:
 *      The number of subtasks that have received the full set of responses.
 */
int Client::responsesProcessed() {
    int count = 0;
    for (auto &m : responses) {
       // Each subtask must have responses from (totalServers/2 + 1) servers.
       if (m.size() == (unsigned)(totalServers / 2 + 1))
          count++;
    }
    return count;
}

/*
 * Function: logToFile
 * -------------------
 *   This helper function writes a log message to a client-specific log file.
 *   The file name is constructed using the client's module ID to ensure uniqueness.
 *
 *   Parameters:
 *      msg - The log message to be written.
 */
void Client::logToFile(const string &msg) {
    stringstream filename;
    filename << "client_" << getId() << "_log.txt";
    ofstream logFile(filename.str().c_str(), ios::app); // Open file in append mode.
    if (logFile.is_open()) {
        logFile << msg << endl;
        logFile.close();
    }
}

/*
 * Function: broadcastGossip
 * -------------------------
 *   This function creates a gossip message that contains the current server scores.
 *   The message format is: <timestamp>:<module path>:<score1,score2,...>
 *   The gossip message is then sent through every available gossip gate (clientOut).
 *   This facilitates the exchange of server rating information among clients.
 */
void Client::broadcastGossip() {
    // Get current simulation time.
    simtime_t now = simTime();
    stringstream ss;
    ss << now.dbl() << ":" << getFullPath() << ":";

    // Append each server score to the gossip string.
    for (int i = 0; i < (int)serverScores.size(); i++) {
       ss << serverScores[i];
       if (i < (int)serverScores.size() - 1)
          ss << ",";
    }
    string gossipStr = ss.str();

    // Log the gossip message for debugging purposes.
    EV << "Broadcasting gossip: " << gossipStr << "\n";
    logToFile("Broadcasting gossip: " + gossipStr);

    // Create the gossip message and add parameters.
    cMessage *gossipMsg = new cMessage("Gossip");
    gossipMsg->addPar("scores") = gossipStr.c_str();
    gossipMsg->addPar("clientId") = getId();

    // Determine the number of output gates dedicated to gossip.
    int numGossipGates = gateSize("clientOut");
    for (int i = 0; i < numGossipGates; i++) {
         send(gossipMsg->dup(), "clientOut", i);
    }
    delete gossipMsg;
}

/*
 * Function: getTopServers
 * -----------------------
 *   Sorts the serverScores vector in descending order (while keeping track of the original 0-indexed server indices)
 *   and selects the top (totalServers/2 + 1) servers.
 *
 *   Returns:
 *      A vector of 0-indexed server indices corresponding to the top servers.
 */
vector<int> Client::getTopServers() {
    vector<pair<int, int>> scorePairs;

    // Build pairs of (score, index) for each server.
    for (int i = 0; i < (int)serverScores.size(); i++){
       scorePairs.push_back({serverScores[i], i});
    }

    // Sort the pair vector in descending order by score.
    sort(scorePairs.begin(), scorePairs.end(), greater<pair<int,int>>());

    // Determine how many top servers are needed.
    int num = totalServers / 2 + 1;
    vector<int> top;
    for (int i = 0; i < num && i < (int)scorePairs.size(); i++){
       top.push_back(scorePairs[i].second);
    }
    return top;
}

/*
 * Function: initialize
 * ----------------------
 *   This function is called once at the start of the simulation.
 *   It performs the following operations:
 *     1. Reads simulation parameters (totalServers, totalClients, arrayData).
 *     2. Logs initialization information.
 *     3. Builds a mapping from actual server module IDs to a 0-indexed array for score management.
 *     4. Reads and parses the input array, then divides it into subtasks.
 *     5. Randomly assigns each subtask to (n/2 + 1) servers.
 */
void Client::initialize() {
    currentRound = 1;
    totalServers = par("totalServers");
    totalClients = par("totalClients");

    // Log initial configuration.
    stringstream initMsg;
    initMsg << "Client " << getId() << " initialized with totalServers=" << totalServers
            << " and totalClients=" << totalClients;
    EV << initMsg.str() << "\n";
    logToFile(initMsg.str());

    // Prepare the server score vector.
    serverScores.resize(totalServers, 0);

    // Create mapping for server module IDs (assumes servers are named "s[0]", "s[1]", ... in the network)
    for (int i = 0; i < totalServers; i++) {
        cModule *serverModule = getParentModule()->getSubmodule("s", i);
        if (serverModule) {
            int actualId = serverModule->getId();
            serverIdToIndex[actualId] = i;
        }
    }

    // Parse the input array from the "arrayData" parameter.
    const char *arrayStr = par("arrayData").stringValue();
    vector<int> array;
    {
       string s(arrayStr);
       stringstream ss(s);
       string token;
       while (getline(ss, token, ',')) {
           array.push_back(atoi(token.c_str()));
       }
    }
    int n = totalServers;
    int x = array.size();
    if (x / n < 2) {
       logToFile("Array size (" + to_string(x) + ") too small to divide into " + to_string(n) + " subtasks.");
       endSimulation();
    }
    int subtaskSize = x / n;
    subtasks.clear();
    for (int i = 0; i < n; i++) {
       int start = i * subtaskSize;
       int end = (i == n - 1) ? x : (i + 1) * subtaskSize;
       vector<int> part(array.begin() + start, array.begin() + end);
       subtasks.push_back(part);
    }
    responses.clear();
    responses.resize(n);

    // For each subtask, randomly choose (n/2 + 1) servers and send the subtask.
    int numServersPerSubtask = n / 2 + 1;
    for (int i = 0; i < n; i++) {
         stringstream dataStream;
         for (int j = 0; j < (int)subtasks[i].size(); j++) {
             dataStream << subtasks[i][j];
             if (j != (int)subtasks[i].size() - 1)
                dataStream << ",";
         }
         string msgName = "Subtask_" + to_string(i);
         cMessage *msg = new cMessage(msgName.c_str());
         msg->addPar("subtaskId") = i;
         msg->addPar("data") = dataStream.str().c_str();
         set<int> chosen;
         while (chosen.size() < (unsigned)numServersPerSubtask) {
             int r = intuniform(0, n - 1);
             chosen.insert(r);
         }
         for (int serverIndex : chosen) {
             // Reverse mapping: find the actual server module id for logging purposes.
             int actualServerId = -1;
             for (auto &entry : serverIdToIndex) {
                 if (entry.second == serverIndex) {
                     actualServerId = entry.first;
                     break;
                 }
             }
             stringstream ss;
             ss << "Sending " << msgName << " to Server with index " << serverIndex
                << " (module id " << actualServerId << ")";
             EV << ss.str() << "\n";
             logToFile(ss.str());
             send(msg->dup(), "out", serverIndex);
         }
         delete msg;
    }
}

/*
 * Function: handleMessage
 * -------------------------
 *   This function processes incoming messages. It handles two types of messages:
 *   1. "Response" messages from servers containing the computed result for a subtask.
 *   2. "Gossip" messages from other clients containing server score information.
 *
 *   For response messages, the client:
 *     - Updates the responses mapping using a 0-indexed server index.
 *     - Logs the received response and current score state.
 *     - Once the required number of responses for a subtask is received,
 *       it performs majority voting to determine the valid result, updates server scores,
 *       and logs the majority result.
 *     - When responses for all subtasks are received, it computes the final result and
 *       (if in Round 1) transitions to Round 2 by selecting the top servers based on scores.
 *
 *   For gossip messages, the client logs the received score information.
 */
void Client::handleMessage(cMessage *msg) {
    if (strcmp(msg->getName(), "Response") == 0) {
       int subtaskId = msg->par("subtaskId");
       int result = msg->par("result");
       int actualSenderId = msg->par("serverId");
       const char* serverType = msg->par("serverType").stringValue();
       // Convert actual server module id to our 0-indexed server index.
       int senderIndex = serverIdToIndex[actualSenderId];
       responses[subtaskId][senderIndex] = result;

       // Log the received response details.
       stringstream respMsg;
       respMsg << "Received Response for Subtask_" << subtaskId
               << " from Server index " << senderIndex
               << " (module id " << actualSenderId << ", " << serverType
               << ") with result = " << result;
       EV << "Client " << getId() << " " << respMsg.str() << "\n";
       logToFile(respMsg.str());

       // Log the current server scores before processing the subtask.
       {
           stringstream scoreMsg;
           scoreMsg << "Current scores (before processing subtask " << subtaskId << "): ";
           for (int i = 0; i < (int)serverScores.size(); i++) {
               scoreMsg << "Server" << i << "=" << serverScores[i] << " ";
           }
           logToFile(scoreMsg.str());
       }

       // When enough responses for the subtask are received, perform majority voting.
       if (responses[subtaskId].size() == (unsigned)(totalServers / 2 + 1)) {
           map<int, int> freq;
           for (auto &p : responses[subtaskId])
              freq[p.second]++;
           int majorityVal = 0, maxCount = 0;
           for (auto &entry : freq) {
               if (entry.second > maxCount) {
                  maxCount = entry.second;
                  majorityVal = entry.first;
               } else if (entry.second == maxCount && entry.first > majorityVal) {
                  majorityVal = entry.first;
               }
           }
           subtaskResults[subtaskId] = majorityVal;

           // Update scores: For every response matching the majority, increment the server's score.
           for (auto &p : responses[subtaskId]) {
              int idx = p.first;
              if (p.second == majorityVal)
                 serverScores[idx] += 1;
           }

           // Log updated scores after processing the subtask.
           {
               stringstream scoreMsg;
               scoreMsg << "Updated scores after processing Subtask_" << subtaskId << ": ";
               for (int i = 0; i < (int)serverScores.size(); i++) {
                   scoreMsg << "Server" << i << "=" << serverScores[i] << " ";
               }
               EV << "Client " << getId() << " " << scoreMsg.str() << "\n";
               logToFile(scoreMsg.str());
           }

           // Log the majority result for the subtask.
           {
              stringstream subtaskLog;
              subtaskLog << "Subtask_" << subtaskId << " majority result = " << majorityVal;
              logToFile(subtaskLog.str());
           }
       }

       // When responses for all subtasks are in, compute the final result.
       if (responsesProcessed() == totalServers) {
            int finalResult = 0;
            for (auto &entry : subtaskResults)
               finalResult = max(finalResult, entry.second);
            stringstream finalMsg;
            finalMsg << "Final Result in Round " << currentRound << " = " << finalResult;
            EV << "Client " << getId() << " " << finalMsg.str() << "\n";
            logToFile(finalMsg.str());

            // Append final result to the common output file.
            ofstream outfile("outputfile.txt", ios::app);
            if (outfile.is_open()) {
                outfile << "Client " << getId() << " Round " << currentRound
                        << " Final Result = " << finalResult << "\n";
                outfile.close();
            }

            // Transition to Round 2 if in Round 1.
            if (currentRound == 1) {
                currentRound = 2;
                logToFile("Transitioning to Round 2. Selecting top servers based on current scores.");
                vector<int> topServers = getTopServers();
                stringstream round2Log;
                round2Log << "Top servers selected for Round 2: ";
                for (int sid : topServers)
                    round2Log << "Server" << sid << " ";
                logToFile(round2Log.str());
                // Clear previous responses and majority results for round transition.
                for (auto &m : responses)
                    m.clear();
                subtaskResults.clear();

                // Resend the same subtasks in Round 2 to top-scoring servers.
                int n = totalServers;
                int numServersPerSubtask = n / 2 + 1;
                for (int i = 0; i < n; i++) {
                    stringstream dataStream;
                    for (int j = 0; j < (int)subtasks[i].size(); j++){
                       dataStream << subtasks[i][j];
                       if (j != (int)subtasks[i].size() - 1)
                          dataStream << ",";
                    }
                    string msgName = "Subtask_" + to_string(i);
                    cMessage *msg = new cMessage(msgName.c_str());
                    msg->addPar("subtaskId") = i;
                    msg->addPar("data") = dataStream.str().c_str();
                    set<int> chosen;
                    int idx = 0;
                    while (chosen.size() < (unsigned)numServersPerSubtask) {
                       if (idx < (int)topServers.size())
                          chosen.insert(topServers[idx]);
                       else
                          chosen.insert(intuniform(0, n - 1));
                       idx++;
                    }
                    for (int serverIndex : chosen) {
                        stringstream round2Msg;
                        round2Msg << "Round 2: Sending " << msgName << " to Server " << serverIndex;
                        logToFile(round2Msg.str());
                        send(msg->dup(), "out", serverIndex);
                    }
                    delete msg;
                }
            }
       }
       delete msg;
    }
    else if (strcmp(msg->getName(), "Gossip") == 0) {
       const char* scoreStr = msg->par("scores").stringValue();
       int senderClient = msg->par("clientId");
       stringstream gossipRecv;
       gossipRecv << "Received gossip from Client " << senderClient << " with scores: " << scoreStr;
       EV << "Client " << getId() << " " << gossipRecv.str() << "\n";
       logToFile(gossipRecv.str());
       ofstream outfile("outputfile.txt", ios::app);
       if (outfile.is_open()) {
           outfile << "Client " << getId() << " received gossip from Client " << senderClient
                   << " with scores: " << scoreStr << "\n";
           outfile.close();
       }
       delete msg;
    }
    else {
       delete msg;
    }
}
