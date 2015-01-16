/**

*/

/*
 *
 * Copyright(c) 2015,  Vlad Korolev,  <vlad[@]dblfuzzr.com>
 *
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


TEST_GROUP(MXC_EPDC_SMOKE)
{
        void setup()
        {
        }

        void teardown()
        {


        }

};


TEST(MXC_EPDC_SMOKE, ExcerciseStubs)
{

     lcd_initcolregs();
     lcd_setcolreg(0,0,0,0);

}