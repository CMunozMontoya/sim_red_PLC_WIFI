#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/log.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/mobility-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Depa1");

//fx para imprimir nodos
void
PrintNodes (std::string cont_name ,NodeContainer *n)
{
  std::cout << cont_name << std::endl;
  for(uint32_t i=0; i <= n->GetN()-1; i++)
  {
    std::cout << " Node ID: " << n->Get(i)->GetId()
              << std::endl;
  }
  std::cout << std::endl;
}

void
PrintPosition (std::string container_name, NodeContainer *container)
{
  std::cout<< container_name << std::endl;
  for(uint32_t i=0 ; i <= container->GetN()-1 ; i++)
  {
    Ptr<Node> object = container->Get(i);
    Ptr<MobilityModel> position = object->GetObject <MobilityModel> ();
    NS_ASSERT(position != 0);
    Vector pos = position->GetPosition ();
    std::cout << " Nodo: " << container->Get(i)->GetId()
              << " Position: (" << pos.x << ", " << pos.y <<", " << pos.z << ")" << std::endl;
  }
  std::cout << std::endl;
}

int main (int argc, char *argv[])
{      
    //constantes"""
    std::string phyMode ("DsssRate1Mbps");  

    //variables modificables por comando
    bool printPos = false;

    CommandLine cmd;

    cmd.AddValue("printPos", "Imprimir posiciones de nodos", printPos);


    //Crear contendedores
    NodeContainer APNodes;
    APNodes.Create(2);

    NodeContainer PLCNodes;
    PLCNodes.Create(10);

    NodeContainer p2pNodes;
    p2pNodes.Add(APNodes.Get(0));
    p2pNodes.Add(PLCNodes.Get(0));

    //Conectar p2p entre nodos 0 (router/modem) y 2(PLC)
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install (p2pNodes);

    //---------------------------------------------------------------------------------------------
    //Crear y Configurar Red Wifi
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    channel.AddPropagationLoss ("ns3::RangePropagationLossModel");

    YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
    phy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
    phy.SetChannel (channel.Create ());

    WifiHelper wifi;
    wifi.SetRemoteStationManager ("ns3::IdealWifiManager");

    // 802.11b STA
    wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

    WifiMacHelper wifiMac;
    Ssid ssid = Ssid ("ns-3-ssid");

    NetDeviceContainer devices;

    wifiMac.SetType ("ns3::ApWifiMac",
                    "Ssid", SsidValue (ssid));

    NetDeviceContainer apDevices = wifi.Install (phy, wifiMac, APNodes);
    devices.Add (apDevices);

    //-----------------------------------------------------------------------------
    //RED PLC ("lan csma")
    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
    csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));

    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install (PLCNodes); 

    devices.Add (csmaDevices);              

    //--------------------------------------------------------------------------------------
    //Asignar Posiciones de Nodos
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    positionAlloc->Add (Vector (60.0, 55.6, 0));    //0 - Router /Modem - AP
    positionAlloc->Add (Vector (61.0, 02.0, 0));    //1 - AP
    positionAlloc->Add (Vector (60.0, 59.0, 0));    //2 - PLC Conectado a Router via "cable"
    positionAlloc->Add (Vector (40.5, 72.5, 0));    //3 - PLC
    positionAlloc->Add (Vector (12.7, 72.5, 0));    //4 - PLC
    positionAlloc->Add (Vector (02.0, 48.4, 0));    //5 - PLC
    positionAlloc->Add (Vector (24.5, 39.0, 0));    //6 - PLC
    positionAlloc->Add (Vector (17.1, 38.7, 0));    //7 - PLC
    positionAlloc->Add (Vector (06.4, 02.0, 0));    //8 - PLC
    positionAlloc->Add (Vector (25.6, 02.0, 0));    //9 - PLC
    positionAlloc->Add (Vector (13.0, 53.4, 0));    //10- PLC
    positionAlloc->Add (Vector (52.0, 02.0, 0));    //11- PLC
    positionAlloc->Add (Vector (64.0, 33.8, 0));    //12- PLC
    mobility.SetPositionAllocator (positionAlloc);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (APNodes);
    mobility.Install (PLCNodes);

    if (printPos)
    {
        PrintPosition("AP", &APNodes);
        PrintPosition("PLC", &PLCNodes);
    }

    //Config Internet
    InternetStackHelper internet;
    internet.Install (APNodes);
    internet.Install (PLCNodes);

    Ipv4AddressHelper ipv4;

    //AP
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer apInterface = ipv4.Assign(apDevices);

    //p2p
    ipv4.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces = ipv4.Assign (p2pDevices);

    //PLC
    ipv4.SetBase ("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterface = ipv4.Assign(csmaDevices);

    return 0;
}