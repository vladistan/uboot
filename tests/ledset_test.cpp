/**

*/

/*
 *
 * Copyright(c) 2014,  Vlad Korolev,  <vlad[@]dblfuzzr.com>
 *
 * with contributions from Henry Nestler < Henry at BigFoot.de >
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://www.gnu.org/licenses/lgpl-3.0.txt
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 */


#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/JUnitTestOutput.h>
#include <CppUTestExt/MockSupport.h>
#include <mockio.h>


extern "C" { 
#include <common.h> 
}



class MockVoidPtrComparator : public MockNamedValueComparator
{
public:
    
    virtual bool isEqual(const void* object1, const void* object2)
    {
        return object1 == object2;
    }
    
    virtual SimpleString valueToString(const void* object)
    {
        return StringFrom(object);
    }
};

MockVoidPtrComparator voidPtrComparator;


TEST_GROUP(TestLib)
{
    
    void setup()
    {
        mock().installComparator("void *", voidPtrComparator);
        
    }
    
    void teardown()
    {
        mock().checkExpectations();
        mock().removeAllComparators();
        mock().clear();
    }
};


TEST_GROUP(InitialTest)
{
};

TEST_GROUP(LedSetArgs)
{
    void setup()
    {
        mock().installComparator("void *", voidPtrComparator);
     
    }

    void teardown()
    {  
        mock().checkExpectations();
        mock().removeAllComparators();
        mock().clear();
    }

};

TEST_GROUP(SetDebugLED)
{
    void setup()
    {
	    mock().installComparator("void *", voidPtrComparator);
    }

    void teardown()
    {
       
        mock().checkExpectations();
        mock().removeAllComparators();
        mock().clear();
        
    }

};


TEST(TestLib, VerifyViaMock)
{
    mock().expectOneCall("cmd_usage").withParameterOfType("void *","p",NULL); 
    cmd_usage(NULL);
   

}


TEST(TestLib, VerifyReset)
{
   void * cmd_ptr = (void*)75;
    
   mock().expectNCalls(2,"cmd_usage").withParameterOfType("void *","p",NULL); 
   mock().expectOneCall("cmd_usage").withParameterOfType("void *","p",cmd_ptr); 
   
   
   cmd_usage(NULL);
   cmd_usage(cmd_ptr);
   cmd_usage(NULL);
   

   mock().checkExpectations();
   
}

TEST(LedSetArgs, NotEnoughArgs)
{
   void * cmd_ptr = (void*)12;
   
   mock().expectOneCall("cmd_usage").withParameterOfType("void *","p",(void*)12); 
   
   int rv = do_ledset(cmd_ptr,0,0,NULL);

   LONGS_EQUAL(1, rv);
   
   mock().checkExpectations();
}

void MockIO_ExpectLEDIO(int bank, int led, int val)
{
    
    mock().expectOneCall("IMX_GPIO_NR").withParameter("bank",bank).withParameter("led", led);
    int port = (bank << 8) | led;
    mock().expectOneCall("gpio_direction_output").withParameter("port",port).withParameter("state", val);
 
}


TEST(LedSetArgs, LedArgsParsedCorrectly)
{

   void * cmd_ptr = (void*)12;
   const char * args[] = {"ledset","4","0"};

   mock().expectOneCall("IMX_GPIO_NR").withParameter("bank",1).withParameter("led", 7);
   mock().expectOneCall("gpio_direction_output").withParameter("port",0x107).withParameter("state", 0);
   
  
   int rv = do_ledset(cmd_ptr,0,3,(char**)args);

   LONGS_EQUAL(0, rv);

}


TEST(LedSetArgs, LedArgsParsedCorrectlyWithHexArg)
{

   void * cmd_ptr = (void*)12;
   const char * args[] = {"ledset","0x4","0"};

   mock().expectOneCall("gpio_direction_output").withParameter("port",0x107).withParameter("state", 0);
   mock().ignoreOtherCalls();
     
   int rv = do_ledset(cmd_ptr,0,3,(char**)args);

   LONGS_EQUAL(0, rv);
}


TEST(LedSetArgs, LedArgsParsedCorrectlyWithOctArg)
{

   void * cmd_ptr = (void*)12;
   const char * args[] = {"ledset","04","0"};

   mock().expectOneCall("gpio_direction_output").withParameter("port",0x107).withParameter("state", 0);
   mock().ignoreOtherCalls();
   

   int rv = do_ledset(cmd_ptr,0,3,(char**)args);

   LONGS_EQUAL(0, rv);

}


TEST(LedSetArgs, LedFunctionCorrectlyForOverflow)
{

   void * cmd_ptr = (void*)12;
   const char * args[] = {"ledset","0xAE","5"};

   MockIO_ExpectLEDIO(1, 6 , 1);


   int rv = do_ledset(cmd_ptr,0,3,(char**)args);

   LONGS_EQUAL(0, rv);

}


TEST(LedSetArgs, LedBank1Pin6SholdBeOn_ForLED11)
{

   void * cmd_ptr = (void*)12;
   const char * args[] = {"ledset","1","1"};

   MockIO_ExpectLEDIO(1, 6 , 1);

   int rv = do_ledset(cmd_ptr,0,3,(char**)args);

   LONGS_EQUAL(0, rv);

}

TEST(LedSetArgs, LedBank7Pin12SholdBeOff_ForLED20)
{
   void * cmd_ptr = (void*)12;
   const char * args[] = {"ledset","2","0"};

    mock().expectOneCall("IMX_GPIO_NR").withParameter("bank",7).withParameter("led", 12);
    mock().expectOneCall("gpio_direction_output").withParameter("port",0x70C).withParameter("state", 0);
     
   
   int rv = do_ledset(cmd_ptr,0,3,(char**)args);

   LONGS_EQUAL(0, rv);

}


TEST(LedSetArgs, LedBank1Pin8SholdBeOn_ForLED31)
{

   void * cmd_ptr = (void*)12;
   const char * args[] = {"ledset","3","1"};

    mock().expectOneCall("IMX_GPIO_NR").withParameter("bank",1).withParameter("led", 8);
    mock().expectOneCall("gpio_direction_output").withParameter("port",0x108).withParameter("state", 1);
     
    

   int rv = do_ledset(cmd_ptr,0,3,(char**)args);

   LONGS_EQUAL(0, rv);

}

TEST(LedSetArgs, LedBank7Pin13SholdBeOff_ForLED50)
{

    mock().expectOneCall("IMX_GPIO_NR").withParameter("bank",7).withParameter("led", 13);
    mock().expectOneCall("gpio_direction_output").withParameter("port",0x70D).withParameter("state", 0);
  
    void * cmd_ptr = (void*)12;
    const char * args[] = {"ledset","5","0"};
    int rv = do_ledset(cmd_ptr,0,3,(char**)args);

    LONGS_EQUAL(0, rv);

}

TEST(LedSetArgs, TestMultipleLEDs)
{

   void * cmd_ptr = (void*)12;
   const char * args1[] = {"ledset","3","1"};
   const char * args2[] = {"ledset","4","1"};
   int rv;

   mock().expectOneCall("IMX_GPIO_NR").withParameter("bank",1).withParameter("led", 8);
   mock().expectOneCall("IMX_GPIO_NR").withParameter("bank",1).withParameter("led", 7);

   mock().expectOneCall("gpio_direction_output").withParameter("port",0x108).withParameter("state", 1);
   mock().expectOneCall("gpio_direction_output").withParameter("port",0x107).withParameter("state", 1);
 
 
   rv = do_ledset(cmd_ptr,0,3,(char**)args1);
   LONGS_EQUAL(0, rv);
   
   rv = do_ledset(cmd_ptr,0,3,(char**)args2);
   LONGS_EQUAL(0, rv);
   

}

TEST(SetDebugLED, LedBankShouldSetupImxGpioNrCorrectly)
{

    mock().expectOneCall("IMX_GPIO_NR").withParameter("bank",7).withParameter("led", 13);
    mock().expectOneCall("gpio_direction_output").withParameter("port",0x70D).withParameter("state", 1);
  
  
    set_debug_led(0x5,0x1);

}


TEST(SetDebugLED,  LedBankShouldGoZeroWhenZeroPatternRequested  )
{

    MockIO_ExpectLEDIO(1, 0x6 , 0);
    MockIO_ExpectLEDIO(7, 0xC , 0);
    MockIO_ExpectLEDIO(1, 0x8 , 0);
    MockIO_ExpectLEDIO(1, 0x7 , 0);
    MockIO_ExpectLEDIO(7, 0xD , 0);

    set_debug_led_bank(0x0);

}

TEST(SetDebugLED,  PatternOfOneShouldTurnFirstLedOnly  )
{

    MockIO_ExpectLEDIO(1, 0x6 , 1);
    MockIO_ExpectLEDIO(7, 0xC , 0);
    MockIO_ExpectLEDIO(1, 0x8 , 0);
    MockIO_ExpectLEDIO(1, 0x7 , 0);
    MockIO_ExpectLEDIO(7, 0xD , 0);

    set_debug_led_bank(0x1);

}


TEST(SetDebugLED,  PatternOf15ShouldTurnEveryOtherOne  )
{

    MockIO_ExpectLEDIO(1, 0x6 , 1);
    MockIO_ExpectLEDIO(7, 0xC , 0);
    MockIO_ExpectLEDIO(1, 0x8 , 1);
    MockIO_ExpectLEDIO(1, 0x7 , 0);
    MockIO_ExpectLEDIO(7, 0xD , 1);

    set_debug_led_bank(0x15);

}

TEST(SetDebugLED,  PatternOf0AShouldTurnEveryOtherOne  )
{

    MockIO_ExpectLEDIO(1, 0x6 , 0);
    MockIO_ExpectLEDIO(7, 0xC , 1);
    MockIO_ExpectLEDIO(1, 0x8 , 0);
    MockIO_ExpectLEDIO(1, 0x7 , 1);
    MockIO_ExpectLEDIO(7, 0xD , 0);

    set_debug_led_bank(0xA);

}


TEST(SetDebugLED,  PatternOf1FShouldTurnAllOfThemOn  )
{

    MockIO_ExpectLEDIO(1, 0x6 , 1);
    MockIO_ExpectLEDIO(7, 0xC , 1);
    MockIO_ExpectLEDIO(1, 0x8 , 1);
    MockIO_ExpectLEDIO(1, 0x7 , 1);
    MockIO_ExpectLEDIO(7, 0xD , 1);

    set_debug_led_bank(0x1F);

}



int main(int ac, char** av)
{
    return CommandLineTestRunner::RunAllTests(ac, av);
}
