/***
 * Excerpted from "Test-Driven Development for Embedded C",
 * published by The Pragmatic Bookshelf.
 * Copyrights apply to this code. It may not be used to create training material, 
 * courses, books, articles, and the like. Contact us if you are in doubt.
 * We make no guarantees that this code is fit for any purpose. 
 * Visit http://www.pragmaticprogrammer.com/titles/jgade for more book information.
***/
/*- ------------------------------------------------------------------ -*/
/*-    Copyright (c) James W. Grenning -- All Rights Reserved          -*/
/*-    For use by owners of Test-Driven Development for Embedded C,    -*/
/*-    and attendees of Renaissance Software Consulting, Co. training  -*/
/*-    classes.                                                        -*/
/*-                                                                    -*/
/*-    Available at http://pragprog.com/titles/jgade/                  -*/
/*-        ISBN 1-934356-62-X, ISBN13 978-1-934356-62-3                -*/
/*-                                                                    -*/
/*-    Authorized users may use this source code in your own           -*/
/*-    projects, however the source code may not be used to            -*/
/*-    create training material, courses, books, articles, and         -*/
/*-    the like. We make no guarantees that this source code is        -*/
/*-    fit for any purpose.                                            -*/
/*-                                                                    -*/
/*-    www.renaissancesoftware.net james@renaissancesoftware.net       -*/
/*- ------------------------------------------------------------------ -*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mockio.h"
#include "CppUTest/TestHarness_c.h"


enum
{
    FLASH_READ, FLASH_WRITE, GPIO_NR, GPIO_OUTPUT, I2C_READ, I2C_WRITE,  NoExpectedValue = -1
};

typedef struct Expectation
{
    int kind;
    int bank;
    int led;
    int port;
    int state;
    uint8_t chip;
    unsigned int addr;
    uint8_t rv;
} Expectation;


static Expectation * expectations = 0;
static int setExpectationCount;
static int getExpectationCount;
static int maxExpectationCount;
static int failureAlreadyReported = 0;

static Expectation expected;
static Expectation actual;


static const char * report_incorrect_addr_len =
        "Mock I2C does not support alen of 0x%x\n";
static const char * report_incorrect_buf_len =
        "Mock I2C does not support buffer len of 0x%x\n";

static const char * report_expect_wrong_func_gpio_out =
        "Expected GPPIO_OUT(0x%x)\n"
                "\t        But was ( 0x%x)";
static const char * report_expect_wrong_func_i2c_read =
        "Expected I2C_READ(0x%x)\n"
                "\t        But was ( 0x%x)";
static const char * report_expect_wrong_func_i2c_write =
        "Expected I2C_WRITE(0x%x)\n"
                "\t        But was ( 0x%x)";
static const char * report_expect_wrong_func_gpionr =
        "Expected GPIONR(0x%x)\n"
                "\t        But was ( 0x%x)";
static const char * report_wrong_bank_led =
        "Expected BankLed(0x%x, 0x%x)\n"
                "\t        But was (0x%x, 0x%x)\n";
static const char * report_wrong_portstate =
        "Expected PortState(0x%x, 0x%x)\n"
                "\t        But was (0x%x, 0x%x)\n";
static const char * report_wrong_chipaddr =
        "Expected ChipAddr(0x%x, 0x%x)\n"
                "\t        But was (0x%x, 0x%x)\n";
static const char * report_wrong_chipaddrvalue =
        "Expected ChipAddr(0x%x, 0x%x, 0x%x)\n"
                "\t        But was (0x%x, 0x%x, 0x%x)\n";
static const char * report_too_many_expectations =
    "MockIO_Expect: Too many expectations";
static const char * report_MockIO_not_initialized =
    "MockIO not initialized, call MockIO_Create()";
static const char * report_GPIO_NR_but_out_of_expectations =
        "GPIONR(0x%x)";
static const char * report_GPIO_OUTPUT_but_out_of_expectations =
        "GPIOOUT(0x%x)";
static const char * report_i2c_read_but_out_of_expectations =
        "I2CREAD(0x%x)";
static const char * report_no_more_expectations =
    "Exp %d: No more expectations but was ";
static const char * report_expectation_number =
    "Exp %d: ";


void MockIO_Create(int maxExpectations)
{
    expectations = (Expectation*)calloc(maxExpectations, sizeof(Expectation));
    setExpectationCount = 0;
    getExpectationCount = 0;
    maxExpectationCount = maxExpectations;
    failureAlreadyReported = 0;
}

void MockIO_Destroy(void)
{
    if (expectations)
        free(expectations);
    expectations = 0;
}

static void fail(const char * message)
{
    failureAlreadyReported = 1;
    FAIL_TEXT_C(message);
}

static void failWhenNotInitialized(void)
{
    if (expectations == 0)
        fail(report_MockIO_not_initialized);
}

static void failWhenNoRoomForExpectations(const char * message)
{
    failWhenNotInitialized();
    if (setExpectationCount >= maxExpectationCount)
        fail(message);
}

static void failForIncorrectAddrLen(int condition, const char * expectationFailMessage, int alen)
{
    char message[100];

    if (!condition)
        return;

    int size = sizeof message - 1;

    snprintf(message, size,
            expectationFailMessage, alen );
    fail(message);
}

static void failForIncorrectBufLen(int condition, const char * expectationFailMessage, int blen)
{
    char message[100];

    if (!condition)
        return;

    int size = sizeof message - 1;

    snprintf(message, size,
            expectationFailMessage, blen );
    fail(message);
}




void recordGPIONRExpectation(int bank, int led)
{
    expectations[setExpectationCount].kind = GPIO_NR;
    expectations[setExpectationCount].bank = bank;
    expectations[setExpectationCount].led = led;
    setExpectationCount++;
}

void recordGpioOutputExpectation(int port, int state)
{
    expectations[setExpectationCount].kind = GPIO_OUTPUT;
    expectations[setExpectationCount].port = port;
    expectations[setExpectationCount].state = state;
    setExpectationCount++;
}

void recordI2cReadExpectation (uint8_t chip, unsigned int addr, uint8_t rv)
{
    expectations[setExpectationCount].kind = I2C_READ;
    expectations[setExpectationCount].chip = chip;
    expectations[setExpectationCount].addr = addr;
    expectations[setExpectationCount].rv = rv;
    setExpectationCount++;
}

void recordI2cWriteExpectation (uint8_t chip, unsigned int addr, uint8_t rv)
{
    expectations[setExpectationCount].kind = I2C_WRITE;
    expectations[setExpectationCount].chip = chip;
    expectations[setExpectationCount].addr = addr;
    expectations[setExpectationCount].rv = rv;
    setExpectationCount++;
}

void MockIO_Expect_GPIONR(int led, int bank)
{
    failWhenNoRoomForExpectations(report_too_many_expectations);
    recordGPIONRExpectation(led, bank);
}

void MockIO_Expect_gpio_output(int port, int state)
{
    failWhenNoRoomForExpectations(report_too_many_expectations);
    recordGpioOutputExpectation(port, state);
}

static int addrLenIsNotOne(int alen)
{
    return alen != 1;
}

static int bufferLenIsNotOne(int blen)
{
    return blen != 1;
}

void MockIO_Expect_i2c_write(uint8_t chip, unsigned int addr, uint8_t rv)
{
    failWhenNoRoomForExpectations(report_too_many_expectations);
    recordI2cWriteExpectation(chip, addr, rv);
}

void MockIO_Expect_i2c_read(uint8_t chip, unsigned int addr, uint8_t rv)
{
    failWhenNoRoomForExpectations(report_too_many_expectations);

    recordI2cReadExpectation(chip, addr, rv);
}

static void failWhenNoUnusedExpectations(const char * format)
{
    char message[100];
    int size = sizeof message - 1;

    if (getExpectationCount >= setExpectationCount)
    {
        int offset = snprintf(message, size,
                report_no_more_expectations, getExpectationCount + 1);
        snprintf(message + offset, size - offset, format, getExpectationCount);
        fail(message);
    }
}

static void gpionr_setExpectedAndActual(int bank, int led) {

    expected.kind = GPIO_NR;
    expected.bank = expectations[getExpectationCount].bank;
    expected.led  = expectations[getExpectationCount].led;

    actual.kind = GPIO_NR;
    actual.bank = bank;
    actual.led = led;
}

static void gpio_out_setExpectedAndActual(int port, int state) {

    expected.kind = expectations[getExpectationCount].kind;
    expected.port = expectations[getExpectationCount].port;
    expected.state  = expectations[getExpectationCount].state;

    actual.kind = GPIO_OUTPUT;
    actual.port = port;
    actual.state = state;
}

static void i2c_read_setExpectedAndActual(int chip, int addr) {

    expected.kind = expectations[getExpectationCount].kind;
    expected.chip = expectations[getExpectationCount].chip;
    expected.addr  = expectations[getExpectationCount].addr;
    expected.rv  = expectations[getExpectationCount].rv;


    actual.kind = I2C_READ;
    actual.chip = chip;
    actual.addr = addr;
}

static void i2c_write_setExpectedAndActual(int chip, int addr, uint8_t rv) {

    expected.kind = expectations[getExpectationCount].kind;
    expected.chip = expectations[getExpectationCount].chip;
    expected.addr  = expectations[getExpectationCount].addr;
    expected.rv  = expectations[getExpectationCount].rv;


    actual.kind = I2C_WRITE;
    actual.chip = chip;
    actual.addr = addr;
    actual.rv   = rv;
}


static void failWhenWrongBankOrLed(int condition, const char * expectationFailMessage) {

    char message[100];
    int size = sizeof message - 1;

    if (!condition)
        return;

    int offset = snprintf(message, size,
            report_expectation_number, getExpectationCount + 1);
    snprintf(message + offset, size - offset,
            expectationFailMessage, expected.bank, expected.led,
            actual.bank, actual.led );
    fail(message);
}

static void failWhenWrongPortState(int condition, const char * expectationFailMessage) {

    char message[100];
    int size = sizeof message - 1;

    if (!condition)
        return;

    int offset = snprintf(message, size,
            report_expectation_number, getExpectationCount + 1);
    snprintf(message + offset, size - offset,
            expectationFailMessage, expected.port, expected.state,
            actual.port, actual.state );
    fail(message);
}

static void failWhenWrongChipAddr(int condition, const char * expectationFailMessage) {

    char message[100];
    int size = sizeof message - 1;

    if (!condition)
        return;

    int offset = snprintf(message, size,
            report_expectation_number, getExpectationCount + 1);
    snprintf(message + offset, size - offset,
            expectationFailMessage, expected.chip, expected.addr,
            actual.chip, actual.addr );
    fail(message);
}

static void failWhenWrongChipAddrValue(int condition, const char * expectationFailMessage) {

    char message[100];
    int size = sizeof message - 1;

    if (!condition)
        return;

    int offset = snprintf(message, size,
            report_expectation_number, getExpectationCount + 1);
    snprintf(message + offset, size - offset,
            expectationFailMessage, expected.chip, expected.addr, expected.rv,
            actual.chip, actual.addr, actual.rv );
    fail(message);
}




static void failExpectationGPIO(int condition, const char * expectationFailMessage)
{
    char message[100];

    if (!condition)
        return;

    int size = sizeof message - 1;
    int offset = snprintf(message, size,
            report_expectation_number, getExpectationCount + 1);
    snprintf(message + offset, size - offset,
            expectationFailMessage, expected.kind, actual.kind);
    fail(message);
}


static int expectationIsNot(int kind)
{
    return kind != expectations[getExpectationCount].kind;
}


static int expectedBankLedIsNot(int bank, int led)
{
    return bank != expectations[getExpectationCount].bank ||
            led != expectations[getExpectationCount].led;
}

static int expectedPortStateIsNot(int port, int state)
{
    return port != expectations[getExpectationCount].port ||
            state != expectations[getExpectationCount].state;
}

static int expectedChipAddrIsNot(int chip, int addr)
{
    return chip != expectations[getExpectationCount].chip ||
            addr != expectations[getExpectationCount].addr;
}

static int expectedChipAddrValueIsNot(int chip, int addr, uint8_t value)
{
    return chip != expectations[getExpectationCount].chip ||
            addr != expectations[getExpectationCount].addr ||
            value != expectations[getExpectationCount].rv;
}




int IMX_GPIO_NR(int bank, int led)
{
    failWhenNotInitialized();
    gpionr_setExpectedAndActual(bank, led);
    failWhenNoUnusedExpectations(report_GPIO_NR_but_out_of_expectations);
    failExpectationGPIO(expectationIsNot(GPIO_NR), report_expect_wrong_func_gpionr);
    failWhenWrongBankOrLed(expectedBankLedIsNot(bank, led), report_wrong_bank_led);
    getExpectationCount++;

    return (bank << 8) | led;
}




void gpio_direction_output(int port, int state)
{
    failWhenNotInitialized();
    gpio_out_setExpectedAndActual(port, state);

    failWhenNoUnusedExpectations(report_GPIO_OUTPUT_but_out_of_expectations);
    failExpectationGPIO(expectationIsNot(GPIO_OUTPUT), report_expect_wrong_func_gpio_out);
    failWhenWrongPortState(expectedPortStateIsNot(port, state), report_wrong_portstate);

    getExpectationCount++;

}


int i2c_read(uint8_t chip, unsigned int addr, int alen, uint8_t *buffer, int len)
{
    failWhenNotInitialized();
    i2c_read_setExpectedAndActual(chip, addr);

    failWhenNoUnusedExpectations(report_i2c_read_but_out_of_expectations);
    failExpectationGPIO(expectationIsNot(I2C_READ), report_expect_wrong_func_i2c_read);
    failForIncorrectAddrLen(addrLenIsNotOne(alen), report_incorrect_addr_len, alen);
    failForIncorrectBufLen(bufferLenIsNotOne(len), report_incorrect_buf_len, alen);
    failWhenWrongChipAddr(expectedChipAddrIsNot(chip, addr), report_wrong_chipaddr);

    buffer[0] = expected.rv;
    getExpectationCount++;

    return 0;
}


int i2c_write(uint8_t chip, unsigned int addr, int alen, uint8_t *buffer, int len)
{
    failWhenNotInitialized();
    i2c_write_setExpectedAndActual(chip, addr, buffer[0]);

    failWhenNoUnusedExpectations(report_i2c_read_but_out_of_expectations);
    failExpectationGPIO(expectationIsNot(I2C_WRITE), report_expect_wrong_func_i2c_read);
    failForIncorrectAddrLen(addrLenIsNotOne(alen), report_incorrect_addr_len, alen);
    failForIncorrectBufLen(bufferLenIsNotOne(len), report_incorrect_buf_len, alen);
    failWhenWrongChipAddrValue(expectedChipAddrValueIsNot(chip, addr, buffer[0]), report_wrong_chipaddrvalue);

    getExpectationCount++;

    return 0;
}



static void failWhenNotAllExpectationsUsed(void)
{
    char format[] = "Expected %d requests but got %d";
    char message[sizeof format + 5 + 5];

    if (getExpectationCount == setExpectationCount)
        return;

    snprintf(message, sizeof message, format, setExpectationCount,
            getExpectationCount);
    fail(message);
}


void MockIO_Verify_Complete(void)
{
    if (failureAlreadyReported)
        return;
    failWhenNotAllExpectationsUsed();
}