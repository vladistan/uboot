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
#include <mockio.h>


extern "C" { 
#include <common.h> 
}


TEST_GROUP(TestLib)
{
};


TEST_GROUP(InitialTest)
{
};

TEST_GROUP(LedSetArgs)
{
    void setup()
    {
        reset_verify();
        MockIO_Create(20);
    }

    void teardown()
    {
        MockIO_Destroy();
        MockIO_Verify_Complete();
    }

};

TEST_GROUP(SetDebugLED)
{
    void setup()
    {
	    reset_verify();
        MockIO_Create(20);
    }

    void teardown()
    {
        MockIO_Destroy();
        MockIO_Verify_Complete();
    }

};


TEST(TestLib, VerifyReset)
{
   reset_verify();
   CHECK(verify_cmd_usage(NULL));

   void * cmd_ptr = (void*)75;

   cmd_usage(cmd_ptr);
   CHECK(verify_cmd_usage(cmd_ptr));
   reset_verify();
   CHECK(verify_cmd_usage(NULL));

}

TEST(LedSetArgs, NotEnoughArgs)
{

   void * cmd_ptr = (void*)12;
   int rv = do_ledset(cmd_ptr,0,0,NULL);

   LONGS_EQUAL(1, rv);

   CHECK(verify_cmd_usage((void*)12));
}

TEST(LedSetArgs, LedArgsParsedCorrectly)
{

   void * cmd_ptr = (void*)12;
   const char * args[] = {"ledset","4","0"};

   MockIO_Expect_GPIONR(1,7);
   MockIO_Expect_gpio_output(0x107, 0);

   int rv = do_ledset(cmd_ptr,0,3,(char**)args);

   LONGS_EQUAL(0, rv);

}


TEST(LedSetArgs, LedArgsParsedCorrectlyWithHexArg)
{

   void * cmd_ptr = (void*)12;
   const char * args[] = {"ledset","0x4","0"};

   MockIO_Expect_GPIONR(1,7);
   MockIO_Expect_gpio_output(0x107, 0);

   int rv = do_ledset(cmd_ptr,0,3,(char**)args);

   LONGS_EQUAL(0, rv);




}
TEST(LedSetArgs, LedArgsParsedCorrectlyWithOctArg)
{

   void * cmd_ptr = (void*)12;
   const char * args[] = {"ledset","04","0"};

   MockIO_Expect_GPIONR(1,7);
   MockIO_Expect_gpio_output(0x107, 0);


   int rv = do_ledset(cmd_ptr,0,3,(char**)args);

   LONGS_EQUAL(0, rv);

}


TEST(LedSetArgs, LedFunctionCorrectlyForOverflow)
{

   void * cmd_ptr = (void*)12;
   const char * args[] = {"ledset","0xAE","5"};

   MockIO_Expect_GPIONR(1,6);
   MockIO_Expect_gpio_output(0x106, 1);

   int rv = do_ledset(cmd_ptr,0,3,(char**)args);

   LONGS_EQUAL(0, rv);

}


TEST(LedSetArgs, LedBank1Pin6SholdBeOn_ForLED11)
{

   void * cmd_ptr = (void*)12;
   const char * args[] = {"ledset","1","1"};

    MockIO_Expect_GPIONR(1,6);
    MockIO_Expect_gpio_output(0x106, 1);


   int rv = do_ledset(cmd_ptr,0,3,(char**)args);

   LONGS_EQUAL(0, rv);

}

TEST(LedSetArgs, LedBank7Pin12SholdBeOff_ForLED20)
{
   void * cmd_ptr = (void*)12;
   const char * args[] = {"ledset","2","0"};

    MockIO_Expect_GPIONR(7,12);
    MockIO_Expect_gpio_output(0x70c, 0);

   int rv = do_ledset(cmd_ptr,0,3,(char**)args);

   LONGS_EQUAL(0, rv);

}


TEST(LedSetArgs, LedBank1Pin8SholdBeOn_ForLED31)
{

   void * cmd_ptr = (void*)12;
   const char * args[] = {"ledset","3","1"};

    MockIO_Expect_GPIONR(1,8);
    MockIO_Expect_gpio_output(0x108, 1);


   int rv = do_ledset(cmd_ptr,0,3,(char**)args);

   LONGS_EQUAL(0, rv);

}

TEST(LedSetArgs, LedBank7Pin13SholdBeOff_ForLED50)
{

    MockIO_Expect_GPIONR(0x7,0xD);
    MockIO_Expect_gpio_output(0x70D, 0);
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

    MockIO_Expect_GPIONR(0x1,0x8);
    MockIO_Expect_gpio_output(0x108, 1);
    MockIO_Expect_GPIONR(0x1,0x7);
    MockIO_Expect_gpio_output(0x107, 1);


   rv = do_ledset(cmd_ptr,0,3,(char**)args1);
   LONGS_EQUAL(0, rv);
   
   rv = do_ledset(cmd_ptr,0,3,(char**)args2);
   LONGS_EQUAL(0, rv);
   

}

TEST(SetDebugLED, LedBankShouldSetupImxGpioNrCorrectly)
{

    MockIO_Expect_GPIONR(0x7,0xD);
    MockIO_Expect_gpio_output(0x70D, 1);


   set_debug_led(0x5,0x1);

}


int main(int ac, char** av)
{
    return CommandLineTestRunner::RunAllTests(ac, av);
}
