/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2024,  The University of Memphis,
 *                           Regents of the University of California
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

#ifndef NLSR_COMMON_HPP
#define NLSR_COMMON_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <vector>
#include <list>
#include <ndn-cxx/name.hpp>
#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/util/time.hpp>
#include <boost/operators.hpp>

// NDNライブラリのインクルードパスを修正
#ifdef HAVE_CONFIG_H
#include "config.hpp"
#endif

namespace nlsr {

using namespace ndn::time_literals;

namespace tlv {
enum {
  Name = 7,
  NameLsa = 128,
  ProcessingTime = 129,
  LoadIndex = 130,
};
} // namespace tlv

using std::size_t;
using std::uint64_t;

inline constexpr ndn::time::seconds GRACE_PERIOD = 10_s;
inline constexpr ndn::time::seconds TIME_ALLOWED_FOR_CANONIZATION = 4_s;

} // namespace nlsr

#endif // NLSR_COMMON_HPP
