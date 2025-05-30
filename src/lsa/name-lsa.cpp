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
#include <ndn-cxx/encoding/block-helpers.hpp>
#include <ndn-cxx/encoding/encoding-buffer.hpp>
#include <ndn-cxx/encoding/tlv.hpp>
#include <algorithm>

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
NameLsa::wireEncode(ndn::encoding::EncodingImpl<TAG>& encoder) const
{
  size_t totalLength = 0;

  // エンコードプロセス
  for (const auto& name : std::begin(m_npl), std::end(m_npl)) {
    totalLength += name.wireEncode(encoder);
  }

  // 処理時間のエンコード
  size_t processingTimeLength = encoder.prependBytes(
    reinterpret_cast<const uint8_t*>(&m_processingTime),
    sizeof(m_processingTime));
  totalLength += processingTimeLength;
  totalLength += encoder.prependVarNumber(processingTimeLength);
  totalLength += encoder.prependVarNumber(tlv::ProcessingTime);

  // 負荷指数のエンコード
  size_t loadIndexLength = encoder.prependBytes(
    reinterpret_cast<const uint8_t*>(&m_loadIndex),
    sizeof(m_loadIndex));
  totalLength += loadIndexLength;
  totalLength += encoder.prependVarNumber(loadIndexLength);
  totalLength += encoder.prependVarNumber(tlv::LoadIndex);

  totalLength += encoder.prependVarNumber(totalLength);
  totalLength += encoder.prependVarNumber(tlv::NameLsa);

  return totalLength;
}

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

  auto val = m_wire.elements_begin();

  if (val != m_wire.elements_end() && val->type() == tlv::ProcessingTime) {
    m_processingTime = decodeDouble(*val);
    ++val;
  }

  if (val != m_wire.elements_end() && val->type() == tlv::LoadIndex) {
    m_loadIndex = decodeDouble(*val);
    ++val;
  }

  for (; val != m_wire.elements_end(); ++val) {
    if (val->type() == tlv::Name) {
      m_npl.insert(ndn::Name(*val));
    }
  }
}

bool
NameLsa::isEqualContent(const NameLsa& other) const
{
  return m_npl == other.m_npl &&
         m_processingTime == other.m_processingTime &&
         m_loadIndex == other.m_loadIndex;
}

std::tuple<bool, std::list<PrefixInfo>, std::list<PrefixInfo>>
NameLsa::update(const std::shared_ptr<Lsa>& lsa)
{
  std::list<PrefixInfo> added;
  std::list<PrefixInfo> removed;

  if (!lsa) {
    return std::make_tuple(false, added, removed);
  }

  auto nameLsa = std::dynamic_pointer_cast<NameLsa>(lsa);
  if (!nameLsa) {
    return std::make_tuple(false, added, removed);
  }

  bool isUpdated = false;

  // 新しい名前の追加
  for (const auto& name : nameLsa->getNpl()) {
    if (m_npl.find(name) == m_npl.end()) {
      m_npl.insert(name);
      added.push_back(PrefixInfo(name));
      isUpdated = true;
    }
  }

  // 古い名前の削除
  for (const auto& name : m_npl) {
    if (nameLsa->getNpl().find(name) == nameLsa->getNpl().end()) {
      removed.push_back(PrefixInfo(name));
      m_npl.remove(name);
      isUpdated = true;
    }
  }

  // 処理時間と負荷指数の更新
  if (m_processingTime != nameLsa->getProcessingTime()) {
    m_processingTime = nameLsa->getProcessingTime();
    isUpdated = true;
  }

  if (m_loadIndex != nameLsa->getLoadIndex()) {
    m_loadIndex = nameLsa->getLoadIndex();
    isUpdated = true;
  }

  return std::make_tuple(isUpdated, added, removed);
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

} // namespace nlsr

// テンプレートのインスタンス化
namespace nlsr {
template size_t
NameLsa::wireEncode(ndn::encoding::EncodingImpl<ndn::encoding::EncoderTag>&) const;

template size_t
NameLsa::wireEncode(ndn::encoding::EncodingImpl<ndn::encoding::EstimatorTag>&) const;
} // namespace nlsr
