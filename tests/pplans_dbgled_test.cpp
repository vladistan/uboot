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
 
 TEST_GROUP(PPlans_DbgLed)
 {
     void setup()
     {
 	reset_verify();
     }


 };
 
 TEST(PPlans_DbgLed, VerifyReset)
 {
    reset_verify();
    CHECK(verify_cmd_usage(NULL));

 }
 
 