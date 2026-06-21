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
axis_master_driver<BYTES>::axis_master_driver(axis_if<BYTES>& mif, DoneCB done)
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

  if  (!transfer_pending_ && !current_txn_.empty()) { // no ongoing transfer and more transfers to send
    // drive next transfer
    drive(current_txn_.transfers[transfer_idx_]);

    // ongoing transfer flag
    transfer_pending_ = true;
  } else if (!transfer_pending_ && !queue_.empty()) { // no ongoing transfer and there are more transactions
    // grab next transaction
    current_txn_ = queue_.front();
    queue_.pop();

    // reset transfer count
    transfer_idx_ = 0;

    // send first transfer in current transaction
    drive(current_txn_.transfers[transfer_idx_]);

    // update ongoing transfer flag
    transfer_pending_ = true;
  } else if (!transfer_pending_) { // current transfer is done
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
  std::memcpy(mif_.data, t.tdata, BYTES);
  std::memcpy(mif_.keep, t.tkeep, BYTES);
  *mif_.tlast = t.tlast? 1 : 0;
}

template <unsigned BYTES>
void axis_master_driver<BYTES>::deassert() {
  *mif_.tvalid = 0;
  *mif_.tlast  = 0;
  std::memset(mif_.data, 0, BYTES);
  std::memset(mif_.keep, 0, BYTES);
}

template <unsigned BYTES>
void axis_master_driver<BYTES>::advance() {
  ++transfer_idx_;

  if (transfer_idx_ >= current_txn_.transfers.size()) { // AXI-S transaction done
    if (done_) {
      // fire callback
      done_(current_txn_);

      // clear transaction buffer
      current_txn_  = Transaction{};

      // reset transfer count
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
