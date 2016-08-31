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
#include <iostream>

#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/ndnSIM-module.h"

namespace ns3{
NS_LOG_COMPONENT_DEFINE("myTest");

int main (int argc, char *argv[])
{
  //================================================================
  uint32_t nodeNum = 241;           //Number of Vehicles
  double stopTime = 1798.0;       //Simulation Stop Time

  //================================================================
  NodeContainer c;
  c.Create (nodeNum);

  //==========================802.11p Parameters=======================
  // The below set of helpers will help us to put together the wifi NICs we want
  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  Ptr<YansWifiChannel> channel = wifiChannel.Create ();
  wifiPhy.SetChannel (channel);

  /* ns-3 supports generate a pcap trace
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);
   */

  NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();

  /*
  if (verbose)
    {
      wifi80211p.EnableLogComponents ();      // Turn on all Wifi 802.11p logging
    }

    wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                      "DataMode",StringValue (phyMode),
                                      "ControlMode",StringValue (phyMode));
                                      */

  NetDeviceContainer devices = wifi80211p.Install (wifiPhy, wifi80211pMac, c);

  /* Tracing
  wifiPhy.EnablePcap ("wave-simple-80211p", devices);
*/

  //=======================Mobility=================================
  ns3::Ns2MobilityHelper ns2helper("Liutingting/scene1.tcl");
  ns2helper.Install();


  //=========================NDN===================================
  //1、 Install NDN stack on all nodes-----------------------------------------
   NS_LOG_UNCOND("Installing NDN stack");
   ndn::StackHelper ndnHelper;
   ndnHelper.SetDefaultRoutes(true);
   ndnHelper.setCsSize (1000);            //Content Store can include 1000 packets
   ndnHelper.InstallAll();                        //Install Ndn stack on all nodes in the simulation.

   // 2、Choosing forwarding strategy
   ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/best-route");

   //3、Set Applications
   NS_LOG_INFO("Installing Applications");
   // Consumer
   ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
   // Consumer will request /prefix/0, /prefix/1, ...
   consumerHelper.SetPrefix("/prefix");
   consumerHelper.SetAttribute("Frequency", StringValue("1.0"));   // 1 interests a second
   consumerHelper.Install(c.Get(0));

   // Producer
   ndn::AppHelper producerHelper("ns3::ndn::Producer");
   // Producer will reply to all requests starting with /prefix
   producerHelper.SetPrefix("/prefix");
   producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
   producerHelper.Install(c.Get(nodeNum-1)); // last node

   //4、Tracer
   ndn::AppDelayTracer::InstallAll("Liutingting/app-delays-trace.txt");
   ndn::L3RateTracer::InstallAll("Liutingting/rate-trace.txt", Seconds(10.0));
  //================================================================
  Simulator::Stop(Seconds(stopTime));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
}// namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
