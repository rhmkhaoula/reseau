#include "UAVServerApp.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"

Define_Module(UAVServerApp);

simsignal_t UAVServerApp::rcvdPkSignal = registerSignal("rcvdPk");

UAVServerApp::UAVServerApp() {
}

UAVServerApp::~UAVServerApp() {
    cancelAndDelete(processingTimer);
}

void UAVServerApp::initialize(int stage) {
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        localPort = par("localPort");
        clientPort = par("clientPort");
        processingInterval = par("processingInterval");
        numReceived = 0;
        WATCH(numReceived);
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        processingTimer = new cMessage("processingTimer");

        // Configuration du socket principal (réception)
        socket.setOutputGate(gate("socketOut"));
        socket.bind(localPort);
        socket.setCallback(this);

        // Configuration du socket client (envoi)
        clientSocket.setOutputGate(gate("socketOut"));

        // Démarrer le traitement périodique des données
        if (operationalState == State::OPERATING) {
            scheduleAt(simTime() + processingInterval, processingTimer);
        }
    }
}

void UAVServerApp::handleMessageWhenUp(cMessage *msg) {
    if (msg->isSelfMessage()) {
        if (msg == processingTimer) {
            processCollectedData();
            scheduleAt(simTime() + processingInterval, processingTimer);
        }
    }
    else
        socket.processMessage(msg);
}

void UAVServerApp::socketDataArrived(UdpSocket *socket, Packet *packet) {
    // Traitement des données reçues des autres UAVs
    auto addressInd = packet->getTag<L3AddressInd>();
    L3Address srcAddr = addressInd->getSrcAddress();

    // Calculer le délai de bout en bout
    auto creationTimeTag = packet->getTag<CreationTimeTag>();
    simtime_t delay = simTime() - creationTimeTag->getCreationTime();

    EV_INFO << "UAV Server received packet " << packet->getName() << " from UAV at "
            << srcAddr.str() << ". Delay: " << delay << "s" << endl;

    // Mettre à jour les statistiques
    numReceived++;
    packetsPerUAV[srcAddr]++;
    emit(rcvdPkSignal, packet);

    // Stocker les données pour traitement ultérieur
    processPacket(packet);

    delete packet;
}

void UAVServerApp::processPacket(Packet *pk) {
    // Extraire et stocker les données du paquet
    auto addressInd = pk->getTag<L3AddressInd>();
    auto creationTimeTag = pk->getTag<CreationTimeTag>();

    SensorData data;
    data.sourceAddress = addressInd->getSrcAddress();
    data.timestamp = creationTimeTag->getCreationTime();

    // Dans un cas réel, nous extrairions les données réelles du paquet
    // Pour la simulation, nous stockons simplement des informations de base
    collectedData.push_back(data);

    EV_INFO << "Processing data from packet " << pk->getName()
            << " from " << data.sourceAddress.str() << endl;
}

void UAVServerApp::processCollectedData() {
    if (collectedData.empty()) {
        EV_INFO << "No data to process at this time." << endl;
        return;
    }

    EV_INFO << "Processing collected data from " << collectedData.size() << " packets." << endl;

    // Dans une implémentation réelle, nous ferions ici l'agrégation des données,
    // l'analyse, la détection d'anomalies, etc.

    // Par exemple, calcul de statistiques simples par drone source
    std::map<L3Address, int> dataPointsPerUAV;
    std::map<L3Address, simtime_t> latestTimestampPerUAV;

    for (const auto& data : collectedData) {
        dataPointsPerUAV[data.sourceAddress]++;

        if (latestTimestampPerUAV.find(data.sourceAddress) == latestTimestampPerUAV.end() ||
            data.timestamp > latestTimestampPerUAV[data.sourceAddress]) {
            latestTimestampPerUAV[data.sourceAddress] = data.timestamp;
        }
    }

    // Afficher les résultats
    EV_INFO << "Data summary:" << endl;
    for (const auto& pair : dataPointsPerUAV) {
        EV_INFO << "  UAV at " << pair.first.str() << ": " << pair.second
                << " data points, latest at " << latestTimestampPerUAV[pair.first] << endl;
    }

    // Optionnellement, envoyer des commandes aux autres drones basées sur l'analyse
    sendControlMessages();

    // Vider le buffer de données après traitement
    collectedData.clear();
}

void UAVServerApp::sendControlMessages() {
    // Fonction pour envoyer des commandes ou des données agrégées aux autres drones
    // Dans une implémentation réelle, on pourrait avoir une logique conditionnelle ici

    // Pour l'instant, cette fonction est un placeholder
    EV_INFO << "Sending control messages or data summary to other UAVs could happen here" << endl;
}

void UAVServerApp::socketErrorArrived(UdpSocket *socket, Indication *indication) {
    EV_WARN << "Socket error: " << indication->getName() << endl;
    delete indication;
}

void UAVServerApp::socketClosed(UdpSocket *socket) {
    if (operationalState == State::STOPPING_OPERATION) {
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
    }
}

void UAVServerApp::handleStartOperation(LifecycleOperation *operation) {
    socket.setOutputGate(gate("socketOut"));
    socket.bind(localPort);
    socket.setCallback(this);

    clientSocket.setOutputGate(gate("socketOut"));

    processingTimer = new cMessage("processingTimer");
    scheduleAt(simTime() + processingInterval, processingTimer);
}

void UAVServerApp::handleStopOperation(LifecycleOperation *operation) {
    cancelEvent(processingTimer);
    socket.close();
    clientSocket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void UAVServerApp::handleCrashOperation(LifecycleOperation *operation) {
    cancelEvent(processingTimer);
    socket.destroy();
    clientSocket.destroy();
}

void UAVServerApp::finish() {
    ApplicationBase::finish();

    EV_INFO << "UAV Server Application finished. Received: " << numReceived << " packets in total." << endl;
    EV_INFO << "Packets received from each UAV:" << endl;

    for (auto& pair : packetsPerUAV) {
        EV_INFO << "  UAV at " << pair.first.str() << ": " << pair.second << " packets" << endl;
    }
}
