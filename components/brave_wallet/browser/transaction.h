#include "brave/components/brave_wallet/brave_wallet_types.h"

class Transaction {
 public:
  Transaction(const uint256_t& nonce,
              const uint256_t& gas_price,
              const uint256_t& gas_limit,
              const std::string& to,
              const uint256_t& value,
              const std::vector<uint8_t> data);
  ~Transaction();

  uint256_t nonce() const { return nonce_; }
  uint256_t gas_price() const { return gas_price_; }
  uint256_t gas_limit() const { return gas_limit_; }
  std::string to() const { return to_; }
  uint256_t value() const { return value_; }
  std::vector<uint8_t> data() const { return data_; }

  // return
  // keccack(rlp([nonce, gasPrice, gasLimit, to, value, data, chainID, 0, 0])
  std::vector<uint8_t> GetMessageToSign(uint64_t chain_id) const;

  // return rlp([nonce, gasPrice, gasLimit, to, value, data, v, r, s])
  std::string GetSignedTransaction() const;

  // signature and recid will be used to produce v, r, s
  void ProcessSignature(const std::vector<uint8_t> signature, int recid);

 private:
  uint256_t nonce_;
  uint256_t gas_price_;
  uint256_t gas_limit_;
  // TODO(darkdh): add an Address class
  std::string to_;
  uint256_t value_;
  std::vector<uint8_t> data_;

  uint8_t v_;
  std::vector<uint8_t> r_;
  std::vector<uint8_t> s_;
};
