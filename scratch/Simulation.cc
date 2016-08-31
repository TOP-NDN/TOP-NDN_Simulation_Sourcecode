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
  ns3::Ns2MobilityHelper ns2helper("Liutingting/scene1.tcl");
  ns2helper.Install();

  // 4、Install NDN stack on all nodes
  //NS_LOG_UNCOND("Installing NDN stack");
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);
  //ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
  //ndnHelper.setCsSize (2000);
  ndnHelper.InstallAll();

  // Choosing forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/best-route");

  //5、 Installing applications
  // Consumer
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  // Consumer will request /prefix/0, /prefix/1, ...
  consumerHelper.SetPrefix("/S/NankaiDistrict/WeijingRoad/A/TrafficInformer/RoadCongestion");
  consumerHelper.SetAttribute("Frequency", StringValue("1.0"));  //1 interests a second
  consumerHelper.Install(c.Get(0));

  // Producer
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  // Producer will reply to all requests starting with /prefix
  producerHelper.SetPrefix("/S/NankaiDistrict/WeijingRoad/A/TrafficInformer/RoadCongestion");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));  //1-Chunk
  producerHelper.Install(c.Get(nodeNum-1)); // last node

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
