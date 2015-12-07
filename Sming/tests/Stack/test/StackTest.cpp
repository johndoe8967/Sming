#include "TestHarness.h"
#include "Stack.h"
#include "StackMain.h"
//#include <string>
#include "HardwareSerial.h"

/*SimpleString StringFrom(const std::string& value)
{
	return SimpleString(value.c_str());
}*/


class creationStackTest : public Test
{
public:
	 creationStackTest ():Test("StackTest") {
	 }
	 void run (TestResult& result_);
};

creationStackTest creationStackInstance;

void creationStackTest::run (TestResult& result_)
{
	tr.testsStarted();
  Stack s;
  LONGS_EQUAL(1, s.size());
  //std::string b = "asa";
  //CHECK_EQUAL("asa", b);
  //CHECK_EQUAL("asb", b);
}
