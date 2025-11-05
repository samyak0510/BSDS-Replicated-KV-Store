#ifndef __STATE_MACHINE_LOG_H__
#define __STATE_MACHINE_LOG_H__

#include <mutex>
#include <vector>
#include "Messages.h"

class StateMachineLog {
public:
  StateMachineLog() = default;

  void WriteAt(size_t index, const MapOp& op) {
    std::lock_guard<std::mutex> lg(m_);
    if (index >= log_.size()) log_.resize(index + 1);
    log_[index] = op;
  }

  size_t Size() const {
    std::lock_guard<std::mutex> lg(m_);
    return log_.size();
  }

  MapOp At(size_t idx) const {
    std::lock_guard<std::mutex> lg(m_);
    return log_.at(idx);
  }

private:
  mutable std::mutex m_;
  std::vector<MapOp> log_;
};

#endif