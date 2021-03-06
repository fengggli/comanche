#ifndef _TEST_PINGPONG_CNXN_STATE_H_
#define _TEST_PINGPONG_CNXN_STATE_H_

#include "delete_copy.h"
#include "pingpong_cb_ctxt.h" /* cb_ctxt::cb_t */
#include <cstddef> /* size_t */

namespace Component
{
  class IFabric_active_endpoint_comm;
}

struct buffer_state;

class cnxn_state
{
public:
  DELETE_COPY(cnxn_state);
private:
  Component::IFabric_active_endpoint_comm *_comm;
  std::size_t _max_inject_size;
public:
  std::size_t msg_size;
  unsigned iterations_left;
  bool done;
  explicit cnxn_state(
    Component::IFabric_active_endpoint_comm &comm_
    , unsigned iteration_count_
    , std::size_t msg_size_
  );
  cnxn_state(cnxn_state &&) noexcept;
  cnxn_state& operator=(cnxn_state &&) noexcept;
  Component::IFabric_active_endpoint_comm &comm() const noexcept { return *_comm; }
  void send(buffer_state &bt, cb_ctxt *tx_ctxt);
};

#endif
