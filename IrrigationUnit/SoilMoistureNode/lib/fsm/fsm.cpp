#include "fsm.h"

/**
 * @file
 * @brief   Simple state machine handler class
 * 
 *          This class provides a minimum state machine processor, that can run through
 *          an array of states. The states are injected through an array of state 
 *          descriptions. \see tFSM_State
 * 
 * @author    Ueli Niederer, ueli.niederer@niederer-engineering.ch         
 * @copyright Copyright (c) 2021 by Niederer Engineering GmbH
 */

static FSM_STATE_HANDLER(NullState);
static const tFSM_State NullState = FSM_STATE_DESCRIBE("NullState", NullState);

FSM::FSM(uintptr_t init, const tFSM_State *states, void *context) {
    m_states = states;
    while (states->name != 0) {
        m_states_len++;
        states++;
    }

    m_state_init = &m_states[init];
    m_state_curr = &NullState;
    m_state_next = m_state_init;
    m_context = context;
}

FSM::~FSM() {

}

void FSM::reset() {
    m_state_curr->handler(this, FSM_REASON_EXIT, m_context);
    m_state_curr = &NullState;
    m_state_next = m_state_init;
}

int FSM::run() {
    if (m_state_next != m_state_curr) {
        m_state_curr->handler(this, FSM_REASON_EXIT, m_context);
        m_state_next->handler(this, FSM_REASON_ENTER, m_context);
        m_state_curr = m_state_next;
    }
    return m_state_curr->handler(this, FSM_REASON_DO, m_context);
}

bool FSM::NextStateSet(uintptr_t nextState) {
    if (nextState >= m_states_len) {
        return false;
    }

    m_state_next = &m_states[nextState];
    return true;
}

uintptr_t FSM::CurrentStateGet() {
    return m_state_curr - m_states;
}

const char *FSM::CurrentStateNameGet() {
    return m_state_curr->name;
}

static FSM_STATE_HANDLER(NullState) {
    return 0;
}