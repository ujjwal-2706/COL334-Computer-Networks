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
// int COUNT_DROP = 0;
// static void
// RxDrop (Ptr<PcapFileWrapper> file, Ptr<const Packet> p)
// {
//   // NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
//   COUNT_DROP += 1;
//   file->Write (Simulator::Now (), p);
// }


int
main (int argc, char *argv[])
{
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpVegas"));
  bool useV6 = false;

  CommandLine cmd;
  cmd.AddValue ("useIpv6", "Use Ipv6", useV6);
  cmd.Parse (argc, argv);
  
  NodeContainer nodes;
  nodes.Create (5);

  InternetStackHelper stack;
  stack.Install (nodes);


  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("500Kbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  std::vector<NodeContainer> p2pConnectionList(4);
  p2pConnectionList[0] = NodeContainer(nodes.Get(0),nodes.Get(1));
  p2pConnectionList[1] = NodeContainer(nodes.Get(1),nodes.Get(2));
  p2pConnectionList[2] = NodeContainer(nodes.Get(2),nodes.Get(3));
  p2pConnectionList[3] = NodeContainer(nodes.Get(2),nodes.Get(4));

  std::vector<NetDeviceContainer> netDevices(4);
  for(int i =0;i < 4;i++)
  {
    netDevices[i] = pointToPoint.Install(p2pConnectionList[i]);
  }
  NetDeviceContainer totalDevices;
  totalDevices.Add(netDevices[0]);
  totalDevices.Add(netDevices[1]);
  totalDevices.Add(netDevices[2]);
  totalDevices.Add(netDevices[3]);

  uint16_t sinkPort = 8080;
  Address sinkAddress;
  Address anyAddress;
  std::string probeType;
  std::string tracePath;
  Ipv4AddressHelper address;
  probeType = "ns3::Ipv4PacketProbe";
  tracePath = "/NodeList/*/$ns3::Ipv4L3Protocol/Tx";

  std::vector<Ipv4InterfaceContainer> interfaceList (4);
  for(int i =0;i < 4;i++)
  {
    std::ostringstream subnet;
    subnet<<"10.1."<<i+1<<".0";
    address.SetBase (subnet.str ().c_str (), "255.255.255.0");
    interfaceList[i] = address.Assign (netDevices[i]);
  }
  sinkAddress = InetSocketAddress (interfaceList[2].GetAddress (1), sinkPort);
  anyAddress = InetSocketAddress (Ipv4Address::GetAny (), sinkPort);
  Address udp_sink_address = InetSocketAddress(interfaceList[3].GetAddress (1),sinkPort);
  Address new_anyAddress = InetSocketAddress(Ipv4Address::GetAny(),sinkPort);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", anyAddress);
  ApplicationContainer sinkApps = packetSinkHelper.Install (nodes.Get (3));
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (100.0));

  PacketSinkHelper udpSinkHelper ("ns3::UdpSocketFactory",new_anyAddress);
  ApplicationContainer udpSinkApp = udpSinkHelper.Install(nodes.Get(4));
  udpSinkApp.Start(Seconds(0.0));
  udpSinkApp.Stop(Seconds(100.0));

  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());
  Ptr<MyApp> app = CreateObject<MyApp> ();
  app->Setup (ns3TcpSocket, sinkAddress, 1040, 100000, DataRate ("250Kbps"));
  nodes.Get (0)->AddApplication (app);
  app->SetStartTime (Seconds (1.0));
  app->SetStopTime (Seconds (100.0));

  Ptr<Socket> ns3UdpSocket1 = Socket::CreateSocket (nodes.Get (1), UdpSocketFactory::GetTypeId ());
  Ptr<MyApp> appUdp1 = CreateObject<MyApp> ();
  appUdp1->Setup (ns3UdpSocket1, udp_sink_address, 1040, 100000, DataRate ("250Kbps"));
  nodes.Get (1)->AddApplication (appUdp1);
  appUdp1->SetStartTime (Seconds (20.0));
  appUdp1->SetStopTime (Seconds (30.0));

  Ptr<Socket> ns3UdpSocket2 = Socket::CreateSocket (nodes.Get (1), UdpSocketFactory::GetTypeId ());
  Ptr<MyApp> appUdp2 = CreateObject<MyApp> ();
  appUdp2->Setup (ns3UdpSocket2, udp_sink_address, 1040, 100000, DataRate ("500Kbps"));
  nodes.Get (1)->AddApplication (appUdp2);
  appUdp2->SetStartTime (Seconds (30.0));
  appUdp2->SetStopTime (Seconds (100.0));

  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream ("task3.txt");
  ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream));

  // PcapHelper pcapHelper;
  // Ptr<PcapFileWrapper> file = pcapHelper.CreateFile ("task3.pcap", std::ios::out, PcapHelper::DLT_PPP);
  // (netDevices[0].Get(0))->TraceConnectWithoutContext ("PhyRxDrop", MakeBoundCallback (&RxDrop, file));

//   Use GnuplotHelper to plot the packet byte count over time
  GnuplotHelper plotHelper;

  // Configure the plot.  The first argument is the file name prefix
  // for the output files generated.  The second, third, and fourth
  // arguments are, respectively, the plot title, x-axis, and y-axis labels
  plotHelper.ConfigurePlot ("seventh-packet-byte-count",
                            "Packet Byte Count vs. Time",
                            "Time (Seconds)",
                            "Packet Byte Count");

  // Specify the probe type, trace source path (in configuration namespace), and
  // probe output trace source ("OutputBytes") to plot.  The fourth argument
  // specifies the name of the data series label on the plot.  The last
  // argument formats the plot by specifying where the key should be placed.
  plotHelper.PlotProbe (probeType,
                        tracePath,
                        "OutputBytes",
                        "Packet Byte Count",
                        GnuplotAggregator::KEY_BELOW);

  // Use FileHelper to write out the packet byte count over time
  FileHelper fileHelper;

  // Configure the file to be written, and the formatting of output data.
  fileHelper.ConfigureFile ("seventh-packet-byte-count",
                            FileAggregator::FORMATTED);

  // Set the labels for this formatted output file.
  fileHelper.Set2dFormat ("Time (Seconds) = %.3e\tPacket Byte Count = %.0f");

  // Specify the probe type, trace source path (in configuration namespace), and
  // probe output trace source ("OutputBytes") to write.
  fileHelper.WriteProbe (probeType,
                         tracePath,
                         "OutputBytes");
  Simulator::Stop (Seconds (100));
  pointToPoint.EnablePcapAll("task3");
  Simulator::Run ();
  Simulator::Destroy ();
  // std::cout << COUNT_DROP << std::endl;
  return 0;
}

