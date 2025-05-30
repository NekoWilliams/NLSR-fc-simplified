/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  The University of Memphis,
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
 * NLSR, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>
 *
 **/

#ifndef NLSR_TEST_ACCESS_CONTROL_HPP
#define NLSR_TEST_ACCESS_CONTROL_HPP

#ifdef HAVE_CONFIG_H
#include "config.hpp"
#endif

#include <memory>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/signing-info.hpp>
#include <ndn-cxx/security/v2/certificate.hpp>
#include <ndn-cxx/security/v2/key-chain.hpp>
#include <ndn-cxx/security/v2/validation-policy.hpp>
#include <ndn-cxx/security/v2/validation-policy-accept-all.hpp>
#include <ndn-cxx/security/v2/validation-policy-command-interest.hpp>
#include <ndn-cxx/security/v2/validation-policy-config.hpp>
#include <ndn-cxx/security/v2/validator.hpp>
#include <ndn-cxx/security/v2/certificate-fetcher-direct.hpp>

namespace nlsr {

class TestAccessControl
{
public:
  TestAccessControl()
    : m_keyChain(ndn::KeyChain())
    , m_validator(std::make_shared<ndn::security::v2::ValidationPolicyAcceptAll>(),
                 std::make_shared<ndn::security::v2::CertificateFetcherDirect>(m_face))
  {
  }

  virtual
  ~TestAccessControl() = default;

  ndn::KeyChain&
  getKeyChain()
  {
    return m_keyChain;
  }

  ndn::security::v2::Validator&
  getValidator()
  {
    return m_validator;
  }

private:
  ndn::Face m_face;
  ndn::KeyChain m_keyChain;
  ndn::security::v2::Validator m_validator;
};

} // namespace nlsr

#endif // NLSR_TEST_ACCESS_CONTROL_HPP
