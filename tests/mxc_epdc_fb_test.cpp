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


TEST_GROUP(MXC_EPDC_LOWLEVEL_IO)
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



void expect_REG_WR(u32 base, u32 off, u32 value )
{
    mock().expectOneCall("REG_WR")
            .withIntParameter("base",   (unsigned int) base)
            .withIntParameter("offset", (unsigned int) off)
            .withIntParameter("value",  (unsigned int) value);
}

void expect_EPDC_REG_WR(u32 off, u32 val )
{
    expect_REG_WR(EPDC_BASE, off, val );
}



extern "C" void epdc_set_screen_res(u32 width, u32 height);
TEST(MXC_EPDC_SIMPLE, CheckSetScreenRes4320x1920)
{

    panel_info.vl_row = 960;
    panel_info.vl_col = 1440;
    panel_info.vl_bpix = 3;

    expect_EPDC_REG_WR(0x40,0x3C005A0);
    epdc_set_screen_res(panel_info.vl_col, panel_info.vl_row);
}

extern "C" void epdc_set_update_coord(u32 x, u32 y);
TEST(MXC_EPDC_SIMPLE, CheckSetUpdateCoord)
{
    expect_EPDC_REG_WR(0x120,0x2340789);
    epdc_set_update_coord(0x789,0x234);
}
