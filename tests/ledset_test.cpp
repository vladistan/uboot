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
};

TEST_GROUP(SetDebugLED)
{
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
   reset_verify();

   void * cmd_ptr = (void*)12;
   int rv = do_ledset(cmd_ptr,0,0,NULL);

   LONGS_EQUAL(1, rv);

   CHECK(verify_cmd_usage((void*)12));
}

TEST(LedSetArgs, LedArgsParsedCorrectly)
{
   reset_verify();

   void * cmd_ptr = (void*)12;
   const char * args[] = {"ledset","4","0"};

   int rv = do_ledset(cmd_ptr,0,3,(char**)args);

   LONGS_EQUAL(0, rv);
   CHECK(verify_IMX_GPIO_NR(1,7));
   CHECK(verify_gpio_direction_output(0x107,0));

}


TEST(LedSetArgs, LedArgsParsedCorrectlyWithHexArg)
{
   reset_verify();

   void * cmd_ptr = (void*)12;
   const char * args[] = {"ledset","0x4","0"};

   int rv = do_ledset(cmd_ptr,0,3,(char**)args);

   LONGS_EQUAL(0, rv);
   CHECK(verify_IMX_GPIO_NR(1,7));
   CHECK(verify_gpio_direction_output(0x107,0));

}



TEST(LedSetArgs, LedArgsParsedCorrectlyWithOctArg)
{
   reset_verify();

   void * cmd_ptr = (void*)12;
   const char * args[] = {"ledset","04","0"};

   int rv = do_ledset(cmd_ptr,0,3,(char**)args);

   LONGS_EQUAL(0, rv);
   CHECK(verify_IMX_GPIO_NR(1,7));
   CHECK(verify_gpio_direction_output(0x107,0));

}


TEST(LedSetArgs, LedFunctionCorrectlyForOverflow)
{
   reset_verify();

   void * cmd_ptr = (void*)12;
   const char * args[] = {"ledset","0xAE","5"};

   int rv = do_ledset(cmd_ptr,0,3,(char**)args);

   LONGS_EQUAL(0, rv);
   CHECK(verify_IMX_GPIO_NR(1,6));
   CHECK(verify_gpio_direction_output(0x106,1));

}


TEST(LedSetArgs, LedBank1Pin6SholdBeOn_ForLED11)
{
   reset_verify();

   void * cmd_ptr = (void*)12;
   const char * args[] = {"ledset","1","1"};

   int rv = do_ledset(cmd_ptr,0,3,(char**)args);

   LONGS_EQUAL(0, rv);
   CHECK(verify_IMX_GPIO_NR(1,6));
   CHECK(verify_gpio_direction_output(0x106,1));
}

TEST(LedSetArgs, LedBank7Pin12SholdBeOff_ForLED20)
{
   reset_verify();

   void * cmd_ptr = (void*)12;
   const char * args[] = {"ledset","2","0"};

   int rv = do_ledset(cmd_ptr,0,3,(char**)args);

   LONGS_EQUAL(0, rv);
   CHECK(verify_IMX_GPIO_NR(7,12));
   CHECK(verify_gpio_direction_output(0x70c,0));
}


TEST(LedSetArgs, LedBank1Pin8SholdBeOn_ForLED31)
{
   reset_verify();

   void * cmd_ptr = (void*)12;
   const char * args[] = {"ledset","3","1"};

   int rv = do_ledset(cmd_ptr,0,3,(char**)args);

   LONGS_EQUAL(0, rv);
   CHECK(verify_IMX_GPIO_NR(1,8));
   CHECK(verify_gpio_direction_output(0x108,1));
}


TEST(LedSetArgs, LedBank7Pin13SholdBeOff_ForLED50)
{
   reset_verify();

   void * cmd_ptr = (void*)12;
   const char * args[] = {"ledset","5","0"};

   int rv = do_ledset(cmd_ptr,0,3,(char**)args);

   LONGS_EQUAL(0, rv);
   CHECK(verify_IMX_GPIO_NR(7,13));
   CHECK(verify_gpio_direction_output(0x70D,0));
}

TEST(SetDebugLED, LedBankShouldSetupImxGpioNrCorrectly)
{

   reset_verify();
   set_debug_led(0x5,0x1);

   CHECK(verify_IMX_GPIO_NR(7,13));
   CHECK(verify_gpio_direction_output(0x70D,1));

}


int main(int ac, char** av)
{
    return CommandLineTestRunner::RunAllTests(ac, av);
}
