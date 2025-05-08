#ifndef __UAVSERVERAPP_H
#define __UAVSERVERAPP_H

#include <omnetpp.h>
#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/packet/Packet.h"

using namespace omnetpp;
using namespace inet;

/**
 * Application serveur pour le drone principal
 * Cette application combine les fonctionnalités de la station de base
 * et du drone pour créer un serveur mobile décentralisé
 */
class UAVServerApp : public ApplicationBase, public UdpSocket::ICallback {
  protected:
    // Configuration
    int localPort = -1;
    int clientPort = -1;      // Port pour communiquer avec les autres drones

    // État
    UdpSocket socket;         // Socket pour recevoir les données
    UdpSocket clientSocket;   // Socket pour envoyer des commandes si nécessaire
    cMessage *processingTimer = nullptr;
    simtime_t processingInterval;

    // Statistiques
    int numReceived = 0;
    std::map<L3Address, int> packetsPerUAV;
    static simsignal_t rcvdPkSignal;

    // Stockage de données
    struct SensorData {
        L3Address sourceAddress;
        simtime_t timestamp;
        std::vector<uint8_t> data;
    };
    std::vector<SensorData> collectedData;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;

    // Méthodes d'application
    virtual void processCollectedData();
    virtual void processPacket(Packet *pk);
    virtual void sendControlMessages();

    // Méthodes du socket
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

    // LifecycleOperation
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  public:
    UAVServerApp();
    virtual ~UAVServerApp();
};

#endif
