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

NS_LOG_COMPONENT_DEFINE ("BasicInfra2");

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

int main (int argc, char *argv[])
{
    std::string phyMode ("DsssRate1Mbps");
    double rss = -30;  // -dBm
    uint32_t packetSize = 1000; // bytes
    uint32_t numPackets = 3;
    double interval = 1.0; // seconds
    bool verbose = false;
    uint32_t nAp = 1;

    CommandLine cmd;
    cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
    cmd.AddValue ("rss", "received signal strength", rss);
    cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
    cmd.AddValue ("numPackets", "number of packets generated", numPackets);
    cmd.AddValue ("interval", "interval (seconds) between packets", interval);
    cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
    cmd.AddValue ("nAp", "numero de Access Point", nAp);
    
    cmd.Parse (argc, argv);

    // Convert to time object
    Time interPacketInterval = Seconds (interval);

    // Fix non-unicast data rate to be the same as that of unicast
    Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                        StringValue (phyMode));

    //CREAR NODOS AP  
    NodeContainer WifiApNodes;
    WifiApNodes.Create(nAp);

    //CREAR NODOS STA
    NodeContainer WifiStaNodes;
    WifiStaNodes.Create(1);

    // The below set of helpers will help us to put together the wifi NICs we want
    WifiHelper wifi;
    if (verbose)
        {
        wifi.EnableLogComponents ();  // Turn on all Wifi logging
        }
    wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

    YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
    // This is one parameter that matters when using FixedRssLossModel
    // set it to zero; otherwise, gain will be added
    wifiPhy.Set ("RxGain", DoubleValue (0) );
    // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
    wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    // The below FixedRssLossModel will cause the rss to be fixed regardless
    // of the distance between the two stations, and the transmit power
    wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (rss));
    wifiPhy.SetChannel (wifiChannel.Create ());

    // Add a mac and disable rate control
    WifiMacHelper wifiMac;
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                    "DataMode",StringValue (phyMode),
                                    "ControlMode",StringValue (phyMode));

    // Setup the rest of the mac
    Ssid ssid = Ssid ("wifi-default");
    // setup sta.
    wifiMac.SetType ("ns3::StaWifiMac",
                    "Ssid", SsidValue (ssid));
                    
    NetDeviceContainer staDevice = wifi.Install (wifiPhy, wifiMac, WifiStaNodes);
    NetDeviceContainer devices = staDevice;
    // setup ap.
    wifiMac.SetType ("ns3::ApWifiMac",
                    "Ssid", SsidValue (ssid));
    NetDeviceContainer apDevice = wifi.Install (wifiPhy, wifiMac, WifiApNodes);
    devices.Add (apDevice);

    // Note that with FixedRssLossModel, the positions below are not
    // used for received signal strength.

    //Colocar Nodos AP
    MobilityHelper mobilityAP;
    Ptr<ListPositionAllocator> positionAllocAP = CreateObject<ListPositionAllocator> ();
    positionAllocAP->Add (Vector (0, 0, 0)); //Nodo AP
    positionAllocAP->Add (Vector (100, 0, 0));
    positionAllocAP->Add (Vector (0, 100, 0));
    positionAllocAP->Add (Vector (100, 100, 0));
    mobilityAP.SetPositionAllocator (positionAllocAP);
    mobilityAP.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobilityAP.Install (WifiApNodes);

    //Colocar Nodos STA
    MobilityHelper mobilitySTA;
    Ptr<ListPositionAllocator> positionAllocSTA = CreateObject<ListPositionAllocator> ();
    positionAllocSTA->Add (Vector (5, 0, 0)); //Nodo STA
    mobilitySTA.SetPositionAllocator (positionAllocSTA);
    mobilitySTA.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobilitySTA.Install (WifiStaNodes);

    //PrintPosition("AP",&WifiApNodes);
    //PrintPosition("STA", &WifiStaNodes);



    InternetStackHelper internet;
    internet.Install (WifiApNodes);
    internet.Install (WifiStaNodes);

    Ipv4AddressHelper ipv4;
    NS_LOG_INFO ("Assign IP Addresses.");
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i = ipv4.Assign (devices);

    //Nodo STA
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    Ptr<Socket> recvSink = Socket::CreateSocket (WifiStaNodes.Get (0), tid);
    InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
    recvSink->Bind (local);
    recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

    Ptr<Socket> source = Socket::CreateSocket (WifiApNodes.Get (0), tid);
    InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
    source->SetAllowBroadcast (true);
    source->Connect (remote);

    std::cout << "Source Id: " << source->GetNode()->GetId() << std::endl;
    std::cout << "Source Net Device: " << source->GetBoundNetDevice() << std::endl;
    std::cout << "Source IP: " << local.GetIpv4() << std::endl;

    std::cout << std::endl;

    std::cout << "Reciver: " << recvSink->GetNode()->GetId() << std::endl;
    std::cout << "Reciver Net Device: " << recvSink->GetBoundNetDevice() << std::endl;
    std::cout << "Reciver IP: " << remote.GetIpv4() << std::endl;

    std::cout << std::endl;
    
    // Tracing
    //wifiPhy.EnablePcap ("Basic-Infra-2", devices);

    // Output what we are doing
    NS_LOG_UNCOND ("Testing " << numPackets  << " packets sent with receiver rss " << rss );

    Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
                                    Seconds (1.0), &GenerateTraffic,
                                    source, packetSize, numPackets, interPacketInterval);
    
    

    Simulator::Stop (Seconds (10.0));
    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}
