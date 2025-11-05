#ifndef __CUSTOMER_RECORDS_H__
#define __CUSTOMER_RECORDS_H__

#include <map>
#include <mutex>

class CustomerRecords {
public:
  CustomerRecords() = default;
  void Update(int customer_id, int last_order) {
    std::lock_guard<std::mutex> lg(m_);
    records_[customer_id] = last_order;
  }
  bool Read(int customer_id, int* out_last_order) const {
    std::lock_guard<std::mutex> lg(m_);
    auto it = records_.find(customer_id);
    if (it == records_.end()) return false;
    if (out_last_order) *out_last_order = it->second;
    return true;
  }
private:
  mutable std::mutex m_;
  std::map<int,int> records_;
};

#endif
