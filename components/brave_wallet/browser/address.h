/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef BRAVE_COMPONENTS_BRAVE_WALLET_ADDRESS_H_
#define BRAVE_COMPONENTS_BRAVE_WALLET_ADDRESS_H_

namespace brave_wallet {

class Address {
 public:
  static Address FromPublicKey(const std::vector<uint8_t> public_key);

  std::vector<uint8_t> bytes() const { return bytes_; }

  std::string ToHex() const;
  std::string ToChecksumAddress() const;

 private:
  explicit Address(const std::vector<uint8_t> bytes);
  ~Address();

  std::vector<uint8_t> bytes_;
};

}  // namespace brave_wallet
#endif  // BRAVE_COMPONENTS_BRAVE_WALLET_ADDRESS_H_
