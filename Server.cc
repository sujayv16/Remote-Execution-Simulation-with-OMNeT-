/*
 * Roll Numbers: B22CS063, B22CS028
 */


#include "Server.h"
#include <omnetpp.h>
#include <sstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <vector>
using namespace omnetpp;
using namespace std;

Define_Module(Server);

// Log a message to the server-specific log file.
void Server::logToFile(const string &msg) {
    stringstream filename;
    filename << "server_" << getId() << "_log.txt";
    ofstream logFile(filename.str().c_str(), ios::app);
    if (logFile.is_open()) {
        logFile << msg << endl;
        logFile.close();
    }
}

void Server::initialize() {
    isMalicious = par("isMalicious");
    stringstream initMsg;
    initMsg << "Server " << getId() << " initialized as "
            << (isMalicious ? "Malicious" : "Honest") << ".";
    EV << initMsg.str() << "\n";
    logToFile(initMsg.str());
}

void Server::handleMessage(cMessage *msg) {
    int subtaskId = msg->par("subtaskId");
    const char* dataStr = msg->par("data").stringValue();
    vector<int> numbers;
    {
      string s(dataStr);
      stringstream ss(s);
      string token;
      while(getline(ss, token, ',')) {
         numbers.push_back(atoi(token.c_str()));
      }
    }
    int correctMax = *max_element(numbers.begin(), numbers.end());
    int result;
    stringstream compMsg;
    if (isMalicious) {
       result = (correctMax > 0) ? correctMax - 1 : 0;
       compMsg << "Server " << getId() << " (Malicious) computed result " << result
               << " for Subtask_" << subtaskId;
    } else {
       result = correctMax;
       compMsg << "Server " << getId() << " (Honest) computed result " << result
               << " for Subtask_" << subtaskId;
    }
    EV << compMsg.str() << "\n";
    logToFile(compMsg.str());
    stringstream sendMsg;
    sendMsg << "Server " << getId() << " sending Response for Subtask_" << subtaskId;
    EV << sendMsg.str() << "\n";
    logToFile(sendMsg.str());
    cMessage *response = new cMessage("Response");
    response->addPar("subtaskId") = subtaskId;
    response->addPar("result") = result;
    response->addPar("serverId") = getId();
    response->addPar("serverType") = isMalicious ? "Malicious" : "Honest";
    send(response, "out", msg->getArrivalGate()->getIndex());
    delete msg;
}
