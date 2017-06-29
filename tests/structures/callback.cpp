#include "structures/parameterset.h"
#include "structures/callback.h"

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

void callback_create_test ()
{
  auto clbk = CallbackMock ();

  assert_are_equal (FALSE, clbk.returnValue ());
}
void callback_internal_parameter_test ()
{
  auto clbk = CallbackMock ();

  assert_are_equal (FALSE, clbk.returnValue ());
  assert_is_true ((clbk && "invoke_count"));
  assert_are_equal (0, clbk["invoke_count"]);
}
void callback_invocation_test ()
{
  auto clbk = CallbackMock ();

  assert_are_equal (FALSE, clbk.returnValue ());
  assert_is_true ((clbk && "invoke_count"));
  assert_are_equal (0, clbk["invoke_count"]);
  clbk ();
  assert_are_equal (TRUE, clbk.returnValue ());
  assert_are_equal (1, clbk["invoke_count"]);
}

void structures_callback_tests ()
{
  TEST(callback_create_test);
  TEST(callback_internal_parameter_test);
  TEST(callback_invocation_test);
}

int main (int argc, char** argv)
{
  TEST_GROUP(structures_callback_tests);
}