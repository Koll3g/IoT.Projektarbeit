#pragma once

/**
 * @file
 * @brief       Simple, small stopwatch for timeout handling
 * 
 * @author      Ueli Niederer, ueli.niederer@niederer-engineering.ch         
 * @copyright   Copyright (c) 2021 by Niederer Engineering GmbH
 */
 
namespace ZbW{
    typedef unsigned long tStopWatchTime;

    class StopWatch {
        public: 
            StopWatch(tStopWatchTime timeout);
            virtual ~StopWatch();

            void start(tStopWatchTime timeout = 0);
            void restart();
            
            tStopWatchTime timePassed();
            bool           isTimeout();

        private:
            tStopWatchTime m_time_start;
            tStopWatchTime m_timeout;
    };
};
