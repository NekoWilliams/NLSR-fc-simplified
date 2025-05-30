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

#ifndef NLSR_LSA_NAME_LSA_HPP
#define NLSR_LSA_NAME_LSA_HPP

#include "lsa.hpp"
#include "../name-prefix-list.hpp"
#include <ndn-cxx/util/time.hpp>
#include <boost/operators.hpp>

namespace nlsr {

/**
 * @brief Represents an LSA of name prefixes announced by the origin router.
 *
 * NameLsa is encoded as:
 * @code{.abnf}
 * NameLsa = NAME-LSA-TYPE TLV-LENGTH
 *             Lsa
 *             1*Name
 * @endcode
 */
class NameLsa : public Lsa, private boost::equality_comparable<NameLsa>
{
public:
  class Error : public Lsa::Error
  {
  public:
    using Lsa::Error::Error;
  };

  NameLsa() = default;

  NameLsa(const ndn::Name& originRouter, uint64_t sequenceNumber,
          const ndn::time::system_clock::time_point& expirationTime,
          const NamePrefixList& npl,
          double processingTime = 0.0,
          double loadIndex = 0.0)
    : Lsa(originRouter, sequenceNumber, expirationTime)
    , m_npl(npl)
    , m_processingTime(processingTime)
    , m_loadIndex(loadIndex)
  {
  }

  const NamePrefixList&
  getNpl() const
  {
    return m_npl;
  }

  double
  getProcessingTime() const
  {
    return m_processingTime;
  }

  double
  getLoadIndex() const
  {
    return m_loadIndex;
  }

  void
  addName(const ndn::Name& name)
  {
    m_npl.insert(name);
  }

  void
  removeName(const ndn::Name& name)
  {
    m_npl.erase(name);
  }

  bool
  isEqualContent(const NameLsa& other) const;

  template<ndn::encoding::Tag TAG>
  size_t
  wireEncode(ndn::encoding::EncodingImpl<TAG>& encoder) const;

  const ndn::Block&
  wireEncode() const;

  void
  wireDecode(const ndn::Block& wire);

  virtual std::tuple<bool, std::list<PrefixInfo>, std::list<PrefixInfo>>
  update(const std::shared_ptr<Lsa>& lsa) override;

private:
  static double
  decodeDouble(const ndn::Block& block)
  {
    return ndn::encoding::readDouble(block);
  }

  NamePrefixList m_npl;
  double m_processingTime;
  double m_loadIndex;

  mutable ndn::Block m_wire;
};

NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(NameLsa);

} // namespace nlsr

#endif // NLSR_LSA_NAME_LSA_HPP
