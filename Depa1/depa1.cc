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

    cmd.Parse (argc,argv);

    //Crear contendedores
    NodeContainer APNodes;
    APNodes.Create(2);

    NodeContainer PLCNodes;
    PLCNodes.Create(10);
    
    NodeContainer STANodes;
    STANodes.Create(14);

    //Contenedores p2p "ethernet"
    NodeContainer p2pNodes;
    p2pNodes.Add(APNodes.Get(0)); //router 1
    p2pNodes.Add(PLCNodes.Get(0)); //plc 1

    NodeContainer ethernet1;
    ethernet1.Add (APNodes.Get (0)); //router 1
    ethernet1.Add (STANodes.Get (1)); //Tele 1

    NodeContainer ethernet2;
    ethernet2.Add (APNodes.Get (0)); //router 1
    ethernet2.Add (STANodes.Get (2)); //consola

    NodeContainer ethernet3;
    ethernet3.Add (APNodes.Get (1)); //router 2
    ethernet3.Add (STANodes.Get (11)); //pc

    //-----------------------------------------------------------------------------------------------
    //Conexiones P2P    
    
    NetDeviceContainer p2pDevices;
    NetDeviceContainer ethernet1Devices;
    NetDeviceContainer ethernet2Devices;
    NetDeviceContainer ethernet3Devices;

    //Conectar p2p entre nodos 0 (router/modem) y 2(PLC)
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
    p2pDevices = pointToPoint.Install (p2pNodes);

    PointToPointHelper ethernet1p2p;
    ethernet1p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
    ethernet1p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
    ethernet1Devices = ethernet1p2p.Install (ethernet1);

    PointToPointHelper ethernet2p2p;
    ethernet2p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
    ethernet2p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
    ethernet2Devices = ethernet2p2p.Install (ethernet2);

    PointToPointHelper ethernet3p2p;
    ethernet3p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
    ethernet3p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
    ethernet3Devices = ethernet3p2p.Install (ethernet3);

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

    //devices.Add (csmaDevices);              

    //--------------------------------------------------------------------------------------

    //RED STA
    NetDeviceContainer STADevices = wifi.Install (phy, wifiMac, STANodes);
    //devices.Add (STADevices);

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

    //-----------------------------------------------------------------------------
    //Posciiones de Estaciones
    MobilityHelper STAmobility;
    Ptr<ListPositionAllocator> STApos = CreateObject<ListPositionAllocator> ();
    STApos->Add (Vector (58.9, 58.1, 0));    //0 - Asistente inteligente 1 (Living)
    STApos->Add (Vector (58.9, 68.0, 0));    //1 - Television 1 (Living)
    STApos->Add (Vector (58.9, 73.0, 0));    //2 - Consola de Videojuegos
    STApos->Add (Vector (48.0, 38.7, 0));    //3 - Alarma
    STApos->Add (Vector (25.5, 63.0, 0));    //4 - Lampara Inteligente 1 (Comedor)
    STApos->Add (Vector (25.6, 39.0, 0));    //5 - Control de Temperatura 1 (Comedor)   
    STApos->Add (Vector (64.0, 36.3, 0));    //6 - Equipo de Audio
    STApos->Add (Vector (12.7, 36.3, 0));    //7 - Television 2 (Dormitorio)
    STApos->Add (Vector (16.5, 14.4, 0));    //8 - Lampara Inteligente 2 (Dormitorio)
    STApos->Add (Vector (25.6, 14.4, 0));    //9 - Control de Temperatura 2 (Dormitorio)
    STApos->Add (Vector (06.0, 02.0, 0));    //10- Asistente inteligente 2 (Dormitorio) 
    STApos->Add (Vector (56.0, 02.5, 0));    //11- Notebook <-- Puede ser movil
    STApos->Add (Vector (49.0, 19.3, 0));    //12- Control de Temperatura 3 (Oficina)  
    STApos->Add (Vector (55.5, 19.3, 0));    //13- Lampara inteligente 3 (Oficina)
    STAmobility.SetPositionAllocator (STApos);
    STAmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    STAmobility.Install(STANodes);
    
    //------------------------------------------------
    if (printPos)
    {
        PrintPosition("AP", &APNodes);
        PrintPosition("PLC", &PLCNodes);
        PrintPosition("STA", &STANodes);
    }

    //Config Internet
    InternetStackHelper internet;
    internet.Install (APNodes);
    internet.Install (PLCNodes);
    internet.Install (STANodes);

    Ipv4AddressHelper ipv4;

    //AP
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer apInterface = ipv4.Assign (apDevices);

    //p2p
    ipv4.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces = ipv4.Assign (p2pDevices);

    //PLC
    ipv4.SetBase ("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterface = ipv4.Assign (csmaDevices);

    //STA
    ipv4.SetBase ("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer staInterface = ipv4.Assign (STADevices);

    //--------------------------------------------------------------------------------

    return 0;
}
