/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//
// Copyright (c) 2006 Georgia Tech Research Corporation
//           (c) 2013 University of Arizona
//           (c) 2013 University of California, Los Angeles
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: Rajib Bhattacharjea<raj.b@gatech.edu>
//         Cheng Yi <yic@email.arizona.edu>
//         Alexander Afanasyev <alexander.afanasyev@ucla.edu>
//

// Georgia Tech Network Simulator - Round Trip Time Estimation Class
// George F. Riley.  Georgia Tech, Spring 2002

#include "ndn-rtt-mean-deviation.hpp"
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "ns3/integer.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE("ndn.RttMeanDeviation");

namespace ns3 {
namespace ndn {

//---------------------------------------------------------------------------------
// A modified version of Mean-Deviation Estimator optimized for NDN packet delivery

NS_OBJECT_ENSURE_REGISTERED(RttMeanDeviation);

TypeId
RttMeanDeviation::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::RttMeanDeviation")
      .SetParent<RttEstimator>()
      .AddConstructor<RttMeanDeviation>()
      .AddAttribute("Gain", "Gain used in estimating the RTT (smoothed RTT), must be 0 < Gain < 1",
                    DoubleValue(0.125), MakeDoubleAccessor(&RttMeanDeviation::m_gain),
                    MakeDoubleChecker<double>())
      .AddAttribute("Gain2", "Gain2 used in estimating the RTT (variance), must be 0 < Gain2 < 1",
                    DoubleValue(0.25), MakeDoubleAccessor(&RttMeanDeviation::m_gain2),
                    MakeDoubleChecker<double>());
  return tid;
}

RttMeanDeviation::RttMeanDeviation()
  : m_variance(0)
{
  NS_LOG_FUNCTION(this);
}

RttMeanDeviation::RttMeanDeviation(const RttMeanDeviation& c)
  : RttEstimator(c)
  , m_gain(c.m_gain)
  , m_gain2(c.m_gain2)
  , m_variance(c.m_variance)
{
  NS_LOG_FUNCTION(this);
}

TypeId
RttMeanDeviation::GetInstanceTypeId(void) const
{
  return GetTypeId();
}

void
RttMeanDeviation::Measurement(Time m)
{
  NS_LOG_FUNCTION(this << m);
  if (m_nSamples)
  { // Not first
    Time err(m - m_currentEstimatedRtt);
    double gErr = err.ToDouble(Time::S) * m_gain;
    m_currentEstimatedRtt += Time::FromDouble(gErr, Time::S);
    Time difference = Abs(err) - m_variance;
    NS_LOG_DEBUG(
      "m_variance += " << Time::FromDouble(difference.ToDouble(Time::S) * m_gain2, Time::S));
    m_variance += Time::FromDouble(difference.ToDouble(Time::S) * m_gain2, Time::S);
  }
  else
  {                       // First sample
    m_currentEstimatedRtt = m; // Set estimate to current
    // variance = sample / 2;               // And variance to current / 2
    // m_variance = m; // try this  why????
    m_variance = Seconds(m.ToDouble(Time::S) / 2);
    NS_LOG_DEBUG("(first sample) m_variance += " << m);
  }
  m_nSamples++;
}

Time
RttMeanDeviation::RetransmitTimeout()
{
  NS_LOG_FUNCTION(this);

  double retval = std::min(m_maxRto.ToDouble(Time::S),
                           std::max(m_multiplier * m_minRto.ToDouble(Time::S),
                                    m_multiplier * (m_currentEstimatedRtt.ToDouble(Time::S)
                                                    + 4 * m_variance.ToDouble(Time::S))));

  NS_LOG_DEBUG("RetransmitTimeout:  return " << retval);

  return Seconds(retval);
}

Ptr<RttEstimator>
RttMeanDeviation::Copy() const
{
  NS_LOG_FUNCTION(this);
  return CopyObject<RttMeanDeviation>(this);
}

void
RttMeanDeviation::Reset()
{
  NS_LOG_FUNCTION(this);
  // Reset to initial state
  m_variance = Seconds(0);
  RttEstimator::Reset();
}

void
RttMeanDeviation::Gain(double g)
{
  NS_LOG_FUNCTION(this);
  NS_ASSERT_MSG((g > 0) && (g < 1),
                "RttMeanDeviation: Gain must be less than 1 and greater than 0");
  m_gain = g;
}

void
RttMeanDeviation::SentSeq(SequenceNumber32 seq, uint32_t size)
{
  NS_LOG_FUNCTION(this << seq << size);
  //cout<<"RttMeanDeviation::SentSeq"<<endl;

  RttHistory_t::iterator i;
  for (i = m_history.begin(); i != m_history.end(); ++i) {
    if (seq == i->seq) { // Found it
      i->retx = true;
      break;
    }
  }

  // Note that a particular sequence has been sent
  if (i == m_history.end())
    m_history.push_back(RttHistory(seq, size, Simulator::Now()));
}

//======================================================
//Yuwei
void
RttMeanDeviation::SetInterestInfo(Name name, SequenceNumber32 seq, uint32_t size, Time rto)
{
	NS_LOG_FUNCTION(this << seq << size);
	//cout<<"RttEstimator=="<<name.toUri()<<endl;

	RttHistory_t::iterator i;
	for (i = m_history.begin(); i != m_history.end(); ++i)
	{
		if (seq == i->seq)
	    { // Found it
	      i->retx = true;
	      break;
	    }
	}

	// Note that a particular sequence has been sent
	if (i == m_history.end())
	    m_history.push_back(RttHistory(seq, size, Simulator::Now(), rto, name));
}
//----------------------------------------------------------------------------------------------------------------
void
RttMeanDeviation::DiscardInterestBySeq(SequenceNumber32 disSeq)
{
	for (RttHistory_t::iterator i = m_history.begin(); i != m_history.end(); ++i)
	{
		if (disSeq == i->seq)
	    {
			//cout<<"Discarding "<<disSeq<<"...."<<endl;
			m_history.erase(i);             //rtt is nor calculated for retransmitted data
			break;
	    }
	}
}
//----------------------------------------------------------------------------------------------------------------
Time
RttMeanDeviation::CalRTObyCorrelativity(Name name)
{
	//SiYan Yao
	CoHistory_t coList;
	CoHistory_t::iterator i;
	double sc=0.0,ac=0.0,tc=0.0,rtoValue=0.0;
	double scSum=0.0, acSum=0.0, tcSum=0.0;
	Time tmpRtt;
	Time timeDiff;

	if(m_history.size()==0)
	{
		//return Seconds(0.0);
		rtoValue=0.0;
	}
	else
	{
		//Update with 3 minutes
		UpateRttHistory(3.0);
		for (RttHistory_t::iterator i = m_history.begin(); i != m_history.end(); ++i)
		{
			if(i->rcvTime > i->time)
			{
				timeDiff = Simulator::Now() - i->rcvTime;
				if(timeDiff.ToDouble(Time::MIN)<=3)  //3 minutes
				{
					//cout<<"Have received..."<<i->name;
					sc=name.getAppcorrelativityWith(i->name);
					ac=name.getSpcorrelativityWith(i->name);
					tc=GetTmpcorrelativity(Simulator::Now(), i->rcvTime);
					//cout<<"sc="<<sc;
					//cout<<",ac="<<ac;
					//cout<<",tc="<<tc<<endl;
					//----------------------------------------------------------------------------------
					if(sc>0.0 || ac>0.0)
					{
						tmpRtt = i->rcvTime-i->time;
						coList.push_back(CorrelativityRcd(sc,ac,tc,tmpRtt));
						scSum+=sc;
						acSum+=ac;
						tcSum+=tc;
					}
				}
			}
		}
		//------------------------------------------------------------------------------------------------------
		if(coList.size()>0)
		{
			//cout<<",scSum="<<scSum;
			//cout<<",acSum="<<acSum;
			//cout<<",tcSum="<<tcSum<<endl;
			for(i=coList.begin();i!=coList.end();++i)
			{
				//cout<<i->rtt.ToDouble(Time::S)<<endl;
				rtoValue+=i->rtt.ToDouble(Time::S)*(i->sCo*acSum*tcSum+i->aCo*scSum*tcSum+i->tCo*scSum*acSum)/(3.0*scSum*acSum*tcSum);
			}
		}
	}

	double retval = std::min(m_maxRto.ToDouble(Time::S),
	                          std::max(m_minRto.ToDouble(Time::S), rtoValue));
	return Seconds(retval);
}
//=====================================================

Time
RttMeanDeviation::AckSeq(Name name, SequenceNumber32 ackSeq)
//RttMeanDeviation::AckSeq(SequenceNumber32 ackSeq)
{
  NS_LOG_FUNCTION(this << ackSeq);
  //cout<<"I'm in AckSeq!!!!"<<endl;
  // An ack has been received, calculate rtt and log this measurement
  // Note we use a linear search (O(n)) for this since for the common
  // case the ack'ed packet will be at the head of the list
  Time m = Seconds(0.0);
  if (m_history.size() == 0)
    return (m); // No pending history, just exit

  for (RttHistory_t::iterator i = m_history.begin(); i != m_history.end(); ++i)
  {
    if (ackSeq == i->seq)
    {
      //------------------------------------------------
      if(name != i->name)
      {
    	cout<<"NAME DISMATCH ERROR"<<endl;
    	cout<<name<<endl;
    	cout<<i->name<<endl;
    	cout<<"----------------------------------------------------------------------------"<<endl;
      }
      // Found it
      if (!i->retx)
      {
    	i->rcvTime = Simulator::Now();
    	//---------------------------------------------------------------------------------------------
        m = i->rcvTime - i->time; // Elapsed time
        //cout<<i->time<<endl;
        //cout<<i->rcvTime<<endl;
        //cout<<m.ToDouble(Time::MS)<<endl;
        //cout<<"-----------------------------------"<<endl;
        //---------------------------------------------------------------------------------------------
        Measurement(m);                      // Log the measurement
        ResetMultiplier();              // Reset multiplier on valid measurement
      }
      else
      {
    	  cout<<"Seq="<<i->seq<<" was retransmitted !!!"<<endl;
    	  //retransmit packet will not longer record
    	  m_history.erase(i);
      }
      break;
    }
  }
  return m;
}

/*
Time
RttMeanDeviation::GetRtobySeq(SequenceNumber32 seq)
{
	Time rto(0);
	for (RttHistory_t::iterator i = m_history.begin(); i != m_history.end(); ++i)
	{
		if(i->seq == seq)
		{
			rto =i->rto;
			break;
		}
	}
	return rto;
}*/

//======================================================
//Yuwei
Time
RttMeanDeviation::AckInterest(SequenceNumber32 ackSeq)
{
	/*
	  cout<<"I'm in AckInterest!"<<endl;
	  //-------------------------------------------------------------------------------------
	  Time m = Seconds(0.0);
	  if (m_history.size() == 0)
	    return (m); // No pending history, just exit

	  for (RttHistory_t::iterator i = m_history.begin(); i != m_history.end(); ++i)
	  {
	    if (ackSeq == i->seq)
	    { // Found it
	      //-------------------------------------------------------------------------
	    	if(name != i->name)
	    	{
	    		cout<<"Error!!!";
	    	}
	      //-------------------------------------------------------------------------
	      if (!i->retx)
	      {
	        m = Simulator::Now() - i->time; // Elapsed time
	        Measurement(m);                      // Log the measurement
	        ResetMultiplier();              // Reset multiplier on valid measurement
	      }
	      m_history.erase(i);             //rtt is nor calculated for retransmitted data
	      break;
	    }
	  }*/
	 cout<<"I'm in AckInterest!"<<endl;
	 Time m = Seconds(0.0);
	 return m;
}
//======================================================
} // namespace ndn
} // namespace ns3
