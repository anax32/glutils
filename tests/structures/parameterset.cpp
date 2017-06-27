//#include "patterns/persistable.h"
#include "structures/parameterset.h"

#include "../../test/tests.h"

void parameterset_create_test ()
{
	ParameterSet<int>	ps;

	ps["test"] = 1;

	assert_are_equal (1, ps["test"]);
}
void parameterset_inline_create_test ()
{
	ParameterSet<int> ps 
	{
		{"test1", 1},
		{"test2", 2},
		{"test3", 3}
	};

	assert_are_equal (3, ps.parameterCount ());
	assert_are_equal (1, ps["test1"]);
	assert_are_equal (2, ps["test2"]);
	assert_are_equal (3, ps["test3"]);
}
void parameterset_parameter_count_test ()
{
	ParameterSet<int>	ps {{"test", 1}};
	assert_are_equal (1, ps.parameterCount ());
}
void parameterset_has_test ()
{
	ParameterSet<int>	ps;

	ps["test"] = 1;

	assert_are_equal (true, (ps && "test"));
}
void parameterset_find_value_test ()
{
	ParameterSet<int> ps 
	{
		{"test1", 1},
		{"test2", 2},
		{"test3", 3}
	};

	assert_are_equal (3, ps.parameterCount ());
	assert_are_equal (1, ps["test1"]);
	assert_are_equal (2, ps["test2"]);
	assert_are_equal (3, ps["test3"]);

	auto val = ps.find (2);
	assert_is_true ((*val).first.compare ("test2") == 0);
}
void parameterset_add_value_test ()
{
	ParameterSet<int> ps 
	{
		{"test1", 1},
		{"test2", 2},
		{"test3", 3}
	};

	assert_are_equal (3, ps.parameterCount ());
	ps["test4"] = 4;
	assert_are_equal (4, ps.parameterCount ());
}
void parameterset_change_value_test ()
{
	ParameterSet<int> ps 
	{
		{"test1", 1},
		{"test2", 2},
		{"test3", 3}
	};

	assert_are_equal (2, ps["test2"]);
	ps["test2"] = 4;
	assert_are_equal (4, ps["test2"]);
}
void parameterset_set_value_lcv_test ()
{
	ParameterSet<int> ps 
	{
		{"test1", 1},
		{"test2", 2},
		{"test3", 3}
	};

	assert_are_equal (3, ps.parameterCount ());
	ps.set ("test4", 8);
	assert_are_equal (4, ps.parameterCount ());
	assert_is_true (ps.lastChangedParameter ().compare ("test4") == 0);
}
void parameterset_clear_test ()
{
	ParameterSet<int> ps 
	{
		{"test1", 1},
		{"test2", 2},
		{"test3", 3}
	};

	assert_are_equal (3, ps.parameterCount ());
	ps.clear ();
	assert_are_equal (0, ps.parameterCount ());
}
void parameterset_remove_test ()
{
	ParameterSet<int> ps 
	{
		{"test1", 1},
		{"test2", 2},
		{"test3", 3}
	};

	assert_are_equal (3, ps.parameterCount ());
	ps.remove ("test2");
	assert_are_equal (2, ps.parameterCount ());
	assert_is_true ((ps && "test1"));
	assert_is_true ((ps && "test3"));
	assert_is_false ((ps && "test2"));
}
void parameterset_iterate_test ()
{
	ParameterSet<int> ps
	{
		{"test1", 1},
		{"test2", 2},
		{"test3", 3}
	};

	auto p = ps.begin ();

	assert_is_true (p->first == "test1");
	assert_is_true (p->second == 1);
	p++;
	assert_is_true (p->first == "test2");
	assert_is_true (p->second == 2);
	p++;
	assert_is_true (p->first == "test3");
	assert_is_true (p->second == 3);
	p++;

	assert_is_true (p == ps.end ());
}
void parameterset_iterator_std_funcs_test ()
{
	ParameterSet<int> ps 
	{
		{"test1", 1},
		{"test2", 2},
		{"test3", 3}
	};

	auto p = std::begin (ps);

	assert_is_true (p->first == "test1");
	assert_is_true (p->second == 1);
	std::advance (p, 1);
	assert_is_true (p->first == "test2");
	assert_is_true (p->second == 2);
	std::advance (p, 1);
	assert_is_true (p->first == "test3");
	assert_is_true (p->second == 3);
	std::advance (p, 1);

	assert_is_true (p == std::end (ps));
}
void parameterset_c14_iterate_test ()
{
	ParameterSet<int> ps 
	{
		{"test1", 1},
		{"test2", 2},
		{"test3", 3}
	};

	int i = 0;

	for (auto p : ps)
	{
		switch (i)
		{
			case 0:
			{
				assert_are_equal (p.first, "test1");
				assert_are_equal (p.second, 1);
				break;
			}
			case 1:
			{
				assert_are_equal (p.first, "test2");
				assert_are_equal (p.second, 2);
				break;
			}
			case 2:
			{
				assert_are_equal (p.first, "test3");
				assert_are_equal (p.second, 3);
				break;
			}
		}

		i++;
	}
}
void integerset_inline_create_test ()
{
	IntegerSet ps
	{
		{"test1", 1},
		{"test2", 2},
		{"test3", 3}
	};

	assert_are_equal (3, ps.parameterCount ());
	assert_are_equal (1, ps["test1"]);
	assert_are_equal (2, ps["test2"]);
	assert_are_equal (3, ps["test3"]);
}

void structures_parameterset_tests ()
{
	TEST(parameterset_create_test);
	TEST(parameterset_inline_create_test);
	TEST(parameterset_parameter_count_test);
	TEST(parameterset_has_test);
	TEST(parameterset_find_value_test);
	TEST(parameterset_add_value_test);
	TEST(parameterset_change_value_test);
	TEST(parameterset_set_value_lcv_test);
	TEST(parameterset_clear_test);
	TEST(parameterset_remove_test);

	TEST(parameterset_iterate_test);
	TEST(parameterset_iterator_std_funcs_test);
	TEST(parameterset_c14_iterate_test);

	TEST(integerset_inline_create_test);
}

int main (int argc, char** argv)
{
	TEST_GROUP(structures_parameterset_tests);
}