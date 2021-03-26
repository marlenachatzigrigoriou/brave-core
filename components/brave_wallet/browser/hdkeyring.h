class HDKeyring {
 public:
  explicit HDKeyring(const std::string& hd_path);
  virtual HDKeyring();

  // seed is optional, ex. for hardware wallet, we will generate root key
  // directly from the key produced in hardware
  virtual void ConstructRootHDKey(const std::vector<uint8_t>& seed);

  virtual void AddAccounts(size_t number = 1);
  virtual std::vector<std::string> GetAccounts();
  virtual void RemoveAccount(const std::string& address);

  // Bitcoin keyring can override this for different address calculation
  virtual std::string GetAddress(size_t index);

  virtual std::vector<uint8_t> SignTransaction(const std::string& address,
                                               const Transaction& tx);
  virtual std::vector<uint8_t> SignMessage(const std::string& address,
                                           const std::vector<uint8_t>& message);
 protected:
  std::unique_ptr<HDKey> root_;
  std::vector<std::unique_ptr<HDKey>> accounts;

 private:
  HDKey* GetHDKeyFromAddress(const std::string& address);
};
