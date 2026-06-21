/**
 * @file axis_monitor.cpp
 * @brief Implementation of axis_monitor.
 */

 #include "lib_axis_agent.h"

//-------------------------------------
// Constructor
//-------------------------------------
template <unsigned BYTES>
axis_monitor<BYTES>::axis_monitor(const axis_if<BYTES>& iface, TxnCB on_txn)
  :iface_(iface), on_txn_(on_txn) { }

//-------------------------------------
// Public
//-------------------------------------
template <unsigned BYTES>
void axis_monitor<BYTES>::eval() {
  // only care about valid-ready handshake
  if ( (!*iface_.tready) || (!*iface_.tvalid)) {
    return;
  }

  // Capture the current transfer
  axis_transfer<BYTES> transfer;
  std::memcpy(transfer.tdata, iface_.tdata, BYTES);
  std::memcpy(transfer.tkeep, iface_.tkeep, BYTES);
  transfer.tlast = (*iface_.tlast != 0);


  // Add captured transfer to current transaction's queue
  current_.transfers.push_back(transfer);

  // If it's the last transfer
  if (*iface_.tlast) {
    // fire callback function
    if (on_txn_) {
      on_txn_(current_);
    }

    // add current transaction to queue
    rx_queue_.push(current_);

    // clear current transaction container
    current_ = Transaction{};
  }
}

template <unsigned BYTES>
bool axis_monitor<BYTES>::has_transaction() const {
  return !rx_queue_.empty();
}

template <unsigned BYTES>
typename axis_monitor<BYTES>::Transaction axis_monitor<BYTES>::pop_transaction() {

  auto txn = rx_queue_.front();
  rx_queue_.pop();
  return txn;
}

template <unsigned BYTES>
std::size_t axis_monitor<BYTES>::rx_count() const {
  return rx_queue_.size();
}

template <unsigned BYTES>
void axis_monitor<BYTES>::set_transaction_cb(TxnCB cb) {
  on_txn_ = cb;
}

template class axis_monitor<1>;
template class axis_monitor<2>;
template class axis_monitor<4>;
template class axis_monitor<8>;
template class axis_monitor<16>;
template class axis_monitor<32>;
template class axis_monitor<64>;
