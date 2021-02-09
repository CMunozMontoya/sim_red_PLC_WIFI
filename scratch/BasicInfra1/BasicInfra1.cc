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

NS_LOG_COMPONENT_DEFINE ("BasicInfra1");

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

void
RxTrace (std::string context, Ptr<const Packet> pkt, const Address& a, const Address& b)
{
  std::cout << context << std::endl;
  std::cout << "RxTrace: size = " << pkt->GetSize() << std::endl
            << "From Address: " << InetSocketAddress::ConvertFrom (a).GetIpv4() << std::endl
            << "Local Addres: " << InetSocketAddress::ConvertFrom (b).GetIpv4() << std::endl
            << std::endl;
}

void
TxTrace (std::string context, Ptr<const Packet> pkt, const Address& a, const Address& b)
{
  std::cout << context << std::endl;
  std::cout << "TxTrace: size = " << pkt->GetSize() << std::endl
            << "From Address: " << InetSocketAddress::ConvertFrom (a).GetIpv4() << std::endl
            << "Local Addres: " << InetSocketAddress::ConvertFrom (b).GetIpv4() << std::endl
            << std::endl;
}

void ReceivePacket (Ptr<Socket> socket)
{
  while (socket->Recv ())
    {
      NS_LOG_UNCOND ("Received one packet!");
    }
}

static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize, uint32_t pktCount, Time pktInterval )
{
  if (pktCount > 0)
    {
      socket->Send (Create<Packet> (pktSize));
      Simulator::Schedule (pktInterval, &GenerateTraffic,
                           socket, pktSize,pktCount - 1, pktInterval);
    }
  else
    {
      socket->Close ();
    }
}

int
main (int argc, char *argv[])
{
    //bool infolog = false; //imprimir lo que sucede en la simu
    uint32_t nCsma = 4; //nodos CSMA
    //uint32_t nAP = 3; //nodos Access Point
    //uint32_t nSTA = 1; //nodos estaciones moviles
    double interval = 1.0;
    double rss = -30; //-dB
    bool wifi_log = false;
    bool position_log = false;
    std::string phyMode ("DsssRate1Mbps");

    Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                        StringValue ("DsssRate1Mbps"));

    Time interPacketInterval = Seconds (interval);
    
    NodeContainer c;
    c.Create (nCsma);


    //RED LAN --------------------------------
    NodeContainer csmaNodes;
    csmaNodes.Add (c);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue ("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds (6000)));

    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install(csmaNodes);

    //-----------------------------------------

    //RED WIFI---------------------------------
    NodeContainer ApNodes;
    ApNodes.Add(c);

    //CREAR nodo STA
    NodeContainer STANodes;
    STANodes.Create (1);

    WifiHelper wifi;

    if (wifi_log)
    {
      wifi.EnableLogComponents ();  // Turn on all Wifi logging
    }
    wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
    wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

    
    YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
    
    // This is one parameter that matters when using FixedRssLossModel
    // set it to zero; otherwise, gain will be added
    phy.Set ("RxGain", DoubleValue (0));

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    channel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (rss));

    phy.SetChannel (channel.Create ());
    


    WifiMacHelper mac;

    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));

    Ssid ssid = Ssid ("Wifi-default");

    mac.SetType ( "ns3::StaWifiMac",
                  "Ssid", SsidValue (ssid),
                  "ActiveProbing",BooleanValue( false));
    NetDeviceContainer STADevices;
    STADevices = wifi.Install (phy, mac, STANodes);
    
    NetDeviceContainer devices = STADevices;


    mac.SetType ("ns3::ApWifiMac",
                   "Ssid", SsidValue (ssid));    
    NetDeviceContainer ApDevices = wifi.Install (phy, mac, ApNodes);

    devices.Add (ApDevices);
    

    //---------------------------------------------

    //Colocar Nodos AP
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    positionAlloc->Add (Vector (0, 0, 0));
    positionAlloc->Add (Vector (0, 100, 0));
    positionAlloc->Add (Vector (100, 0, 0));
    positionAlloc->Add (Vector (100, 100, 0));
    mobility.SetPositionAllocator (positionAlloc);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install(ApNodes);


    //-----------------------------------------
  
    MobilityHelper staMob;
    Ptr<ListPositionAllocator> staPosAlloc = CreateObject<ListPositionAllocator> ();
    staPosAlloc->Add (Vector (0,0,0));

    /*
    staMob.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (90),
                                 "MinY", DoubleValue (90),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));
    staMob.SetMobilityModel( "ns3::RandomWalk2dMobilityModel",
                              "Bounds", RectangleValue (Rectangle (-100,100,-100,100)));
    */
    staMob.SetPositionAllocator (staPosAlloc);
    staMob.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    staMob.Install (STANodes);


    //-----------------------------------------

    InternetStackHelper internet;
    internet.Install (csmaNodes);
    internet.Install (STANodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0","255.255.255.0");
    
    Ipv4InterfaceContainer i = address.Assign (devices);
    
    /*
    Ipv4InterfaceContainer csmaInterfaces;
    Ipv4InterfaceContainer ApInterfaces;
    Ipv4InterfaceContainer STAInterfaces;

    csmaInterfaces = address.Assign (csmaDevices);
    ApInterfaces = address.Assign (ApDevices);
    STAInterfaces = address.Assign (STADevices);
    */

    //----------------------------------

    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");

    //socket para STA
    Ptr<Socket> reciverSocket = Socket::CreateSocket (STANodes.Get(0), tid);
    InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
    reciverSocket->Bind (local);
    reciverSocket->SetRecvCallback (MakeCallback (&ReceivePacket));
    
    //socket AP
    Ptr<Socket> sourceSocket = Socket::CreateSocket (ApNodes.Get(0), tid);
    InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
    sourceSocket->SetAllowBroadcast (true);
    sourceSocket->Connect (remote);



    //------------------------------------------   
    
    if(position_log){ //para cambiar despues si se desea obtener o no la poscicion inicial
      PrintNodes("c",&c);
      PrintNodes("CSMA",&csmaNodes);
      PrintPosition("AP", &ApNodes);
      PrintPosition("STA",&STANodes);
    }

    NS_LOG_UNCOND ("Probando >> " << 3  << " paquetes enviados con RSS = " << rss );

    //Simulating
    Simulator::ScheduleWithContext (sourceSocket->GetNode ()-> GetId (),
                                    Seconds (1.0),
                                    &GenerateTraffic,
                                    sourceSocket,
                                    1000, //packet size
                                    3, //num packet
                                    interPacketInterval);
    

    Simulator::Stop (Seconds (5.0));
    Simulator::Run ();
    Simulator::Destroy ();
    

    return 0;
}