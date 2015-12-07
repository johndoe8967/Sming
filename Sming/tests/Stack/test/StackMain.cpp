#include "TestHarness.h"
#include <SmingCore/SmingCore.h>



Timer procTimer;
TestResult tr;


void test() {
	Serial.println("test start 123");
    TestRegistry::runAllTests(tr);
    Serial.println("test end");
    System.restart();
}
void init()
{

	procTimer.initializeMs(10000, test).start(false);


}
