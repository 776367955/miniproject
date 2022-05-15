#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"




using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SecondScriptExample");
//
int main (int argc, char *argv[]) {
  bool verbose = true;
  uint32_t nCsma = 3;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc,argv);

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  nCsma = nCsma == 0 ? 1 : nCsma;

  NodeContainer p2pN;
  p2pN.Create (2);

  NodeContainer csmaN;
  csmaN.Add (p2pN.Get (1));
  csmaN.Create (nCsma);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("10ms"));

  NetDeviceContainer p2pD;
  p2pD = pointToPoint.Install (p2pN);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("50Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);

  InternetStackHelper stack;
  stack.Install (p2pNodes.Get (0));
  stack.Install (csmaNodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pI;
  p2pI = address.Assign (p2pD);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaI;
  csmaI = address.Assign (csmaDevices);

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer server = echoServer.Install (csmaNodes.Get (nCsma));
  server.Start (Seconds (1.0));
  server.Stop (Seconds (10.0));

  UdpEchoClientHelper echoC (csmaI.GetAddress (nCsma), 9);
  echoC.SetAttribute ("MaxPackets", UintegerValue (1));
  echoC.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoC.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer client = echoC.Install (p2pN.Get (0));
  client.Start (Seconds (2.0));
  client.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  pointToPoint.EnablePcapAll ("second");
  csma.EnablePcap ("second", csmaDevices.Get (1), true);

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
