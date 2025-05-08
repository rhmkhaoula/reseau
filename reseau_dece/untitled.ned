// UAVNetwork_Decentralized.ned

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;
import inet.node.inet.AdhocHost;

network UAVNetwork_Decentralized
{
    parameters:
        @display("bgb=800,600;bgi=background/terrain,s");

    submodules:
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }

        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,150");
        }

        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,250");
        }

        // Le drone serveur
        uav[0]: AdhocHost {
            parameters:
                @display("i=misc/drone_server");
        }

        // Les autres drones clients
        uav[1..4]: AdhocHost {
            parameters:
                @display("i=misc/drone");
        }

    connections allowunconnected:
}
