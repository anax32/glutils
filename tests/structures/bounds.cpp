#include "structures/parameterset.h"
#include "structures/bounds.h"

#include "maketest.h"

void bounds_create_test ()
{
  UI::Bounds    b;

  assert_are_equal (0.0f, b.left ());
  assert_are_equal (0.0f, b.right ());
  assert_are_equal (0.0f, b.top ());
  assert_are_equal (0.0f, b.bottom ());
  assert_are_equal (0.0f, b.width ());
  assert_are_equal (0.0f, b.height ());
}
void bounds_left_test ()
{
  UI::Bounds  b (10, 12, 20, 24);
  assert_are_equal (10, b.left ());
}
void bounds_right_test ()
{
  UI::Bounds  b (10, 12, 20, 24);
  assert_are_equal (20, b.right ());
}
void bounds_top_test ()
{
  UI::Bounds  b (10, 12, 20, 24);
  assert_are_equal (12, b.top ());
}
void bounds_bottom_test ()
{
  UI::Bounds  b (10, 12, 20, 24);
  assert_are_equal (24, b.bottom ());
}
void bounds_width_test ()
{
  UI::Bounds  b (10, 12, 20, 24);
  assert_are_equal (10, b.width ());
}
void bounds_height_test ()
{
  UI::Bounds  b (10, 12, 20, 24);
  assert_are_equal (12, b.height ());
}
void bounds_area_test ()
{
  UI::Bounds  b (10, 10, 20, 20);
  assert_are_equal (100, b.area ());
}
void bounds_aspect_ratio_test ()
{
  UI::Bounds  b (10, 10, 20, 30);
  assert_are_equal (0.5f, b.aspectRatio ());
}
void bounds_center_test ()
{
  UI::Bounds  b (10, 10, 20, 30);
  float    x, y;
  b.center (x, y);

  assert_are_equal (15.0f, x);
  assert_are_equal (20.0f, y);
}
void bounds_orientation_none_test ()
{
  UI::Bounds  b (10, 10, 40, 20);

  assert_are_equal (UI::ZERO, b.orientation ());
}
void bounds_orientation_left90_test ()
{
  UI::Bounds  b (20, 10, 10, 40);

  assert_are_equal (UI::LEFT90, b.orientation ());
}
void bounds_orientation_right90_test ()
{
  UI::Bounds  b (10, 10, 20, 40);

  assert_are_equal (UI::RIGHT90, b.orientation ());
}

void structures_bounds_tests ()
{
  TEST(bounds_create_test);
  TEST(bounds_left_test);
  TEST(bounds_right_test);
  TEST(bounds_top_test);
  TEST(bounds_bottom_test);
  TEST(bounds_width_test);
  TEST(bounds_height_test);
  TEST(bounds_area_test);
  TEST(bounds_aspect_ratio_test);
  TEST(bounds_center_test);
  TEST(bounds_orientation_none_test);
  TEST(bounds_orientation_left90_test);
  TEST(bounds_orientation_right90_test);
}

int main (int argc, char** argv)
{
  TEST_GROUP(structures_bounds_tests);
}