#include "structures/parameterset.h"
#include "structures/callback.h"
#include "structures/callbackset.h"

#include "../../test/tests.h"

class CallbackMock : public Utils::Callback
{
public:
  CallbackMock ()
  {
    set ("invoke_count", 0);
  }
  void operator() ()
  {
    (*this)["invoke_count"]++;
    setReturnValue (TRUE);
  }
};

void callbackset_create_test ()
{
  auto cs = Utils::CallbackSet ();

  assert_are_equal (0, cs.parameterCount ());
}
void callbackset_add_callback_test ()
{
  auto cs = Utils::CallbackSet ();

  assert_are_equal (0, cs.parameterCount ());
  cs.set ("mock1", new CallbackMock ());
  assert_are_equal (1, cs.parameterCount ());
  assert_is_true ((cs && "mock1"));
}
void callbackset_add_and_invoke_callback_test ()
{
  auto cs = Utils::CallbackSet ();

  assert_are_equal (0, cs.parameterCount ());
  cs.set ("mock1", new CallbackMock ());
  assert_are_equal (1, cs.parameterCount ());

  cs ("mock1");

  assert_are_equal (1, cs["mock1"]["invoke_count"]);
  assert_are_equal (TRUE, cs["mock1"].returnValue ());
  assert_are_equal (1, cs["mock1"]["returnValue"]);
}
void callbackset_invoke_noop_test ()
{
  auto cs = Utils::CallbackSet ();

  assert_are_equal (0, cs.parameterCount ());
  assert_is_false ((cs && "mock1"));

  cs ("mock1");
}

void structures_callbackset_tests ()
{
  TEST(callbackset_create_test);
  TEST(callbackset_add_callback_test);
  TEST(callbackset_add_and_invoke_callback_test);
  TEST(callbackset_invoke_noop_test);
}

int main (int argc, char** argv)
{
  TEST_GROUP(structures_callbackset_tests);
}