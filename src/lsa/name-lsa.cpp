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
NameLsa::wireEncode(ndn::EncodingImpl<TAG>& encoder) const
{
  size_t totalLength = 0;

  // Encode backwards
  for (const auto& name : m_npl) {
    totalLength += name.wireEncode(encoder);
  }

  // Encode service information
  if (!m_serviceName.empty()) {
    size_t serviceNameLength = encoder.prependByteArray(
      reinterpret_cast<const uint8_t*>(m_serviceName.data()), m_serviceName.size());
    totalLength += encoder.prependVarNumber(serviceNameLength);
    totalLength += encoder.prependVarNumber(tlv::ServiceName);
  }

  // Encode processing time
  if (m_processingTime > 0) {
    totalLength += prependDoubleBlock(encoder, tlv::ProcessingTime, m_processingTime);
  }

  // Encode load index
  if (m_loadIndex > 0) {
    totalLength += prependDoubleBlock(encoder, tlv::LoadIndex, m_loadIndex);
  }

  totalLength += Lsa::wireEncode(encoder);

  totalLength += encoder.prependVarNumber(totalLength);
  totalLength += encoder.prependVarNumber(tlv::NameLsa);

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
  m_wire.parse();

  Lsa::wireDecode(m_wire);

  m_npl.clear();
  m_serviceName.clear();
  m_processingTime = 0.0;
  m_loadIndex = 0.0;

  for (const auto& element : m_wire.elements()) {
    if (element.type() == tlv::PrefixInfo) {
      m_npl.insert(PrefixInfo(element));
    }
    else if (element.type() == tlv::ServiceName) {
      m_serviceName = std::string(reinterpret_cast<const char*>(element.value()),
                                element.value_size());
    }
    else if (element.type() == tlv::ProcessingTime) {
      m_processingTime = decodeDouble(element);
    }
    else if (element.type() == tlv::LoadIndex) {
      m_loadIndex = decodeDouble(element);
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
