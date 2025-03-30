/*
 * Roll Numbers: B22CS063, B22CS028
 */



#ifndef __ASSIGNMENT_A_H_
#define __ASSIGNMENT_A_H_

#include <omnetpp.h>
#include <vector>
#include <set>
#include <map>
#include <string>
using namespace omnetpp;
using namespace std;

/**
 * ClientMessage
 * -------------
 * This is a custom message class used by the Client module to send integer array responses.
 * Each message contains:
 *   - 'arr': The integer array (or subarray) being processed.
 *   - 'arr_len': The length of the array.
 *   - 'ID': Identifier for the client sending the message.
 *   - 'time': A timestamp indicating when the message was created.
 */
class ClientMessage : public cMessage {
  public:
    vector<int> arr;  // The integer array or subarray.
    int arr_len;      // Length of the array.
    int ID;           // Identifier of the sending client.
    time_t time;      // Timestamp of message creation.
};

/**
 * gossipMessage
 * ---------------
 * This is a custom message class used for exchanging server scores (via gossip protocol)
 * between client nodes. The message contains:
 *   - 's': A string formatted as <timestamp>:<IP>:<score1,score2,...> representing the
 *          sender's score information.
 */
class gossipMessage : public cMessage {
  public:
    string s;  // Gossip message string.
};

/**
 * Client Module Class
 * ---------------------
 * The Client module performs the following tasks:
 * 1. Reads an input array (provided as a comma-separated string via the 'arrayData'
 *    parameter) and divides it into 'n' subtasks, where 'n' is the total number of server nodes.
 *    Each subtask has at least 2 elements.
 * 2. For each subtask, it randomly selects (n/2 + 1) servers to execute the subtask.
 * 3. It collects the responses from these servers and uses majority voting to determine
 *    the correct result for each subtask.
 * 4. The final result is computed as the maximum among the majority results of the subtasks.
 * 5. The client assigns scores to servers based on correctness (1 for correct, 0 for incorrect),
 *    then exchanges these scores with other clients via a gossip protocol.
 * 6. In a subsequent round, tasks are reassigned to the top (n/2 + 1) servers based on
 *    the accumulated scores.
 * 7. Detailed logs are maintained in a client-specific log file (client_<id>_log.txt)
 *    and a common output file (outputfile.txt).
 */
class Client : public cSimpleModule {
  private:
    int currentRound;                             // Indicates current round (1 or 2)
    int totalServers;                             // Total number of server nodes (n)
    int totalClients;                             // Total number of client nodes (m)
    vector<vector<int>> subtasks;                 // Each element is a subarray (subtask)
    vector< map<int,int> > responses;             // For each subtask, maps server index (0-indexed) to its result.
    map<int,int> subtaskResults;                  // Majority result for each subtask.
    vector<int> serverScores;                     // 0-indexed score vector for servers.
    map<int,int> serverIdToIndex;                 // Maps actual Server module ID to 0-indexed server index.

    // Helper Functions:
    int responsesProcessed();                     // Returns the number of subtasks that received (n/2+1) responses.
    void broadcastGossip();                       // Broadcasts the server score gossip message to other clients.
    vector<int> getTopServers();                  // Returns indices of top servers based on serverScores.
    void logToFile(const string &msg);            // Writes a log message to a client-specific log file.

  public:
    int server_mutex = 0;                         // Optional: Mutex for synchronizing responses.
    set<string> ML;                               // Message log to avoid duplicate gossip forwarding.
    vector<int> res;                              // Additional result storage if needed.

    // send_message: Sends a custom ClientMessage to a specified server.
    void send_message(vector<int> arr, time_t time, int ID, int arr_len, int server_id) {
        ClientMessage *newm = new ClientMessage();
        newm->arr = arr;
        newm->arr_len = arr_len;
        newm->time = time;
        newm->ID = ID;
        send(newm, "out", server_id);
    }

    // gossip_message: Sends a custom gossipMessage to a specified server.
    void gossip_message(string s, int server_id) {
        gossipMessage *newm = new gossipMessage();
        newm->s = s;
        send(newm, "out", server_id);
    }

  protected:
    virtual void initialize() override;         // Called at simulation startup.
    virtual void handleMessage(cMessage *msg) override; // Handles incoming messages.
};

#endif
