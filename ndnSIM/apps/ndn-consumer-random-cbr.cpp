/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "ndn-consumer-random-cbr.hpp"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/double.h"
#include<time.h>
#include<cstdlib>
#include "model/ndn-app-face.hpp"

NS_LOG_COMPONENT_DEFINE("ndn.ConsumerRandomCbr");

namespace ns3 {
namespace ndn {
//==================================================================================================
//Yuwei
NsNode::NsNode(string str)
  : m_element(str)
{
	//m_element = str;
	NS_LOG_FUNCTION(this);
}
void NsNode::CreateChilds(unsigned short num)
{
	for (int i = 0; i < num; i++)
	{
		stringstream ss;
		ss << i;
		string str_i = m_element + "_" + ss.str();
		NsNode* child_i = new NsNode(str_i);
		p_childs.push_back(child_i);
	}
}

string NsNode::GetString(void)
{
	return m_element;
}

NsNode * NsNode::GetChildByIndex(unsigned short num)
{
	// TODO: 在此处插入 return 语句
	list<NsNode*>::iterator child=p_childs.begin();
	for (int i = 0; i < num; i++)
		if (child != p_childs.end())
			++child;
	return *child;
}

unsigned short NsNode::GetChildNum(void)
{
	return p_childs.size();
}
/*
void NsNode::SetParent(NsNode &p)
{
	parent = &p;
}
*/

NsTree::NsTree(string root)
{
	this->root = new NsNode(root);
	this->prefix = "";

	//nodes.push_back(new NsNode(root));
}

void NsTree::InitBuild(unsigned short levels, unsigned short maxChilds)
{
	this->levels = levels;
	Build(root, levels, maxChilds);
}

void NsTree::Build(NsNode * r, unsigned short levels, unsigned short maxChilds)
{
	if (levels == 0)
		return;
	r->CreateChilds(maxChilds);
	for (int i = 0; i < r->GetChildNum(); i++)
	{
		Build(r->GetChildByIndex(i), levels - 1, maxChilds);
	}
}

string NsTree::GetName(NsNode * node, unsigned int levels)
{
	//string prefix0 = "";
	if (levels == 0)
		return prefix;
	else {
		string prefix0 = "/" + node->GetString();
		//
		//cout << "ChildNum" << node->GetChildNum() << endl;
		int i = rand() % (node->GetChildNum());
		//cout << "i=" <<i<< endl;
		prefix = prefix0+GetName(node->GetChildByIndex(i), levels - 1);
			//+ "/" + node->GetString();
		//cout << prefix0 << endl;
		//cout << "prefix:" << prefix << endl;
		return prefix;
			}
	return prefix;
}


string NsTree::GetRandomName(void)
{
	//string prefix="";
	//string prefix0 = "/";
	//prefix +=prefix0+ root->GetString();

	string prefix0 = GetName(root, this->GetLevels());
	return prefix;
}

int NsTree::GetLevels()
{
	return this->levels;
}

//==================================================================================================
NS_OBJECT_ENSURE_REGISTERED(ConsumerRandomCbr);
TypeId
ConsumerRandomCbr::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::ConsumerRandomCbr")
      .SetGroupName("Ndn")
      .SetParent<Consumer>()
      .AddConstructor<ConsumerRandomCbr>()

      .AddAttribute("Frequency", "Frequency of interest packets", StringValue("1.0"),
                    MakeDoubleAccessor(&ConsumerRandomCbr::m_frequency), MakeDoubleChecker<double>())

      .AddAttribute("Randomize",
                    "Type of send time randomization: none (default), uniform, exponential",
                    StringValue("none"),
                    MakeStringAccessor(&ConsumerRandomCbr::SetRandomize, &ConsumerRandomCbr::GetRandomize),
                    MakeStringChecker())

      .AddAttribute("MaxSeq", "Maximum sequence number to request",
                    IntegerValue(std::numeric_limits<uint32_t>::max()),
                    MakeIntegerAccessor(&ConsumerRandomCbr::m_seqMax), MakeIntegerChecker<uint32_t>())

    ;

  return tid;
}

ConsumerRandomCbr::ConsumerRandomCbr()
  : m_frequency(1.0)
  , m_firstTime(true)
{
  NS_LOG_FUNCTION_NOARGS();
  m_seqMax = std::numeric_limits<uint32_t>::max();
}

ConsumerRandomCbr::~ConsumerRandomCbr()
{
}

void
ConsumerRandomCbr::ScheduleNextPacket()
{
	//Yuwei
	//srand((unsigned)time(NULL));
	NsTree tree1= NsTree("Application");
	tree1.InitBuild(3, 4);
	NsTree tree2=NsTree("Location");
	tree2.InitBuild(2,3);
	string prefix="/S"+tree2.GetRandomName()+"/A"+tree1.GetRandomName();
	//cout << tree.GetRandomName() << endl;
	//SetPrefix("/S/NankaiDistrict/FukangRoad/A/TrafficInformer/RoadCongestion");
	SetPrefix(prefix);
	//===============================================================================

  if (m_firstTime) {
    m_sendEvent = Simulator::Schedule(Seconds(0.0), &Consumer::SendPacket, this);
    m_firstTime = false;
  }
  else if (!m_sendEvent.IsRunning())
    m_sendEvent = Simulator::Schedule((m_random == 0) ? Seconds(1.0 / m_frequency)
                                                      : Seconds(m_random->GetValue()),
                                      &Consumer::SendPacket, this);
}

void
ConsumerRandomCbr::SetRandomize(const std::string& value)
{
  if (value == "uniform") {
    m_random = CreateObject<UniformRandomVariable>();
    m_random->SetAttribute("Min", DoubleValue(0.0));
    m_random->SetAttribute("Max", DoubleValue(2 * 1.0 / m_frequency));
  }
  else if (value == "exponential") {
    m_random = CreateObject<ExponentialRandomVariable>();
    m_random->SetAttribute("Mean", DoubleValue(1.0 / m_frequency));
    m_random->SetAttribute("Bound", DoubleValue(50 * 1.0 / m_frequency));
  }
  else
    m_random = 0;

  m_randomType = value;
}

std::string
ConsumerRandomCbr::GetRandomize() const
{
  return m_randomType;
}

} // namespace ndn
} // namespace ns3
