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
#include <pplans_pmic.h>
}

void MockIO_Expect_i2c_write(uint8_t chip, int addr, uint8_t val)
{  
    mock().expectOneCall("i2c_write")
        .withParameter("chip",chip)
        .withParameter("addr",addr)
        .withParameter("val",val)
        .andReturnValue(0);
}

void MockIO_Expect_i2c_read(uint8_t chip, int addr, uint8_t val, int seq)
{  
    mock().expectOneCall("i2c_read")
        .withParameter("chip",chip)
        .withParameter("addr",addr)
        .andReturnValue(0);
    
    char X[40]; 
    sprintf (X, "I2C:RD:%d:%d:%d", chip, addr, seq);  
    mock().setData( X, val );
    
}

void MockIO_Expect_i2c_read_failure(uint8_t chip, int addr)
{
        mock().expectOneCall("i2c_read")
             .withParameter("chip",chip)
             .withParameter("addr",addr)
             .andReturnValue(-1);
}


void MockIO_Expect_i2c_write_failure(uint8_t chip,  int addr, int val)
{

    mock().expectOneCall("i2c_write")
        .withParameter("chip",chip)
        .withParameter("addr",addr)
        .withParameter("val",val)
        .andReturnValue(-1);
    
}

TEST_GROUP(I2C_Mock)
{
        void setup()
        {
        }

        void teardown()
        {
            
            mock().checkExpectations();
            mock().removeAllComparators();
            mock().clear();
            resetI2CMock();
        }

};


TEST(I2C_Mock, I2CRead_Works)
{

   MockIO_Expect_i2c_read(0x8, 0, 23 , 1);

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
            
        }

        void teardown()
        {
            resetI2CMock();
            
            mock().checkExpectations();
            mock().removeAllComparators();
            mock().clear();
            
        }

};



TEST(PMIC_Setup, FindPFuzeWhenItIsThere )
{
    MockIO_Expect_i2c_read(0x8, 0, 0x10, 1);
    MockIO_Expect_i2c_read(0x8, 3, 0x11, 2);
    
     int rv = probe_pfuze100();

     LONGS_EQUAL(0, rv);
}

TEST(PMIC_Setup, FindPFuzeWhenItIsThereButWrongPMICVersion )
{
    
    MockIO_Expect_i2c_read(0x8, 0, 0x15, 1);
     
    int rv = probe_pfuze100();

    LONGS_EQUAL(-1, rv);
}

TEST(PMIC_Setup, FindPFuzeWhenItIsThereButWrongRevision)
{
     
    MockIO_Expect_i2c_read(0x8, 0, 0x10, 1);
    MockIO_Expect_i2c_read(0x8, 3, 0x19, 2);

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

TEST(PMIC_Setup, TestPMICSW3Setup )
{

    MockIO_Expect_i2c_write (0x8, 0x3c, 0x20 );
    MockIO_Expect_i2c_write (0x8, 0x43, 0x20 );

    int rv = pplans_pmic_sw3_reg_setup ();

    LONGS_EQUAL(0, rv);
}



TEST(PMIC_Setup, TestPMICSW3IndependentOpSetup )
{

    MockIO_Expect_i2c_write (0x8, 0x7F, 0x01 );
    MockIO_Expect_i2c_write (0x8, 0xB2, 0x0D );
    MockIO_Expect_i2c_write (0x8, 0xB6, 0x03 );

    int rv = pplans_pmic_sw3_independent_op_setup ();

    LONGS_EQUAL(0, rv);
}



TEST(PMIC_Setup, TestPMICSW3IndependentOpCheckShouldPass )
{

    MockIO_Expect_i2c_write (0x8, 0x7F, 0x01 );
    MockIO_Expect_i2c_read (0x8, 0xB2, 0x0D, 1 );
    MockIO_Expect_i2c_read (0x8, 0xB6, 0x03, 2 );

    int rv = pplans_pmic_sw3_independent_op_check ();

    LONGS_EQUAL(0, rv);
}


TEST(PMIC_Setup, TestPMICSW3IndependentOpCheckShouldFailWhenB2IsNotSet )
{

    MockIO_Expect_i2c_write (0x8, 0x7F, 0x01 );
    MockIO_Expect_i2c_read (0x8, 0xB2, 0x00, 1 );

    int rv = pplans_pmic_sw3_independent_op_check ();

    LONGS_EQUAL(-1, rv);
}

TEST(PMIC_Setup, TestPMICSW3IndependentOpCheckShouldFailWhenB6IsNotSet )
{

    MockIO_Expect_i2c_write (0x8, 0x7F, 0x01 );
    MockIO_Expect_i2c_read (0x8, 0xB2, 0x0D, 1 );
    MockIO_Expect_i2c_read (0x8, 0xB6, 0x00, 2 );

    int rv = pplans_pmic_sw3_independent_op_check ();

    LONGS_EQUAL(-1, rv);
}




TEST_GROUP(PMIC_SW3Programming)
{
        void setup()
        {
        }

        void teardown()
        {
            mock().checkExpectations();
            mock().removeAllComparators();
            mock().clear();
            resetI2CMock();
        }

};


TEST(PMIC_SW3Programming, TestWhenSW3CIsProgrammedWeShouldNotRestart )
{
    MockIO_Expect_i2c_write (0x8, 0x7F, 0x01 );
    MockIO_Expect_i2c_read (0x8, 0xB2, 0x0D, 1 );
    MockIO_Expect_i2c_read (0x8, 0xB6, 0x03, 2 );
    mock().ignoreOtherCalls();

    pplans_pmic_handle_sw3 ();

}



TEST(PMIC_SW3Programming, TestWhenSW3CIsProgrammedWeShouldRestartAfterProgramming )
{

    MockIO_Expect_i2c_write (0x8, 0x7F, 0x01 );
    MockIO_Expect_i2c_read (0x8, 0xB2, 0x0D, 1 );
    MockIO_Expect_i2c_read (0x8, 0xB6, 0x00, 2 );

    MockIO_ExpectLEDIO(1, 0x6 , 1);
    MockIO_ExpectLEDIO(7, 0xC , 1);
    MockIO_ExpectLEDIO(1, 0x8 , 1);
    MockIO_ExpectLEDIO(1, 0x7 , 1);
    MockIO_ExpectLEDIO(7, 0xD , 1);

    MockIO_Expect_i2c_write (0x8, 0x7F, 0x01 );
    MockIO_Expect_i2c_write (0x8, 0xB2, 0x0D );
    MockIO_Expect_i2c_write (0x8, 0xB6, 0x03 );

    MockIO_ExpectLEDIO(1, 0x6 , 1);
    MockIO_ExpectLEDIO(7, 0xC , 0);
    MockIO_ExpectLEDIO(1, 0x8 , 1);
    MockIO_ExpectLEDIO(1, 0x7 , 0);
    MockIO_ExpectLEDIO(7, 0xD , 1);

    MockIO_ExpectLEDIO(1, 0x6 , 0);
    MockIO_ExpectLEDIO(7, 0xC , 1);
    MockIO_ExpectLEDIO(1, 0x8 , 0);
    MockIO_ExpectLEDIO(1, 0x7 , 1);
    MockIO_ExpectLEDIO(7, 0xD , 0);

    MockIO_Expect_i2c_write (0x8, 0xE4, 0x80 );
    MockIO_Expect_i2c_write (0x8, 0x84, 0xF0 );
    
    mock().ignoreOtherCalls();


    pplans_pmic_handle_sw3 ();

}