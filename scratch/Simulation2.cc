/* ****************************************************
 * This is a simulation draft for our Vehicular NDN
 * Yuwei Xu
 * 2016-8-26
**************************************************** */
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
#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/ns2-mobility-helper.h"
#include <iostream>

namespace ns3{

//NS_LOG_COMPONENT_DEFINE ("myScript");

//====================================================================================
int main (int argc, char *argv[])
{

	/***********************************************************************************/
	  uint32_t nodeNum = 1612;//
	/***********************************************************************************/
	  double stopTime = 1000.0;//8.0;
	/***********************************************************************************/
	 //double per = 0.4;
	  LogComponentEnable ("ndn.Producer", LOG_LEVEL_INFO);

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

  //1、Create nodes
  NodeContainer c;
  c.Create (nodeNum);


  // 2、Set PHY and  MAC Layer
  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  Ptr<YansWifiChannel> channel = wifiChannel.Create ();
  wifiPhy.SetChannel (channel);

  NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
  NetDeviceContainer devices = wifi80211p.Install (wifiPhy, wifi80211pMac, c);

  //3、Set mobility of nodes from a tcl file
  ns3::Ns2MobilityHelper ns2helper("Liutingting/scene2.tcl");
  ns2helper.Install();

  // 4、Install NDN stack on all nodes
  //NS_LOG_UNCOND("Installing NDN stack");
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);
  //ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
  //ndnHelper.setCsSize (2000);
  ndnHelper.InstallAll();

  // Choosing forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/multicast");

  //5、 Installing applications
  // Consumer
  //------------------------------------------------------------------------------------------
  ndn::AppHelper consumerHelper2("ns3::ndn::ConsumerRandomCbr");
  consumerHelper2.SetAttribute("Frequency", StringValue("1.0"));  //1 interests a second
  ApplicationContainer consumerApp2 = consumerHelper2.Install(c.Get(100));
  consumerApp2.Start(Seconds(32));
  consumerApp2.Stop(Seconds(419));

  //Provider
  //------------------------------------------------------------------------------------------
  //node 1600
   ndn::AppHelper producerHelper("ns3::ndn::Producer");
   producerHelper.SetPrefix("/S/addr_0/addr_0_1/addr_0_1_2/A");
   producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
   producerHelper.Install(c.Get(1600));

   //node 1601
   ndn::AppHelper producerHelper2("ns3::ndn::Producer");
   producerHelper2.SetPrefix("/S/addr_0/addr_0_1/addr_0_1_0/A");
   producerHelper2.SetAttribute("PayloadSize", StringValue("1024"));
   producerHelper2.Install(c.Get(1601));

   //node 1602
   ndn::AppHelper producerHelper3("ns3::ndn::Producer");
   producerHelper3.SetPrefix("/S/addr_0/addr_0_1/addr_0_1_1/A");
   producerHelper3.SetAttribute("PayloadSize", StringValue("1024"));
   producerHelper3.Install(c.Get(1602));

   //node 1603
   ndn::AppHelper producerHelper4("ns3::ndn::Producer");
   producerHelper4.SetPrefix("/S/addr_1/addr_1_2/addr_1_2_0/A");
   producerHelper4.SetAttribute("PayloadSize", StringValue("1024"));
   producerHelper4.Install(c.Get(1603));

   //node 1604
   ndn::AppHelper producerHelper5("ns3::ndn::Producer");
   producerHelper5.SetPrefix("/S/addr_1/addr_1_2/addr_1_2_2/A");
   producerHelper5.SetAttribute("PayloadSize", StringValue("1024"));
   producerHelper5.Install(c.Get(1604));

   //node 1605
   ndn::AppHelper producerHelper6("ns3::ndn::Producer");
   producerHelper6.SetPrefix("/S/addr_1/addr_1_2/addr_1_2_1/A");
   producerHelper6.SetAttribute("PayloadSize", StringValue("1024"));
   producerHelper6.Install(c.Get(1605));

   //node 1606
   ndn::AppHelper producerHelper7("ns3::ndn::Producer");
   producerHelper7.SetPrefix("/S/addr_1/addr_1_0/addr_1_0_1/A");
   producerHelper7.SetAttribute("PayloadSize", StringValue("1024"));
   producerHelper7.Install(c.Get(1606));

   //node 1607
   ndn::AppHelper producerHelper8("ns3::ndn::Producer");
   producerHelper8.SetPrefix("/S/addr_1/addr_1_0/addr_1_0_0/A");
   producerHelper8.SetAttribute("PayloadSize", StringValue("1024"));
   producerHelper8.Install(c.Get(1607));

   //node 1608
   ndn::AppHelper producerHelper9("ns3::ndn::Producer");
   producerHelper9.SetPrefix("/S/addr_1/addr_1_0/addr_1_0_2/A");
   producerHelper9.SetAttribute("PayloadSize", StringValue("1024"));
   producerHelper9.Install(c.Get(1608));

   //node 1609
   ndn::AppHelper producerHelper10("ns3::ndn::Producer");
   producerHelper10.SetPrefix("/S/addr_2/addr_2_1/addr_2_1_2/A");
   producerHelper10.SetAttribute("PayloadSize", StringValue("1024"));
   producerHelper10.Install(c.Get(1609));

   //node 1690
   ndn::AppHelper producerHelper11("ns3::ndn::Producer");
   producerHelper11.SetPrefix("/S/addr_2/addr_2_1/addr_2_1_0/A");
   producerHelper11.SetAttribute("PayloadSize", StringValue("1024"));
   producerHelper11.Install(c.Get(1610));

   //node 1691
   ndn::AppHelper producerHelper12("ns3::ndn::Producer");
   producerHelper12.SetPrefix("/S/addr_2/addr_2_1/addr_2_1_1/A");
   producerHelper12.SetAttribute("PayloadSize", StringValue("1024"));
   producerHelper12.Install(c.Get(1611));


  //liutingting 2016.1.7
  ndn::AppDelayTracer::InstallAll("Liutingting/app-delays-trace.txt");
  ndn::L3RateTracer::InstallAll("Liutingting/rate-trace.txt", Seconds(10.0));


  Simulator::Stop(Seconds(stopTime));

  Simulator::Run();
  Simulator::Destroy();

  cout<<"Simulation End Successfully!"<<endl;
  return 0;
}
}

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
