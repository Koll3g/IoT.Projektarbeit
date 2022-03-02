#pragma once

/**
 * @file fsm.h
 * @brief Simple state machine handler class
 * 
 * @author Ueli Niederer, ueli.niederer@niederer-engineering.ch         
 * @copyright Copyright (c) 2021 by Niederer Engineering GmbH
 */
 
#include <stdint.h>

class FSM;

/**
    \brief  Represents the potential reasons to enter a state handler 
 */
typedef enum eFSM_Reason {
    FSM_REASON_ENTER = 0,   /**< Handler is called to execute "enter" action */
    FSM_REASON_DO,          /**< Handler is called to execute "do" action */
    FSM_REASON_EXIT,        /**< Handler is called to execute "exit" action */

    FSM_REASON_COUNT
}tFSM_Reason;

typedef bool (*tFSM_StateHandler)(FSM *fsm, tFSM_Reason reason, void *context);
#define FSM_STATE_HANDLER(stateName) bool stateName ## Handler(FSM *fsm, tFSM_Reason reason, void *context)

/**
    \brief  Describes a single state for the FSM. For maximum portability, 
            use #FSM_STATE_DESCRIBE to create instances rather than 
            creating the instances yourself.
 */
typedef struct sFSM_State {
    const char *name;               /**< Printable name of the state */
    tFSM_StateHandler handler;      /**< State handler */
}tFSM_State;
#define FSM_STATE_DESCRIBE(stateName, stateHandler)    { stateName, stateHandler ## Handler }
#define FSM_STATE_LAST()                               { 0, 0 }

/**
    \brief  Simple finite state machine library
 */
class FSM
{
    public:
                        FSM(uintptr_t init, const tFSM_State *states, void *context);
        virtual         ~FSM();

        int             run();
        void            reset();

        bool            NextStateSet(uintptr_t nextState);
        uintptr_t       CurrentStateGet();
        const char *    CurrentStateNameGet();

    private:
      void             *m_context;

      const tFSM_State *m_states;
      uintptr_t         m_states_len;

      const tFSM_State *m_state_init; /**< Initial state */
      const tFSM_State *m_state_curr; /**< Current state running */
      const tFSM_State *m_state_next; /**< Next state to enter */
};