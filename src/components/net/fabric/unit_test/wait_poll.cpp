#include "wait_poll.h"

#include <gtest/gtest.h>
#include <chrono> /* seconds */
#include <cstddef> /* size_t */

namespace
{
  /* A callback which simply rejects (for requeue) any callback it comes across */
  Component::IFabric_communicator::cb_acceptance reject(void *, ::status_t, std::uint64_t, std::size_t, void *)
  {
    return Component::IFabric_communicator::cb_acceptance::DEFER;
  }
}

unsigned wait_poll(
  Component::IFabric_communicator &comm_
  , std::function<void(
    void *context
    , ::status_t
    , std::uint64_t completion_flags
    , std::size_t len
    , void *error_data
  )> cb_
  , test_type test_type_
)
{
  std::size_t ct = 0;
  unsigned poll_count = 0;
  while ( ct == 0 )
  {
    if ( test_type_ == test_type::function )
    {
      comm_.wait_for_next_completion(std::chrono::seconds(6000));
      /* To test deferral of completions (poll_completions_tentative), call it. */
      ct += comm_.poll_completions_tentative(reject);
      /* deferrals should not count as completions */
      EXPECT_EQ(ct,0);
      /* To test deferral of deferred completions, call it again. */
      ct += comm_.poll_completions_tentative(reject);
      /* deferrals should not count as completions */
      EXPECT_EQ(ct,0);
    }

    ++poll_count;
    ct += comm_.poll_completions(cb_);
  }
  /* poll_completions does not always get a completion after wait_for_next_completion returns
   * (does it perhaps return when a message begins to appear in the completion queue?)
   * but it should not take more than two trips through the loop to get the completion.
   *
   * The socketss provider, though takes many more.
   */
  if ( test_type_ == test_type::function )
  {
    EXPECT_LE(poll_count,200);
  }
  EXPECT_EQ(ct,1);
  return poll_count;
}
