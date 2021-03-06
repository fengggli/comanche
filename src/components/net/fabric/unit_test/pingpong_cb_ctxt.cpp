#include "pingpong_cb_ctxt.h"

#include "pingpong_cnxn_state.h"

buffer_state::buffer_state(Component::IFabric_connection &cnxn_, std::size_t buffer_size_, std::uint64_t remote_key_, std::size_t msg_size_)
  : _rm{cnxn_, buffer_size_, remote_key_}
  , v{{&_rm[0], msg_size_}}
  , d{_rm.desc()}
{}

void cb_ctxt::cb_simple(void *ctxt_, ::status_t stat_, std::uint64_t, std::size_t len_, void *)
{
  auto ctxt = static_cast<cb_ctxt *>(ctxt_);
  /* calls one of send_cb, recv_cb */
  return (ctxt->cb_call)(ctxt, stat_, len_);
}

Component::IFabric_op_completer::complete_definite cb_ctxt::cb{cb_simple};

cb_ctxt::cb_ctxt(
  cnxn_state &state_
  , cb_t cb_call_
  , cb_ctxt *response_
  , Component::IFabric_connection &cnxn_
  , std::size_t buffer_size_
  , std::uint64_t remote_key_
  , std::size_t msg_size_
  , const char *id_
)
  : _state(&state_)
  , cb_call(cb_call_)
  , _buffer(cnxn_, buffer_size_, remote_key_, msg_size_)
  , _response(response_)
  , _id(id_)
{
  if ( _response )
  {
    _state->comm().post_recv(&*_buffer.v.begin(), &*_buffer.v.end(), &*_buffer.d.begin(), this);
  }
}
