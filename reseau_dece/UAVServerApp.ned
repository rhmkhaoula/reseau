import inet.applications.contract.IApp;

simple UAVServerApp like IApp
{
    parameters:
        string interfaceTableModule;
        int localPort;                    // Port sur lequel le serveur écoute
        int clientPort = default(-1);     // Port pour communiquer avec les clients
        double processingInterval @unit(s) = default(5s);  // Interval de traitement des données
        double stopOperationExtraTime @unit(s) = default(2s);
        double stopOperationTimeout @unit(s) = default(2s);
        
        @display("i=block/app");
        @signal[rcvdPk](type=inet::Packet);
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        
    gates:
        input socketIn;
        output socketOut;
}