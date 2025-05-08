#ifndef __UAVCLIENTAPP_H
#define __UAVCLIENTAPP_H

#include <omnetpp.h>
#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/packet/Packet.h"

using namespace omnetpp;
using namespace inet;

/**
 * Application client pour les drones standards
 * Ces drones envoient leurs données au drone serveur au lieu de la station de base
 */
class UAVClientApp : public ApplicationBase, public UdpSocket::ICallback {
  protected:
    // Configuration
    int localPort = -1;
    int destPort = -1;
    L3Address serverAddress;  // Adresse du drone serveur

    // État
    UdpSocket socket;
    cMessage *sendTimer = nullptr;
    simtime_t sendInterval;

    // Statistiques
    int numSent = 0;
    int numReceived = 0;
    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;

    // Méthodes d'application
    virtual void sendSensorData();
    virtual void collectSensorData();
    virtual void processServerCommand(Packet *packet);

    // Méthodes du socket
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

    // LifecycleOperation
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  public:
    UAVClientApp();
    virtual ~UAVClientApp();
};

#endif
