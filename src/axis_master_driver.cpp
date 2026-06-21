/**
 * @file axis_master_driver.cpp
 * @brief Implementation of axis_master_driver.
 */

#include "lib_axis_agent.h"
#include <stdexcept>

//-------------------------------------
// Constructor
//-------------------------------------
template <unsigned BYTES>
axis_master_driver<BYTES>::axis_master_driver(const axis_if<BYTES>& mif, DoneCB done)
  :mif_(mif), done_(done) {

  if (mif_.direction != axis_if<BYTES>::MASTER) {
    throw std::invalid_argument(
      "axis_master_driver: mif must have direction MASTER");
    deassert();
  }
}

//-------------------------------------
// Public
//-------------------------------------
template <unsigned BYTES>
void axis_master_driver<BYTES>::send_txn(const Transaction& txn) {
  if (!txn.empty()) {
    queue_.push(txn);
  }
}

template <unsigned BYTES>
void axis_master_driver<BYTES>::send_bytes(const std::vector<uint8_t>& byte_vec) {
  send_txn(Transaction::from_bytes(byte_vec));
}


template <unsigned BYTES>
void axis_master_driver<BYTES>::eval() {
  if (transfer_pending_) {
    if (*mif_.tready) {
      transfer_pending_ = false;
      advance();
    }
  }

  if  (!transfer_pending_ && !current_txn_.empty()) {
    // drive next transfer
    drive(current_txn_.transfers[transfer_idx_]);
    transfer_pending_ = true;
  } else if (!transfer_pending_ && !queue_.empty()) {
    // start next transaction
    current_txn_ = queue_.front();
    queue_.pop();
    transfer_idx_ = 0;
    drive(current_txn_.transfers[transfer_idx_]);
    transfer_pending_ = true;
  } else if (!transfer_pending_) {
    // nothing to send, deassert tvalid
    deassert();
  }
}

template <unsigned BYTES>
bool axis_master_driver<BYTES>::is_idle() const {
  return queue_.empty() && current_txn_.empty() && !transfer_pending_;
}

template <unsigned BYTES>
std::size_t axis_master_driver<BYTES>::queued_transactions() const {
  return queue_.size();
}

template <unsigned BYTES>
void axis_master_driver<BYTES>::set_done_cb(DoneCB cb) {
  done_ = cb;
}

//-------------------------------------
// Private
//-------------------------------------
template <unsigned BYTES>
void axis_master_driver<BYTES>::drive(const Transfer& t) {
  *mif_.tvalid = 1;
  std::memcpy(mif_.tdata, t.tdata, BYTES);
  std::memcpy(mif_.tkeep, t.tkeep, BYTES);
  *mif_.tlast = t.tlast? 1 : 0;
}

template <unsigned BYTES>
void axis_master_driver<BYTES>::deassert() {
  *mif_.tvalid = 0;
  *mif_.tlast  = 0;
  std::memset(mif_.tdata, 0, BYTES);
  std::memset(mif_.tkeep, 0, BYTES);
}

template <unsigned BYTES>
void axis_master_driver<BYTES>::advance() {
  ++transfer_idx_;

  if (transfer_idx_ >= current_txn_.transfers.size()) {
    if (done_) {
      done_(current_txn_);
      current_txn_  = Transaction{};
      transfer_idx_ = 0;
    }
  }
}

template class axis_master_driver<1>;
template class axis_master_driver<2>;
template class axis_master_driver<4>;
template class axis_master_driver<8>;
template class axis_master_driver<16>;
template class axis_master_driver<32>;
template class axis_master_driver<64>;
