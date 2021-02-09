/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"

// Default Network Topology
//
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n4   n3   n2   n1 -------------- n0 
//                   point-to-point  | 
//                                   =
//                                     LAN 10.1.2.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");


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


int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma = 1; //nodos lan
  uint32_t nWifi = 5; //nodos wifi
  uint32_t nPkt = 3; //numero de paquetes a enviar
  uint32_t nWifiAp = 3; //numero de Access Points
  bool tracing = false;

  CommandLine cmd;
  //cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
  cmd.AddValue ("nPkt", "Cantidad de paquetes a enviar", nPkt);

  cmd.Parse (argc,argv);

  // The underlying restriction of 18 is due to the grid position
  // allocator's configuration; the grid layout will exceed the
  // bounding box if more than 18 nodes are provided.
  if (nWifi > 18)
    {
      std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box" << std::endl;
      return 1;
    }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  //crear coneccion ppp entre lan y wifi ----------
  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

  //red lan ------------
  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get (0)); //usar nodo 0 de p2pNodes
  csmaNodes.Create (nCsma); //crear N nodos 

  CsmaHelper csma; //setup de red
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);

  //red wifi --------------
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);
  NodeContainer wifiApNodes = p2pNodes.Get (1); //usar nodo 1 de p2pNodes

  //añadir nodos estaticos al wifi
  wifiApNodes.Create(nWifiAp - 1);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNodes);


  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  mobility.Install (wifiStaNodes);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNodes);


  InternetStackHelper stack;
  stack.Install (csmaNodes);
  stack.Install (wifiApNodes);
  stack.Install (wifiStaNodes);

  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);

  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (staDevices);
  address.Assign (apDevices);


  //echo ---
  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (nPkt));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (wifiStaNodes.Get (1));
  clientApps.Start (Seconds (2.0)); 
  clientApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (10.0));

  if (tracing == true)
    {
      pointToPoint.EnablePcapAll ("wifi_test");
      phy.EnablePcap ("wifi_test", apDevices.Get (0));
      csma.EnablePcap ("wifi_test", csmaDevices.Get (0), true);
    }

  //imprimir posiciones----
  //PrintPosition("wifiApNodes",&wifiApNodes);
  //PrintPosition("wifiStaNodes",&wifiStaNodes);

  Config::Connect("/NodeList/*/ApplicationList/*/$ns3::UdpEchoClient/RxWithAddresses",MakeCallback(&RxTrace));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
