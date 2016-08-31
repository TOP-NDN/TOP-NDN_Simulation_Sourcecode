/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006,2007 INRIA
 * Copyright (c) 2013 Dalian University of Technology
 *
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
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 * Author: Junling Bu <linlinjavaer@gmail.com>
 *
 */
/**
 * This example shows basic construction of an 802.11p node.  Two nodes
 * are constructed with 802.11p devices, and by default, one node sends a single
 * packet to another node (the number of packets and interval between
 * them can be configured by command-line arguments).  The example shows
 * typical usage of the helper classes for this mode of WiFi (where "OCB" refers
 * to "Outside the Context of a BSS")."
 */

#include "ns3/vector.h"
#include "ns3/string.h"
#include "ns3/socket.h"
#include "ns3/double.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/command-line.h"
#include "ns3/mobility-model.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/position-allocator.h"
#include "ns3/mobility-helper.h"
//#include "ns3/internet-stack-helper.h"
//#include "ns3/ipv4-address-helper.h"
//#include "ns3/ipv4-interface-container.h"
#include <iostream>

#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"

#include "ns3/ns2-mobility-helper.h"

//liutingting
/*#include "ns3/random-variable-stream.h"
#include "ns3/rng-seed-manager.h"
*/
namespace ns3{

//NS_LOG_COMPONENT_DEFINE ("myScript");

/*
 * In WAVE module, there is no net device class named like "Wifi80211pNetDevice",
 * instead, we need to use Wifi80211pHelper to create an object of
 * WifiNetDevice class.
 *
 * usage:
 *  NodeContainer nodes;
 *  NetDeviceContainer devices;
 *  nodes.Create (2);
 *  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
 *  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
 *  wifiPhy.SetChannel (wifiChannel.Create ());
 *  NqosWaveMacHelper wifi80211pMac = NqosWave80211pMacHelper::Default();
 *  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
 *  devices = wifi80211p.Install (wifiPhy, wifi80211pMac, nodes);
 *
 * The reason of not providing a 802.11p class is that most of modeling
 * 802.11p standard has been done in wifi module, so we only need a high
 * MAC class that enables OCB mode.
 */

/*
void ReceivePacket (Ptr<Socket> socket)
{
  while (socket->Recv ())
    {
      NS_LOG_UNCOND ("Received one packet!");
    }
}

static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
                             uint32_t pktCount, Time pktInterval )
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
*/

int main (int argc, char *argv[])
{

	/***********************************************************************************/
	  uint32_t nodeNum = 241;//
	/***********************************************************************************/
	  double stopTime = 1798.0;//8.0;
	/***********************************************************************************/
	 //double per = 0.4;


  //std::string phyMode ("OfdmRate6MbpsBW10MHz");
 // uint32_t packetSize = 1000; // bytes
 //uint32_t numPackets = 1;
  //double interval = 1.0; // seconds
  //bool verbose = false;

  CommandLine cmd;

  /*cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
  cmd.AddValue ("numPackets", "number of packets generated", numPackets);
  cmd.AddValue ("interval", "interval (seconds) between packets", interval);
  cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);*/
  cmd.Parse (argc, argv);
  // Convert to time object
  //Time interPacketInterval = Seconds (interval);

  NodeContainer c;
  c.Create (nodeNum);


  // The below set of helpers will help us to put together the wifi NICs we want
  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  Ptr<YansWifiChannel> channel = wifiChannel.Create ();
  wifiPhy.SetChannel (channel);
  // ns-3 supports generate a pcap trace
  //wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);
  NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
  /*if (verbose)
    {
      wifi80211p.EnableLogComponents ();      // Turn on all Wifi 802.11p logging
    }*/

  /*wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                      "DataMode",StringValue (phyMode),
                                      "ControlMode",StringValue (phyMode));*/
  NetDeviceContainer devices = wifi80211p.Install (wifiPhy, wifi80211pMac, c);

  // Tracing
  //wifiPhy.EnablePcap ("myScript", devices);

 /*
MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (50.0, 50.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (c);
*/

  ns3::Ns2MobilityHelper ns2helper("Liutingting/scene1.tcl");
  ns2helper.Install();


  /*
  InternetStackHelper internet;
  internet.Install (c);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (0), tid);
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
  recvSink->Bind (local);
  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket (c.Get (1), tid);
  InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
  source->SetAllowBroadcast (true);
  source->Connect (remote);

  Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
                                  Seconds (1.0), &GenerateTraffic,
                                  source, packetSize, numPackets, interPacketInterval);
*/

  // Install NDN stack on all nodes
   //NS_LOG_UNCOND("Installing NDN stack");//liutingting 2016.1.7
   ndn::StackHelper ndnHelper;
   ndnHelper.SetDefaultRoutes(true);
   //ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
   //ndnHelper.setCsSize (2000);
   ndnHelper.InstallAll();

   // Choosing forwarding strategy
   ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/best-route");

  // Installing applications


    // Consumer
    ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
    // Consumer will request /prefix/0, /prefix/1, ...
    consumerHelper.SetPrefix("/prefix");
    consumerHelper.SetAttribute("Frequency", StringValue("1.0")); // 1 interests a second
    consumerHelper.Install(c.Get(0));
    //std::set<uint32_t> select;

      /*RngSeedManager::SetSeed();
    ns3::Ptr<ns3::UniformRandomVariable> r=ns3::CreateObject<ns3::UniformRandomVariable>();
    r->SetAttribute("Max", nodeNum-1);
    r->SetAttribute("Min", 0);
*/

/*
    srand(0);
    for(uint32_t i = 0;i < (nodeNum - 1) * per; i++)
    {
    	uint32_t num = rand() % (nodeNum-1);
    	while(select.find(num) != select.end())
    	{
    		num = rand() % (nodeNum-1);
    	}
    	select.insert(num);
    	//consumerHelper.Install(c.Get(num));
    	//cout<<num<<" ";
    }

    ifstream infile("consumer.txt");
    char line[21];
    string strLine;
    infile.getline(line,20);
    while(!infile.eof())
    {
    		strLine = line;
    		int num = atoi(strLine.c_str());
    		consumerHelper.Install(c.Get(num)).Start(Seconds(60.0));
                //std::cout<<num<<std::endl;
    		infile.getline(line,20);
    }
    //cout<<endl;
    infile.close();*/


    // Producer
    ndn::AppHelper producerHelper("ns3::ndn::Producer");
    // Producer will reply to all requests starting with /prefix
    producerHelper.SetPrefix("/prefix");
    producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
    producerHelper.Install(c.Get(nodeNum-1)); // last node

    //liutingting 2016.1.7
    ndn::AppDelayTracer::InstallAll("Liutingting/app-delays-trace.txt");
    ndn::L3RateTracer::InstallAll("Liutingting/rate-trace.txt", Seconds(10.0));
   //------------

    Simulator::Stop(Seconds(stopTime));

    Simulator::Run();
    Simulator::Destroy();
    //NS_LOG_UNCOND("simulation end");//liutingting 2016.1.14

    cout<<"Simulation End !"<<endl;
    return 0;
}
}

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
