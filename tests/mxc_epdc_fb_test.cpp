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

void expect_REG_SET(u32 base, u32 off, u32 mask )
{
    mock().expectOneCall("REG_SET")
            .withIntParameter("base",   (unsigned int) base)
            .withIntParameter("offset", (unsigned int) off)
            .withIntParameter("mask",  (unsigned int) mask);
}

void expect_REG_RD(u32 base, u32 off, u32 rv ) {

    mock().expectOneCall("REG_RD")
            .withIntParameter("base", (unsigned int) base)
            .withIntParameter("offset", (unsigned int) off)
            .andReturnValue((unsigned  int)rv);
}

void expect_EPDC_REG_RD(u32 off, u32 rv )
{
    expect_REG_RD(0x8, off, rv);
}

void expect_EPDC_REG_WR(u32 off, u32 val )
{
    expect_REG_WR(EPDC_BASE, off, val );
}

void expect_EPDC_REG_SET(u32 off, u32 mask )
{
    expect_REG_SET(EPDC_BASE, off, mask );
}




extern "C" void epdc_set_screen_res(u32 width, u32 height);
TEST(MXC_EPDC_LOWLEVEL_IO, CheckSetScreenRes4320x1920)
{

    panel_info.vl_row = 960;
    panel_info.vl_col = 1440;
    panel_info.vl_bpix = 3;

    expect_EPDC_REG_WR(0x40,0x3C005A0);
    epdc_set_screen_res(panel_info.vl_col, panel_info.vl_row);
}

extern "C" void epdc_set_update_coord(u32 x, u32 y);
TEST(MXC_EPDC_LOWLEVEL_IO, CheckSetUpdateCoord)
{
    expect_EPDC_REG_WR(0x120,0x2340789);
    epdc_set_update_coord(0x789,0x234);
}

extern "C" void epdc_set_update_dimensions(u32 width, u32 height);
TEST(MXC_EPDC_LOWLEVEL_IO, CheckSetUpdateDimenshions)
{
    expect_EPDC_REG_WR(0x140,0x4560123);
    epdc_set_update_dimensions(0x123,0x456);
}


extern "C" void lcd_disable();
TEST(MXC_EPDC_LOWLEVEL_IO, CheckLcdDisable)
{
    expect_EPDC_REG_SET(0x0,0x40000000);
    lcd_disable();
}


extern "C" int epdc_is_lut_active(u32 lut_num);
TEST(MXC_EPDC_LOWLEVEL_IO, CheckIsLutActiveWhenAllZero)
{
    expect_EPDC_REG_RD(0x440,0);

    int rv = epdc_is_lut_active(5);

    LONGS_EQUAL(0, rv);
}


TEST(MXC_EPDC_LOWLEVEL_IO, CheckIsLutActiveWhenThirdIsSet)
{
    expect_EPDC_REG_RD(0x440,0x8);

    int rv = epdc_is_lut_active(3);

    LONGS_EQUAL(1, rv);
}


extern "C" void epdc_submit_update(u32 lut_num, u32 waveform_mode, u32 update_mode, int use_test_mode, u32 np_val);
#define UPDATE_MODE_PARTIAL	0x0
#define UPDATE_MODE_FULL	0x1

TEST(MXC_EPDC_LOWLEVEL_IO, CheckSubmitUpdateCaseFullUpdDefaultLW)
{

    expect_EPDC_REG_WR(0x180,0x0);
    expect_EPDC_REG_WR(0x160,0x1);

    epdc_submit_update(0, 0, UPDATE_MODE_FULL, 0, 0);

}


TEST(MXC_EPDC_LOWLEVEL_IO, CheckSubmitUpdateCaseFullUpdDefaultLWTestMode)
{

    expect_EPDC_REG_WR(0x180,0x80000000);
    expect_EPDC_REG_WR(0x160,0x1 | 0x80000000);

    epdc_submit_update(0, 0, UPDATE_MODE_FULL, 1, 0);

}


extern "C" void epdc_set_horizontal_timing(u32 horiz_start, u32 horiz_end, u32 hsync_width, u32 hsync_line_length);
TEST(MXC_EPDC_LOWLEVEL_IO, CheckSetHorizontalTiming)
{

    expect_EPDC_REG_WR(0x260,  0x40004 );
    expect_EPDC_REG_WR(0x280, 0x640008 );

    epdc_set_horizontal_timing (8, 100,  4, 4);

}


extern "C"   void epdc_set_vertical_timing(u32 vert_start, u32 vert_end, u32 vsync_width);
TEST(MXC_EPDC_LOWLEVEL_IO, CheckSetVerticalTiming)
{

    expect_EPDC_REG_WR(0x2A0, 0x080401 );

    epdc_set_vertical_timing( 4, 8, 1 );

}


TEST_GROUP(MXC_EPDC_COMPLEX_IO)
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

extern "C"  void epdc_init_settings(void);
TEST(MXC_EPDC_COMPLEX_IO, CheckEPDCInitSettings)
{

    panel_info.vl_row =  960;
    panel_info.vl_col = 1440;
    panel_info.vl_left_margin = 176;
    panel_info.vl_right_margin = 64;
    panel_info.vl_hsync = 60;



    panel_info.vl_upper_margin = 1;
    panel_info.vl_lower_margin = 10;
    panel_info.vl_vsync = 1;

    panel_info.epdc_data.epdc_timings.vscan_holdoff = 0x4;

    panel_info.epdc_data.epdc_timings.sdoed_width = 10;
    panel_info.epdc_data.epdc_timings.sdoed_delay = 20;
    panel_info.epdc_data.epdc_timings.sdoez_width = 10;
    panel_info.epdc_data.epdc_timings.sdoez_delay = 20;
    panel_info.epdc_data.epdc_timings.gdclk_offs = 40;
    panel_info.epdc_data.epdc_timings.gdoe_offs = 94;
    panel_info.epdc_data.epdc_timings.gdsp_offs = 0;
    panel_info.epdc_data.epdc_timings.gdclk_hp_offs = 740;
    panel_info.epdc_data.epdc_timings.num_ce = 1;



    expect_EPDC_REG_RD(0x0, 0x12345);
    expect_EPDC_REG_SET(0x0, 0x12305 );         // Set Swizzle Mask
    expect_EPDC_REG_WR(0x50, 0x200 );           // EPDC_FORMAT
    expect_EPDC_REG_WR(0xA0, 0x64C864 );        // FIFO ( DIsabled, 100 200 100 )
//    expect_EPDC_REG_WR(0x1A0, 8 );              // ???? Default temperature
    expect_EPDC_REG_WR(0x40, 0x3c005a0 );       // Resolution  960 x 1440  (1280+160)
    expect_EPDC_REG_WR(0x200, 0x40002 );        // TCE Ctrl
    expect_EPDC_REG_WR(0x260, 0x3C003C );       // HSCAN Timing
    expect_EPDC_REG_WR(0x280, 0x4000B0   );       // HSCAN2 Timing
    expect_EPDC_REG_WR(0x2A0, 0xA0101 );       // VSCAN Timing
    expect_EPDC_REG_WR(0x2C0, 0xA140a14 );      // TCE Timing
    expect_EPDC_REG_WR(0x300, 0 );              // TCE Timing1
    expect_EPDC_REG_WR(0x310, 0x2e40000 );      // TCE Timing2
    expect_EPDC_REG_WR(0x320, 0x5e0028 );              // TCE Timing3
    expect_EPDC_REG_WR(0x220, 0x1105a0 );       // TCE SDCFG
    expect_EPDC_REG_WR(0x240, 0x12 );           // TCE GDCFG
    expect_EPDC_REG_WR(0x2E0, 0x16 );           // TCE Polarity
    expect_EPDC_REG_WR(0x400, 0 );           // IRQ
    expect_EPDC_REG_WR(0x700, 0 );           // EPDC_GPIO


    epdc_init_settings();

}


/*
extern "C"  void draw_mode0(void);
TEST(MXC_EPDC_COMPLEX_IO, draw_mode0)
{

    panel_info.vl_row =  960;
    panel_info.vl_col = 1440;

    panel_info.epdc_data.wv_modes.mode_init = 0;

    expect_EPDC_REG_WR(0x120,0x0);    // Update Coord
    expect_EPDC_REG_WR(0x140,0x3C005A0); // Update Dimensions
    expect_EPDC_REG_WR(0x180,0x0);   // Submit update 1
    expect_EPDC_REG_WR(0x160,0x1);   // Submit update 2
    expect_EPDC_REG_RD(0x440,7);  // 3 LUT active
    expect_EPDC_REG_RD(0x440,3);  // 2 LUT active
    expect_EPDC_REG_RD(0x440,1);  // 1 LUT active
    expect_EPDC_REG_RD(0x440,0);  // All Lut complete

    draw_mode0();
}
*/


/*
extern "C"  void draw_splash_screen(void);
TEST(MXC_EPDC_COMPLEX_IO, draw_splash_screen)
{

    panel_info.vl_row =  960;
    panel_info.vl_col = 1440;

    panel_info.epdc_data.wv_modes.mode_init = 0;
    panel_info.epdc_data.wv_modes.mode_gc16 = 2;

    expect_EPDC_REG_WR(0x120,0x0);    // Update Coord
    expect_EPDC_REG_WR(0x140,0x3C005A0); // Update Dimensions
    expect_EPDC_REG_WR(0x180,0x0);   // Submit update 1
    expect_EPDC_REG_WR(0x160,0x201);   // Submit update 2
    expect_EPDC_REG_RD(0x440,7);  // 3 LUT active
    expect_EPDC_REG_RD(0x440,3);  // 2 LUT active
    expect_EPDC_REG_RD(0x440,1);  // 1 LUT active
    expect_EPDC_REG_RD(0x440,0);  // All Lut complete

    draw_splash_screen();
}
*/