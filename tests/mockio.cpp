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
#include "MockIO.h"
#include "CppUTest/TestHarness_c.h"


enum
{
    FLASH_READ, FLASH_WRITE, GPIO_NR, GPIO_OUTPUT,  NoExpectedValue = -1
};

typedef struct Expectation
{
    int kind;
    int bank;
    int led;
    int port;
    int state;
} Expectation;

static Expectation * expectations = 0;
static int setExpectationCount;
static int getExpectationCount;
static int maxExpectationCount;
static int failureAlreadyReported = 0;

static Expectation expected;
static Expectation actual;


static const char * report_expect_wrong_func_gpio_out =
        "Expected GPPIO_OUT(0x%x)\n"
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
static const char * report_too_many_expectations =
    "MockIO_Expect: Too many expectations";
static const char * report_MockIO_not_initialized =
    "MockIO not initialized, call MockIO_Create()";
static const char * report_GPIO_NR_but_out_of_expectations =
        "GPIONR(0x%x)";
static const char * report_GPIO_OUTPUT_but_out_of_expectations =
        "GPIOOUT(0x%x)";
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