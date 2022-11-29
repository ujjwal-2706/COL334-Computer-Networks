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

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/stats-module.h"
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SeventhScriptExample");

// ===========================================================================
//
//         node 0                 node 1
//   +----------------+    +----------------+
//   |    ns-3 TCP    |    |    ns-3 TCP    |
//   +----------------+    +----------------+
//   |    10.1.1.1    |    |    10.1.1.2    |
//   +----------------+    +----------------+
//   | point-to-point |    | point-to-point |
//   +----------------+    +----------------+
//           |                     |
//           +---------------------+
//                5 Mbps, 2 ms
//
//
// We want to look at changes in the ns-3 TCP congestion window.  We need
// to crank up a flow and hook the CongestionWindow attribute on the socket
// of the sender.  Normally one would use an on-off application to generate a
// flow, but this has a couple of problems.  First, the socket of the on-off
// application is not created until Application Start time, so we wouldn't be
// able to hook the socket (now) at configuration time.  Second, even if we
// could arrange a call after start time, the socket is not public so we
// couldn't get at it.
//
// So, we can cook up a simple version of the on-off application that does what
// we want.  On the plus side we don't need all of the complexity of the on-off
// application.  On the minus side, we don't have a helper, so we have to get
// a little more involved in the details, but this is trivial.
//
// So first, we create a socket and do the trace connect on it; then we pass
// this socket into the constructor of our simple application which we then
// install in the source node.
// ===========================================================================
//
class MyApp : public Application
{
public:
  MyApp ();
  virtual ~MyApp ();

  /**
   * Register this type.
   * \return The TypeId.
   */
  static TypeId GetTypeId (void);
  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

MyApp::MyApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_nPackets (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

MyApp::~MyApp ()
{
  m_socket = 0;
}

/* static */
TypeId MyApp::GetTypeId (void)
{
  static TypeId tid = TypeId ("MyApp")
    .SetParent<Application> ()
    .SetGroupName ("Tutorial")
    .AddConstructor<MyApp> ()
    ;
  return tid;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  if (InetSocketAddress::IsMatchingType (m_peer))
    {
      m_socket->Bind ();
    }
  else
    {
      m_socket->Bind6 ();
    }
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

void
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}

static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  // NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << " " << newCwnd);
  *stream->GetStream () << Simulator::Now ().GetSeconds () << " " << oldCwnd << " " << newCwnd << std::endl;
}


int
main (int argc, char *argv[])
{
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewRenoPlus"));
  bool useV6 = false;

  CommandLine cmd;
  cmd.AddValue ("useIpv6", "Use Ipv6", useV6);
  cmd.Parse (argc, argv);
  
  NodeContainer nodes;
  nodes.Create (3);

  PointToPointHelper pointToPoint1;
  PointToPointHelper pointToPoint2;

  pointToPoint1.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  pointToPoint1.SetChannelAttribute ("Delay", StringValue ("3ms"));

  pointToPoint2.SetDeviceAttribute ("DataRate", StringValue ("9Mbps"));
  pointToPoint2.SetChannelAttribute ("Delay", StringValue ("3ms"));

  std::vector<NodeContainer> p2pConnectionList(2);
  p2pConnectionList[0] = NodeContainer(nodes.Get(0),nodes.Get(2));
  p2pConnectionList[1] = NodeContainer(nodes.Get(1),nodes.Get(2));

  std::vector<NetDeviceContainer> netDevices(2);
  netDevices[0] = pointToPoint1.Install(p2pConnectionList[0]);
  netDevices[1] = pointToPoint2.Install(p2pConnectionList[1]);


  // set the error rate model
  Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  em->SetAttribute ("ErrorRate", DoubleValue (0.00001));
  netDevices[0].Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
  netDevices[1].Get(1)->SetAttribute("ReceiveErrorModel",PointerValue(em)); // check this

  InternetStackHelper stack;
  stack.Install (nodes);
  
  uint16_t sinkPort1 = 8070;
  uint16_t sinkPort2 = 8080;
  uint16_t sinkPort3 = 8090;
  Address sinkAddress1,sinkAddress2,sinkAddress3;
  Address anyAddress1,anyAddress2,anyAddress3;

  std::string probeType;
  std::string tracePath;
  Ipv4AddressHelper address;
  probeType = "ns3::Ipv4PacketProbe";
  tracePath = "/NodeList/*/$ns3::Ipv4L3Protocol/Tx";

  std::vector<Ipv4InterfaceContainer> interfaceList (2);
  for(int i =0;i < 2;i++)
  {
    std::ostringstream subnet;
    subnet<<"10.1."<<i+1<<".0";
    address.SetBase (subnet.str ().c_str (), "255.255.255.0");
    interfaceList[i] = address.Assign (netDevices[i]);
  }
  sinkAddress1 = InetSocketAddress (interfaceList[0].GetAddress (1), sinkPort1);
  anyAddress1 = InetSocketAddress (Ipv4Address::GetAny (), sinkPort1);

  sinkAddress2 = InetSocketAddress (interfaceList[0].GetAddress (1), sinkPort2);
  anyAddress2 = InetSocketAddress (Ipv4Address::GetAny (), sinkPort2);
  
  sinkAddress3 = InetSocketAddress (interfaceList[0].GetAddress (1), sinkPort3);
  anyAddress3 = InetSocketAddress (Ipv4Address::GetAny (), sinkPort3);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  PacketSinkHelper packetSinkHelper1 ("ns3::TcpSocketFactory", anyAddress1);
  ApplicationContainer sinkApps1 = packetSinkHelper1.Install (nodes.Get (2));
  sinkApps1.Start (Seconds (0.));
  sinkApps1.Stop (Seconds (30.0));

  PacketSinkHelper packetSinkHelper2 ("ns3::TcpSocketFactory", anyAddress2);
  ApplicationContainer sinkApps2 = packetSinkHelper2.Install (nodes.Get (2));
  sinkApps2.Start (Seconds (0.));
  sinkApps2.Stop (Seconds (30.0));

  PacketSinkHelper packetSinkHelper3 ("ns3::TcpSocketFactory", anyAddress3);
  ApplicationContainer sinkApps3 = packetSinkHelper3.Install (nodes.Get (2));
  sinkApps3.Start (Seconds (0.));
  sinkApps3.Stop (Seconds (30.0));

  // server side 3 sinks created above

  Ptr<Socket> ns3TcpSocket1 = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());
  Ptr<MyApp> app1 = CreateObject<MyApp> ();
  app1->Setup (ns3TcpSocket1, sinkAddress1, 3000, 100000, DataRate ("1.5Mbps"));
  nodes.Get (0)->AddApplication (app1);
  app1->SetStartTime (Seconds (1.0));
  app1->SetStopTime (Seconds (20.0));

  Ptr<Socket> ns3TcpSocket2 = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());
  Ptr<MyApp> app2 = CreateObject<MyApp> ();
  app2->Setup (ns3TcpSocket2, sinkAddress2, 3000, 100000, DataRate ("1.5Mbps"));
  nodes.Get (0)->AddApplication (app2);
  app2->SetStartTime (Seconds (5.0));
  app2->SetStopTime (Seconds (25.0));

  Ptr<Socket> ns3TcpSocket3 = Socket::CreateSocket (nodes.Get (1), TcpSocketFactory::GetTypeId ());
  Ptr<MyApp> app3 = CreateObject<MyApp> ();
  app3->Setup (ns3TcpSocket3, sinkAddress3, 3000, 100000, DataRate ("1.5Mbps"));
  nodes.Get (1)->AddApplication (app3);
  app3->SetStartTime (Seconds (15.0));
  app3->SetStopTime (Seconds (30.0));

  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> stream1 = asciiTraceHelper.CreateFileStream ("socket1.txt");
  ns3TcpSocket1->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream1));

  Ptr<OutputStreamWrapper> stream2 = asciiTraceHelper.CreateFileStream ("socket2.txt");
  ns3TcpSocket2->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream2));

  Ptr<OutputStreamWrapper> stream3 = asciiTraceHelper.CreateFileStream ("socket3.txt");
  ns3TcpSocket3->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream3));


  Simulator::Stop (Seconds (30));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

