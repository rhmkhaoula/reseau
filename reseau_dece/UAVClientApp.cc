#include "UAVClientApp.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"

Define_Module(UAVClientApp);

simsignal_t UAVClientApp::sentPkSignal = registerSignal("sentPk");
simsignal_t UAVClientApp::rcvdPkSignal = registerSignal("rcvdPk");

UAVClientApp::UAVClientApp() {
}

UAVClientApp::~UAVClientApp() {
    cancelAndDelete(sendTimer);
}

void UAVClientApp::initialize(int stage) {
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        sendInterval = par("sendInterval");
        localPort = par("localPort");
        destPort = par("destPort");

        numSent = 0;
        numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        sendTimer = new cMessage("sendTimer");

        socket.setOutputGate(gate("socketOut"));
        socket.bind(localPort);
        socket.setCallback(this);

        // Résoudre l'adresse du drone serveur
        const char *destAddrs = par("serverAddress");
        L3AddressResolver().tryResolve(destAddrs, serverAddress);

        if (serverAddress.isUnspecified()) {
            EV_ERROR << "Cannot resolve server address: " << destAddrs << endl;
        }
        else if (operationalState == State::OPERATING) {
            scheduleAt(simTime() + par("startTime"), sendTimer);
        }
    }
}

void UAVClientApp::handleMessageWhenUp(cMessage *msg) {
    if (msg->isSelfMessage()) {
        if (msg == sendTimer) {
            sendSensorData();
            scheduleAt(simTime() + sendInterval, sendTimer);
        }
    }
    else
        socket.processMessage(msg);
}

void UAVClientApp::collectSensorData() {
    // Simulation de la collecte de données d'un capteur
    // Similaire à l'application originale UAVSensorApp
    EV_INFO << "Collecting sensor data..." << endl;
}

void UAVClientApp::sendSensorData() {
    collectSensorData();

    char msgName[32];
    sprintf(msgName, "UAVClientData-%d", numSent);

    // Créer un paquet avec les données du capteur
    Packet *packet = new Packet(msgName);

    // Ajouter un tag de temps pour mesurer la latence
    auto creationTimeTag = packet->addTag<CreationTimeTag>();
    creationTimeTag->setCreationTime(simTime());

    // Simuler les données du capteur avec un payload de taille fixe
    const auto& payload = makeShared<ByteCountChunk>(B(par("messageLength")));
    packet->insertAtBack(payload);

    // Envoyer au drone serveur (au lieu de la station de base)
    socket.sendTo(packet, serverAddress, destPort);

    numSent++;
    emit(sentPkSignal, packet);

    EV_INFO << "Sent sensor data to UAV server at " << serverAddress.str() << endl;
}

void UAVClientApp::socketDataArrived(UdpSocket *socket, Packet *packet) {
    // Traitement des commandes ou données reçues du drone serveur
    EV_INFO << "Received packet from UAV server: " << packet->getName() << endl;
    numReceived++;
    emit(rcvdPkSignal, packet);

    // Traiter les commandes éventuelles
    processServerCommand(packet);

    delete packet;
}

void UAVClientApp::processServerCommand(Packet *packet) {
    // Traiter les commandes ou instructions reçues du drone serveur
    // Dans une implémentation réelle, cela pourrait inclure:
    // - Changer le comportement ou la trajectoire du drone
    // - Modifier les paramètres de collecte de données
    // - Exécuter des actions spécifiques

    // Pour la simulation, nous nous contentons de journaliser l'événement
    EV_INFO << "Processing command from UAV server..." << endl;
}

void UAVClientApp::socketErrorArrived(UdpSocket *socket, Indication *indication) {
    EV_WARN << "Socket error: " << indication->getName() << endl;
    delete indication;
}

void UAVClientApp::socketClosed(UdpSocket *socket) {
    if (operationalState == State::STOPPING_OPERATION) {
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
    }
}

void UAVClientApp::handleStartOperation(LifecycleOperation *operation) {
    socket.setOutputGate(gate("socketOut"));
    socket.bind(localPort);
    socket.setCallback(this);

    if (!serverAddress.isUnspecified()) {
        sendTimer = new cMessage("sendTimer");
        scheduleAt(simTime() + par("startTime"), sendTimer);
    }
}

void UAVClientApp::handleStopOperation(LifecycleOperation *operation) {
    cancelEvent(sendTimer);
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void UAVClientApp::handleCrashOperation(LifecycleOperation *operation) {
    cancelEvent(sendTimer);
    socket.destroy();
}

void UAVClientApp::finish() {
    ApplicationBase::finish();
    EV_INFO << "UAV Client Application finished. Sent: " << numSent << " packets, Received: " << numReceived << " packets." << endl;
}
