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
#include <pplans_pmic.h>
}



TEST_GROUP(I2C_Mock)
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


TEST(I2C_Mock, I2CRead_Works)
{

   MockIO_Expect_i2c_read(0x8, 0, 23 );

   uint8_t value = 0;

   int rv = i2c_read( 0x8, 0, 1, &value,  1 );


   LONGS_EQUAL(0, rv);
   LONGS_EQUAL(23, value);

}


TEST(I2C_Mock, I2CReadFailure_Works)
{

    MockIO_Expect_i2c_read_failure(0x8, 0 );

    uint8_t value = 0;

    int rv = i2c_read( 0x8, 0, 1, &value,  1 );

    LONGS_EQUAL(-1, rv);
    LONGS_EQUAL(0, value);

}

TEST(I2C_Mock, I2CWrite_Works)
{

    MockIO_Expect_i2c_write(0x8, 0, 23 );

    uint8_t value = 23;

    int rv = i2c_write( 0x8, 0, 1, &value,  1 );


    LONGS_EQUAL(0, rv);

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



TEST(PMIC_Setup, FindPFuzeWhenItIsThere )
{
     MockIO_Expect_i2c_read(0x8, 0, 0x10 );
     MockIO_Expect_i2c_read(0x8, 3, 0x11 );

     int rv = probe_pfuze100();

     LONGS_EQUAL(0, rv);
}

TEST(PMIC_Setup, FindPFuzeWhenItIsThereButWrongPMICVersion )
{
    MockIO_Expect_i2c_read(0x8, 0, 0x15 );

    int rv = probe_pfuze100();

    LONGS_EQUAL(-1, rv);
}

TEST(PMIC_Setup, FindPFuzeWhenItIsThereButWrongRevision)
{
    MockIO_Expect_i2c_read(0x8, 0, 0x10 );
    MockIO_Expect_i2c_read(0x8, 3, 0x19 );

    int rv = probe_pfuze100();

    LONGS_EQUAL(-1, rv);
}


TEST(PMIC_Setup, FindPFuzeFailsIfCantReadRegister0 )
{

     MockIO_Expect_i2c_read_failure(0x8, 0 );

     int rv = probe_pfuze100();

     LONGS_EQUAL(-1, rv);
}

TEST(PMIC_Setup, TestWriteToPMIC )
{

     MockIO_Expect_i2c_write (0x8, 0x6d, 0x1e );

     int rv = pplans_pmic_write (0x6d, 0x1e, "Set VGEN2");

     LONGS_EQUAL(0, rv);
}


TEST(PMIC_Setup, TestWriteToPMICwithFailure )
{

    MockIO_Expect_i2c_write_failure(0x8, 0x6d, 0x1e );

    int rv = pplans_pmic_write (0x6d, 0x1e, "Set VGEN2");

    LONGS_EQUAL(-1, rv);
}


TEST(PMIC_Setup, TestIntialPMICSetup )
{

    MockIO_Expect_i2c_write (0x8, 0x6d, 0x1e );
    MockIO_Expect_i2c_write (0x8, 0x6E, 0x10 );
    MockIO_Expect_i2c_write (0x8, 0x6f, 0x1d );
    MockIO_Expect_i2c_write (0x8, 0x71, 0x1a );

    int rv = pplans_pmic_basic_reg_setup ();

    LONGS_EQUAL(0, rv);
}



