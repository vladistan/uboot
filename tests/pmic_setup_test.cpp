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



TEST_GROUP(PMIC_Setup)
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


TEST(PMIC_Setup, I2CRead_Works)
{

   MockIO_Expect_i2c_read(0x8, 0, 23 );

   uint8_t value = 0;

   int rv = i2c_read( 0x8, 0, 1, &value,  1 );


   LONGS_EQUAL(0, rv);
   LONGS_EQUAL(23, value);

}

TEST(PMIC_Setup, I2CWrite_Works)
{

    MockIO_Expect_i2c_write(0x8, 0, 23 );

    uint8_t value = 23;

    int rv = i2c_write( 0x8, 0, 1, &value,  1 );


    LONGS_EQUAL(0, rv);


}


