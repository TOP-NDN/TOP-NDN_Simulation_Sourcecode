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
#include <stdlib.h>
#include<cstdlib>
#include "model/ndn-app-face.hpp"

NS_LOG_COMPONENT_DEFINE("ndn.ConsumerRandomCbr");

namespace ns3 {
namespace ndn {
//==================================================================================================
//Node
//by Siyan Yao
NsNode::NsNode(string str)
  : m_element(str)
{
	//m_element = str;
	NS_LOG_FUNCTION(this);
}

NsNode::~NsNode()
{
	for(list<NsNode*>::iterator it=p_childs.begin();it!=p_childs.end();it++)
	{
		cout<<"delete node"<<*it;
		delete *it;
	}
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
	list<NsNode*>::iterator child=p_childs.begin();
	for (int i = 0; i < num; i++)
	{
		if (child != p_childs.end())
			++child;
	}
	return *child;
}

unsigned short NsNode::GetChildNum(void)
{
	return p_childs.size();
}

void NsNode::Print(void)
{
	cout<<m_element<<endl;
	for (int i = 0; i < GetChildNum(); i++)
	{
		GetChildByIndex(i)->Print();
	}
}
//=============================================================
//Tree
//by Siyan Yao
NsTree::NsTree(string r)
{
	this->root = new NsNode(r);
	if(r =="A")
	{
		//root->m_element = "app";
		this->root = new NsNode("app");
	}
	if(r == "S")
	{
		//root->m_element = "addr";
		this->root = new NsNode("addr");
	}
	//Init rand
	unsigned int seed = (unsigned int) std::time(NULL);
	srand(seed);
}

NsTree::~NsTree()
{
	cout<<"delete root";
	delete root;
}

void NsTree::InitBuild(unsigned short levels, unsigned short maxChilds)
{
	this->levels = levels;
	Build(root, levels, maxChilds);
}

void NsTree::Build(NsNode * r, unsigned short levels, unsigned short maxChilds)
{
	int childNum;
	if (levels == 0)
		return;
	childNum = rand() % maxChilds+1;
	r->CreateChilds(childNum);
	for (int i = 0; i < r->GetChildNum(); i++)
	{
		Build(r->GetChildByIndex(i), levels - 1, maxChilds);
	}
}

string NsTree::GetName(NsNode * node, unsigned int levels)
{
	string tmpElement="", tmpName = "";
	if (levels == 0)
		return "";
	else
	{
		tmpElement = node->GetString();
		if(tmpElement == "app")
		{
			tmpName = "/A";
		}
		else if(tmpElement == "addr")
		{
			tmpName = "/S";
		}
		else
		{
			tmpName = "/" + node->GetString();
		}
		//--------------------------------------------------------------------------------------------
		if(node->GetChildNum() > 0)
		{
			int i = rand() % (node->GetChildNum());
			tmpName += GetName(node->GetChildByIndex(i), levels - 1);
		}
		return tmpName;
	}
}

string NsTree::GetRandomName(void)
{
	string name = GetName(root, this->GetLevels());
	return name;
}

int NsTree::GetLevels()
{
	return this->levels;
}

void NsTree::Print(void)
{
	root->Print();
}

void NsTree::BuildScene(string sc)
{
	if(sc == "scene1")
	{
		NsNode* level1, *level2;
		//=======================================================
		//addr_0, addr_1, addr_2
		root->CreateChilds(3);
		//addr_0
		level1 = root->GetChildByIndex(0);
		level1->CreateChilds(3);
		level2 = level1->GetChildByIndex(0);
		level2->CreateChilds(2);
		level2 = level1->GetChildByIndex(1);
		level2->CreateChilds(3);
		level2 = level1->GetChildByIndex(2);
		level2->CreateChilds(2);

		//addr_1
		level1 = root->GetChildByIndex(1);
		level1->CreateChilds(3);
		level2 = level1->GetChildByIndex(0);
		level2->CreateChilds(3);
		level2 = level1->GetChildByIndex(1);
		level2->CreateChilds(4);
		level2 = level1->GetChildByIndex(2);
		level2->CreateChilds(3);

		//addr_2
		level1 = root->GetChildByIndex(2);
		level1->CreateChilds(3);
		level2 = level1->GetChildByIndex(0);
		level2->CreateChilds(2);
		level2 = level1->GetChildByIndex(1);
		level2->CreateChilds(3);
		level2 = level1->GetChildByIndex(2);
		level2->CreateChilds(2);
	}
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
  , aNameTree("A")
  , sNameTree("S")
{
  NS_LOG_FUNCTION_NOARGS();
  m_seqMax = std::numeric_limits<uint32_t>::max();
  //================================================
  //Yuwei
  aNameTree.InitBuild(3,3);
  sNameTree.BuildScene("scene1");
  //sNameTree.InitBuild(4,3);
  //aNameTree.Print();
  sNameTree.Print();
  //cout<<"============================"<<endl;
}

ConsumerRandomCbr::~ConsumerRandomCbr()
{
}

void
ConsumerRandomCbr::ScheduleNextPacket()
{
	//For Random Cbr
	//Yuwei
	//===============================================================
	string prefix=sNameTree.GetRandomName()+aNameTree.GetRandomName();
	SetPrefix(prefix);
	m_isSameWithLastInterest = false;

	//===============================================================
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
