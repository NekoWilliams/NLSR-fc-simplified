/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2025,  The University of Memphis,
 *                           Regents of the University of California,
 *                           Arizona Board of Regents.
 *
 * This file is part of NLSR (Named-data Link State Routing).
 * See AUTHORS.md for complete list of NLSR authors and contributors.
 *
 * NLSR is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NLSR is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NLSR, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "name-lsa.hpp"
#include "tlv-nlsr.hpp"

namespace nlsr {

NameLsa::NameLsa(const ndn::Name& originRouter, uint64_t seqNo,
                 const ndn::time::system_clock::time_point& timepoint,
                 const NamePrefixList& npl)
  : Lsa(originRouter, seqNo, timepoint)
{
  for (const auto& name : npl.getPrefixInfo()) {
    addName(name);
  }
}

NameLsa::NameLsa(const ndn::Block& block)
{
  wireDecode(block);
}

template<ndn::encoding::Tag TAG>
size_t
NameLsa::wireEncode(ndn::EncodingImpl<TAG>& block) const
{
  size_t totalLength = 0;

  // Encode service function chaining information
  if (!m_serviceName.empty()) {
    totalLength += prependStringBlock(block, nlsr::tlv::ServiceName, m_serviceName);
  }
  totalLength += prependNonNegativeDoubleBlock(block, nlsr::tlv::ProcessingTime, m_processingTime);
  totalLength += prependNonNegativeDoubleBlock(block, nlsr::tlv::LoadIndex, m_loadIndex);

  // Encode name prefix list
  for (const auto& name : m_npl) {
    totalLength += name.wireEncode(block);
  }

  totalLength += block.prependVarNumber(totalLength);
  totalLength += block.prependVarNumber(nlsr::tlv::NameLsa);

  return totalLength;
}

NDN_CXX_DEFINE_WIRE_ENCODE_INSTANTIATIONS(NameLsa);

const ndn::Block&
NameLsa::wireEncode() const
{
  if (m_wire.hasWire()) {
    return m_wire;
  }

  ndn::EncodingEstimator estimator;
  size_t estimatedSize = wireEncode(estimator);

  ndn::EncodingBuffer buffer(estimatedSize, 0);
  wireEncode(buffer);

  m_wire = buffer.block();

  return m_wire;
}

void
NameLsa::wireDecode(const ndn::Block& wire)
{
  m_wire = wire;

  if (m_wire.type() != nlsr::tlv::NameLsa) {
    NDN_THROW(Error("NameLsa", m_wire.type()));
  }

  m_wire.parse();
  auto val = m_wire.elements_begin();

  for (; val != m_wire.elements_end(); ++val) {
    if (val->type() == nlsr::tlv::ServiceName) {
      m_serviceName = readString(*val);
    }
    else if (val->type() == nlsr::tlv::ProcessingTime) {
      m_processingTime = readNonNegativeDouble(*val);
    }
    else if (val->type() == nlsr::tlv::LoadIndex) {
      m_loadIndex = readNonNegativeDouble(*val);
    }
    else if (val->type() == nlsr::tlv::NameLsa) {
      m_npl.wireDecode(*val);
    }
    else {
      NDN_THROW(Error("Unexpected TLV type", val->type()));
    }
  }
}

void
NameLsa::print(std::ostream& os) const
{
  os << "      Names:\n";
  int i = 0;
  for (const auto& name : m_npl.getPrefixInfo()) {
    os << "        Name " << i << ": " << name.getName()
       << " | Cost: " << name.getCost() << "\n";
    i++;
  }
}

std::tuple<bool, std::list<ndn::Name>, std::list<ndn::Name>>
NameLsa::update(const std::shared_ptr<Lsa>& lsa)
{
  auto nlsa = std::static_pointer_cast<NameLsa>(lsa);
  bool updated = false;

  if (m_serviceName != nlsa->getServiceName() ||
      m_processingTime != nlsa->getProcessingTime() ||
      m_loadIndex != nlsa->getLoadIndex()) {
    m_serviceName = nlsa->getServiceName();
    m_processingTime = nlsa->getProcessingTime();
    m_loadIndex = nlsa->getLoadIndex();
    updated = true;
  }

  auto [nameUpdated, namesToAdd, namesToRemove] = m_npl.update(nlsa->getNpl());
  return std::make_tuple(updated || nameUpdated, namesToAdd, namesToRemove);
}

} // namespace nlsr
