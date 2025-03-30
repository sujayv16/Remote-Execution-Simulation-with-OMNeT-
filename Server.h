/*
 * Roll Numbers: B22CS063, B22CS028
 */


#ifndef _ASSIGNMENT_A_H
#define _ASSIGNMENT_A_H

#include <omnetpp.h>
using namespace omnetpp;
using namespace std;

/**
 * @brief Server module class representing a computational server.
 * 
 * This class models a server that processes incoming messages.
 * It includes functionality to determine whether the server behaves
 * maliciously or honestly.
 */
class Server : public cSimpleModule {
  private:
    // Flag to indicate if this server is malicious.
    bool isMalicious;
    
    /**
     * @brief Logs messages to a dedicated server-specific log file.
     * 
     * Each server maintains a separate log file identified by its ID.
     * @param msg The message to be logged.
     */
    void logToFile(const string &msg);

  protected:
    /**
     * @brief Initializes the server module.
     * 
     * Determines if the server is malicious or honest based on parameters.
     * Logs initialization details.
     */
    virtual void initialize() override;
    
    /**
     * @brief Handles incoming messages.
     * 
     * Processes incoming computational tasks and determines results
     * based on the server's behavior (malicious/honest). 
     * Logs computations and sends responses accordingly.
     * @param msg The incoming message to be processed.
     */
    virtual void handleMessage(cMessage *msg) override;
};

#endif // _ASSIGNMENT_A_H