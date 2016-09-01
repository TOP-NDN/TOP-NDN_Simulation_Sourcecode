/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2013-2015 Regents of the University of California.
 *
 * This file is part of ndn-cxx library (NDN C++ library with eXperimental eXtensions).
 *
 * ndn-cxx library is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * ndn-cxx library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received copies of the GNU General Public License and GNU Lesser
 * General Public License along with ndn-cxx, e.g., in COPYING.md file.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * See AUTHORS.md for complete list of ndn-cxx authors and contributors.
 *
 * @author Jeff Thompson <jefft0@remap.ucla.edu>
 * @author Alexander Afanasyev <http://lasr.cs.ucla.edu/afanasyev/index.html>
 * @author Zhenkai Zhu <http://irl.cs.ucla.edu/~zhenkai/>
 */

#include "name.hpp"

#include "util/time.hpp"
#include "util/string-helper.hpp"
#include "encoding/block.hpp"
#include "encoding/encoding-buffer.hpp"

#include <boost/functional/hash.hpp>

namespace ndn {

BOOST_CONCEPT_ASSERT((boost::EqualityComparable<Name>));
BOOST_CONCEPT_ASSERT((WireEncodable<Name>));
BOOST_CONCEPT_ASSERT((WireEncodableWithEncodingBuffer<Name>));
BOOST_CONCEPT_ASSERT((WireDecodable<Name>));
static_assert(std::is_base_of<tlv::Error, Name::Error>::value,
              "Name::Error must inherit from tlv::Error");

const size_t Name::npos = std::numeric_limits<size_t>::max();

Name::Name()
  : m_nameBlock(tlv::Name)
{
}

Name::Name(const Block& wire)
{
  m_nameBlock = wire;
  m_nameBlock.parse();
}

//Create a Name from a uri (统一资源标识符)
Name::Name(const char* uri)
{
  construct(uri);
}

Name::Name(const std::string& uri)
{
  construct(uri.c_str());
}

template<encoding::Tag TAG>
size_t
Name::wireEncode(EncodingImpl<TAG>& encoder) const
{
  size_t totalLength = 0;

  for (const_reverse_iterator i = rbegin(); i != rend(); ++i)
    {
      totalLength += i->wireEncode(encoder);
    }

  totalLength += encoder.prependVarNumber(totalLength);
  totalLength += encoder.prependVarNumber(tlv::Name);
  return totalLength;
}

template size_t
Name::wireEncode<encoding::EncoderTag>(EncodingImpl<encoding::EncoderTag>& encoder) const;

template size_t
Name::wireEncode<encoding::EstimatorTag>(EncodingImpl<encoding::EstimatorTag>& encoder) const;

const Block&
Name::wireEncode() const
{
  if (m_nameBlock.hasWire())
    return m_nameBlock;

  EncodingEstimator estimator;
  size_t estimatedSize = wireEncode(estimator);

  EncodingBuffer buffer(estimatedSize, 0);
  wireEncode(buffer);

  m_nameBlock = buffer.block();
  m_nameBlock.parse();

  return m_nameBlock;
}

void
Name::wireDecode(const Block& wire)
{
  if (wire.type() != tlv::Name)
    BOOST_THROW_EXCEPTION(tlv::Error("Unexpected TLV type when decoding Name"));

  m_nameBlock = wire;
  m_nameBlock.parse();
}

void
Name::construct(const char* uriOrig)
{
  clear();

  std::string uri = uriOrig;
  trim(uri);
  if (uri.size() == 0)
    return;

  size_t iColon = uri.find(':');
  if (iColon != std::string::npos) {
    // Make sure the colon came before a '/'.
    size_t iFirstSlash = uri.find('/');
    if (iFirstSlash == std::string::npos || iColon < iFirstSlash) {
      // Omit the leading protocol such as ndn:
      uri.erase(0, iColon + 1);
      trim(uri);
    }
  }

  // Trim the leading slash and possibly the authority.
  if (uri[0] == '/') {
    if (uri.size() >= 2 && uri[1] == '/') {
      // Strip the authority following "//".
      size_t iAfterAuthority = uri.find('/', 2);
      if (iAfterAuthority == std::string::npos)
        // Unusual case: there was only an authority.
        return;
      else {
        uri.erase(0, iAfterAuthority + 1);
        trim(uri);
      }
    }
    else {
      uri.erase(0, 1);
      trim(uri);
    }
  }

  size_t iComponentStart = 0;

  // Unescape the components.
  while (iComponentStart < uri.size()) {
    size_t iComponentEnd = uri.find("/", iComponentStart);
    if (iComponentEnd == std::string::npos)
      iComponentEnd = uri.size();

    append(Component::fromEscapedString(&uri[0], iComponentStart, iComponentEnd));
    iComponentStart = iComponentEnd + 1;
  }
}

void
Name::set(const char* uri)
{
  *this = std::move(Name(uri));
}

void
Name::set(const std::string& uri)
{
  *this = std::move(Name(uri));
}

std::string
Name::toUri() const
{
  std::ostringstream os;
  os << *this;
  return os.str();
}

Name&
Name::append(const PartialName& name)
{
  if (&name == this)
    // Copying from this name, so need to make a copy first.
    return append(PartialName(name));   //？

  for (size_t i = 0; i < name.size(); ++i)
    append(name.at(i));

  return *this;
}

Name&
Name::appendNumber(uint64_t number)
{
  m_nameBlock.push_back(Component::fromNumber(number));
  return *this;
}

Name&
Name::appendNumberWithMarker(uint8_t marker, uint64_t number)
{
  m_nameBlock.push_back(Component::fromNumberWithMarker(marker, number));
  return *this;
}

Name&
Name::appendVersion(uint64_t version)
{
  m_nameBlock.push_back(Component::fromVersion(version));
  return *this;
}

Name&
Name::appendVersion()
{
  appendVersion(time::toUnixTimestamp(time::system_clock::now()).count());
  return *this;
}

Name&
Name::appendSegment(uint64_t segmentNo)
{
  m_nameBlock.push_back(Component::fromSegment(segmentNo));
  return *this;
}

Name&
Name::appendSegmentOffset(uint64_t offset)
{
  m_nameBlock.push_back(Component::fromSegmentOffset(offset));
  return *this;
}

Name&
Name::appendTimestamp(const time::system_clock::TimePoint& timePoint)
{
  m_nameBlock.push_back(Component::fromTimestamp(timePoint));
  return *this;
}

Name&
Name::appendSequenceNumber(uint64_t seqNo)
{
  m_nameBlock.push_back(Component::fromSequenceNumber(seqNo));
  return *this;
}

Name&
Name::appendImplicitSha256Digest(const ConstBufferPtr& digest)
{
  m_nameBlock.push_back(Component::fromImplicitSha256Digest(digest));
  return *this;
}

Name&
Name::appendImplicitSha256Digest(const uint8_t* digest, size_t digestSize)
{
  m_nameBlock.push_back(Component::fromImplicitSha256Digest(digest, digestSize));
  return *this;
}

PartialName
Name::getSubName(ssize_t iStartComponent, size_t nComponents) const
{
  PartialName result;

  ssize_t iStart = iStartComponent < 0 ? this->size() + iStartComponent : iStartComponent;
  size_t iEnd = this->size();

  iStart = std::max(iStart, static_cast<ssize_t>(0));

  if (nComponents != npos)
    iEnd = std::min(this->size(), iStart + nComponents);

  for (size_t i = iStart; i < iEnd; ++i)
    result.append(at(i));

  return result;
}

Name
Name::getSuccessor() const
{
  if (empty()) {
    static uint8_t firstValue[] = { 0 };
    Name firstName;
    firstName.append(firstValue, 1);
    return firstName;
  }

  return getPrefix(-1).append(get(-1).getSuccessor());
}

bool
Name::equals(const Name& name) const
{
  if (size() != name.size())
    return false;

  for (size_t i = 0; i < size(); ++i) {
    if (at(i) != name.at(i))
      return false;
  }

  return true;
}

//==============================================================
//Yuwei
double
Name::correlativityWith(const Name& name) const
{
	uint32_t MaxLength = this->size()+name.size();
	uint16_t *Pos_A = new  uint16_t [MaxLength];
	uint16_t *Pos_B = new  uint16_t [MaxLength];
	uint16_t *Pos_C = new  uint16_t [MaxLength];
	uint16_t bigger;
	uint16_t cmnLen;
	uint16_t x;

	memset(Pos_A,0,MaxLength*sizeof(uint16_t));
	memset(Pos_B,0,MaxLength*sizeof(uint16_t));
	memset(Pos_C,0,MaxLength*sizeof(uint16_t));

	size_t i, j;
	size_t h, k=0;
	//size_t tmpPosA=0, tmpPosB=0;

    double lcs = 0.0;
    double full = 0.0;
    double co = 0.0;                 //correlativity
	//------------------------------------------------------------------------------------------
	//Find the position of common element
	for(i=0; i<this->size(); i++)
	{
		for(j=0; j<name.size();j++)
		{
			if(this->at(i) == name.at(j))
			{
				Pos_A[k]= (i+1);  //tmpPosA;  From 1 to ...
				Pos_B[k]= (j+1);  //tmpPosB;
				//cout<<this->at(i)<<"------"<<name.at(j)<<endl;
				//cout<<k<<","<<Pos_A[k]<<","<<Pos_B[k]<<endl;
				k++;
			}
		}
	}
	/*
    for(h=0; h<k; h++)
    {
    	cout<<Pos_A[h]<<"--"<<Pos_B[h]<<endl;
    }
    cout<<endl;*/
	//------------------------------------------------------------------------------------------
	if(Pos_A[0] > 0 && Pos_B[0] > 0)
	{
		Pos_C[0]=(Pos_A[0]>=Pos_B[0])?Pos_A[0]:Pos_B[0];
		for(h=0; h<k-1; h++)
		{
			bigger=(Pos_A[h+1]-Pos_A[h])>=(Pos_B[h+1]-Pos_B[h])?Pos_A[h+1]-Pos_A[h]:Pos_B[h+1]-Pos_B[h];
			Pos_C[h+1]=Pos_C[h]+bigger;
		}
		bigger=(this->size()-Pos_A[k-1])>=(name.size()-Pos_B[k-1])?(this->size()-Pos_A[k-1]):(name.size()-Pos_B[k-1]);
		cmnLen = Pos_C[k-1]+bigger;
		//-----------------------------------------------------------------------------------------
	    for(h=0; h<k; h++)
	    {
	    	//cout<<Pos_C[h]<<"--";
	    	lcs+=pow(2,(-Pos_C[h]));
	    }
	    //cout<<endl;
	    //cout<<cmnLen<<endl;

	    for(x=1; x<=cmnLen; x++)
	     {
	    	full+=pow(2,(-x));
	     }
	    //-----------------------------------------------------------------------------------------
	    co = lcs/full;
	}
	else
	{
		co = 0.0;
	}
    //-----------------------------------------------------------------------------------------------
	delete[] Pos_A;
	delete[] Pos_B;
	delete[] Pos_C;
	return co;
}

size_t
Name::find(string part) const
{
	size_t i=0;
	while(this->at(i).toUri() != part)
	{
		i++;
	}
	//cout<<this->at(i)<<endl;
	return i;
}

//-----------------------------------------------------------------------------------------------------------
PartialName
Name::getAppPart() const
{
	size_t pos = this->find("A");
	Name ApplicationName=this->getSubName(pos+1); //we don't want "A" included
	//std::cout<<"ApplicationName:  "<<ApplicationName<<std::endl;
	return ApplicationName;
}

//-----------------------------------------------------------------------------------------------------------
PartialName
Name::getSpPart() const
{
	size_t pos1 = this->find("S");
	size_t pos2 = this->find("A");
	Name SpatialName=this->getSubName(pos1+1, pos2-pos1-1); //we don't want "S" included
	//std::cout<<"SpatialName:  "<<SpatialName<<std::endl;
	return SpatialName;
}
//-----------------------------------------------------------------------------------------------------------
double
Name::getAppcorrelativityWith(const Name& name) const
{
	  Name app1(this->getAppPart());
	  Name app2(name.getAppPart());
	  cout<<app1<<endl;
	  cout<<app2<<endl;
	  return app1.correlativityWith(app2);
}
//-----------------------------------------------------------------------------------------------------------
double
Name::getSpcorrelativityWith(const Name& name) const
{
	  Name sp1, sp2;
	  sp1=this->getSpPart();
	  sp2=name.getSpPart();
	  cout<<sp1.toUri()<<endl;
	  cout<<sp2.toUri()<<endl;
	  return sp1.correlativityWith(sp2);
}

//==============================================================

bool
Name::isPrefixOf(const Name& name) const
{
  // This name is longer than the name we are checking against.
  if (size() > name.size())
    return false;

  // Check if at least one of given components doesn't match.
  for (size_t i = 0; i < size(); ++i) {
    if (at(i) != name.at(i))
      return false;
  }

  return true;
}

int
Name::compare(size_t pos1, size_t count1, const Name& other, size_t pos2, size_t count2) const
{
  count1 = std::min(count1, this->size() - pos1);
  count2 = std::min(count2, other.size() - pos2);
  size_t count = std::min(count1, count2);

  for (size_t i = 0; i < count; ++i) {
    int comp = this->at(pos1 + i).compare(other.at(pos2 + i));
    if (comp != 0) { // i-th component differs
      return comp;
    }
  }
  // [pos1, pos1+count) of this Name equals [pos2, pos2+count) of other Name
  return count1 - count2;
}

std::ostream&
operator<<(std::ostream& os, const Name& name)
{
  if (name.empty())
    {
      os << "/";
    }
  else
    {
      for (Name::const_iterator i = name.begin(); i != name.end(); i++) {
        os << "/";
        i->toUri(os);
      }
    }
  return os;
}

std::istream&
operator>>(std::istream& is, Name& name)
{
  std::string inputString;
  is >> inputString;
  name = std::move(Name(inputString));

  return is;
}

} // namespace ndn

namespace std {
size_t
hash<ndn::Name>::operator()(const ndn::Name& name) const
{
  return boost::hash_range(name.wireEncode().wire(),
                           name.wireEncode().wire() + name.wireEncode().size());
}

} // namespace std
