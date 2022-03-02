#include "StopWatch.h"

/**
 * @file
 * @brief       Simple, small stopwatch for timeout handling
 * 
 * @author      Ueli Niederer, ueli.niederer@niederer-engineering.ch         
 * @copyright   Copyright (c) 2021 by Niederer Engineering GmbH
 */

using namespace ZbW;

#include <Arduino.h>

StopWatch::StopWatch(tStopWatchTime timeout) : m_timeout(timeout) {
}

StopWatch::~StopWatch() {
}

/**
 * @brief Starts the timeout specified through @p timeout
 * 
 * @param timeout   [ms] timeout to wait for
 */
void StopWatch::start(tStopWatchTime timeout) {
    m_timeout = timeout;
    if (timeout > 0) {
        m_time_start = millis();
    }
}

/**
 * @brief Restarts the timeout with the timeout from the current time.
 * 
 * @note  Depending on the code between the detection of a timeout and the restart,
 *        periodicy of execution might jitter.
 */
void StopWatch::restart() {
    m_time_start = millis();
}

/**
 * @return [ms] time since #StopWatch::start was called
 */
unsigned long StopWatch::timePassed() {
    unsigned long now = millis();
    return now - m_time_start;
}

/**
 * @return true     if time since start is at least as long as specified during 
 *                  start
 * @return false    if timeout did not yet occur
 */
bool StopWatch::isTimeout() {
    unsigned long timespan = timePassed();
    return timespan >= m_timeout;
}
