

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ssid.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThirdExample");
void CourseChange (std::string context, Ptr<const MobilityModel> model)
{
Vector position = model->GetPosition ();
NS_LOG_UNCOND (context <<
" x = " << position.x << ", y = " << position.y);
}

int main (int argc, char *argv[])
{
  uint32_t nofWifi = 3;

  bool iflog = true;
  uint32_t nofcsma = 3;
  
  CommandLine cmd (__FILE__);
  cmd.AddValue ( "Number of \"extra\" CSMA nodes/devices", nofcsma);
  cmd.AddValue ( "Number of wifi devices", nofWifi);
  cmd.AddValue ( "check echo applications wthether needs  to log or not", iflog);
 

  cmd.Parse (argc,argv);


  if (nofWifi > 18)
    {
      std::cout << "nofWifi should can not be more than18" << std::endl;
      return 1;
    }

  if (iflog)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("3Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("3ms"));

  NetDeviceContainer p2pdevice;
  p2pdevice = pointToPoint.Install (p2pNodes);

  NodeContainer csmanodes;
  csmanodes.Add (p2pNodes.Get (1));
  csmanodes.Create (nofcsma);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("50Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (5000)));

  NetDeviceContainer csmadevice;
  csmadevice = csma.Install (csmanodes);

  NodeContainer wifiStationNodes;
  wifiStationNodes.Create (nofWifi);
  NodeContainer wifiApNode = p2pNodes.Get (0);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy;
  phy.SetChannel (channel.Create ());

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");

  WifiHelper wifi;

  NetDeviceContainer staDevices;
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));
  staDevices = wifi.Install (phy, mac, wifiStationNodes);

  NetDeviceContainer apDevices;
  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));
  apDevices = wifi.Install (phy, mac, wifiApNode);

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
  mobility.Install (wifiStationNodes);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);

  InternetStackHelper stack;
  stack.Install (csmanodes);
  stack.Install (wifiApNode);
  stack.Install (wifiStationNodes);

  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pdevice);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmadevice);

  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (staDevices);
  address.Assign (apDevices);

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (csmanodes.Get (nofcsma));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nofcsma), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = 
  echoClient.Install (wifiStationNodes.Get (nofWifi - 1));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (10.0));
  phy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
  pointToPoint.EnablePcapAll ("third");
  phy.EnablePcap ("third", apDevices.Get (0));
  csma.EnablePcap ("third", csmaDevices.Get (0), true);

 std::ostringstream oss;
 oss <<
 "/NodeList/" << wifiStaNodes.Get (nWifi - 1)->GetId () <<
 "/$ns3::MobilityModel/CourseChange";
 Config::Connect (oss.str (), MakeCallback (&CourseChange));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}