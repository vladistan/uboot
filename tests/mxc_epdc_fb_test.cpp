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

extern "C"
{
    #include <mxc_epdc_fb.h>
    void lcd_panel_disable(void);
}


TEST_GROUP(MXC_EPDC_SMOKE)
{
        void setup()
        {
        }

        void teardown()
        {
            mock().checkExpectations();
            mock().removeAllComparators();
            mock().clear();

        }

};


TEST(MXC_EPDC_SMOKE, ExcerciseStubs)
{

     lcd_initcolregs();
     lcd_setcolreg(0,0,0,0);

}


TEST(MXC_EPDC_SMOKE, LcdPanelDisable)
{

    mock().expectOneCall("epdc_power_off")
           ;

     lcd_panel_disable();

}

TEST_GROUP(MXC_EPDC_SIMPLE)
{
        void setup()
        {
        }

        void teardown()
        {
            mock().checkExpectations();
            mock().removeAllComparators();
            mock().clear();
        }
};


TEST(MXC_EPDC_SIMPLE, CalcFBSize100x100x4BPP)
{
	panel_info.vl_row = 100;
    panel_info.vl_col = 100;
    panel_info.vl_bpix = 3;

    ulong rv = calc_fbsize();

    LONGS_EQUAL(20000, rv);
}

TEST(MXC_EPDC_SIMPLE, CalcFBSize100x100x1BPP)
{
    panel_info.vl_row = 100;
    panel_info.vl_col = 100;
    panel_info.vl_bpix = 0;

    ulong rv = calc_fbsize();

    LONGS_EQUAL(2500, rv);
}

TEST(MXC_EPDC_SIMPLE, CalcFBSize4320x1920x4BPP)
{
    panel_info.vl_row = 4320;
    panel_info.vl_col = 1920;
    panel_info.vl_bpix = 3;

    ulong rv = calc_fbsize();

    LONGS_EQUAL(4320*1920*2, rv);
}

