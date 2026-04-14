/**
 * @file    Dem_Cfg.c
 * @brief   AUTOSAR CP R23-11 - DEM event configuration table (15 events).
 *
 *  Each entry maps to EventId = index + 1.
 *  Fields: Dtc | Priority | DebounceType | CtrFail | CtrPass |
 *          TimeFail_ms | TimePass_ms | ConfirmThresh | AgingThresh |
 *          EnableCondMask | StorageCondMask
 *
 *  EnableCondMask / StorageCondMask: bit N = condition N must be fulfilled.
 *  DEM_ENABLECOND_ALWAYS  (bit 3) means "no condition required".
 */

#include "Dem_Cfg.h"

/* Shorthand masks */
#define ECOND_VOLT   (1U << DEM_ENABLECOND_VOLTAGE_OK)
#define ECOND_COMM   (1U << DEM_ENABLECOND_COMM_OK)
#define ECOND_TEMP   (1U << DEM_ENABLECOND_TEMP_VALID)
#define ECOND_ANY    (1U << DEM_ENABLECOND_ALWAYS)

#define SCOND_ENG    (1U << DEM_STORAGECOND_ENGINE_RUNNING)
#define SCOND_SPD    (1U << DEM_STORAGECOND_SPEED_VALID)
#define SCOND_INIT   (1U << DEM_STORAGECOND_INIT_DONE)
#define SCOND_ANY    (1U << DEM_STORAGECOND_ALWAYS)

/*
 * Dtc            Pri  DbType                   CtrF CtrP TimF   TimP  Cfm Age ECond         SCond
 */
const Dem_EventCfgType Dem_EventCfg[DEM_MAX_NUMBER_OF_EVENTS] = {
    /* 01 DEM_EVENT_VOLTAGE_LOW      */
    { 0x010101UL, 1U, DEM_DEBOUNCE_COUNTER,          10, -10,   0U,    0U, 2U, 10U, ECOND_VOLT, SCOND_ENG  },
    /* 02 DEM_EVENT_VOLTAGE_HIGH     */
    { 0x010102UL, 1U, DEM_DEBOUNCE_COUNTER,          10, -10,   0U,    0U, 2U, 10U, ECOND_VOLT, SCOND_ENG  },
    /* 03 DEM_EVENT_TEMP_SENSOR_1    */
    { 0x020101UL, 2U, DEM_DEBOUNCE_TIME,              0,   0, 500U,  200U, 1U,  8U, ECOND_TEMP, SCOND_ENG  },
    /* 04 DEM_EVENT_TEMP_SENSOR_2    */
    { 0x020102UL, 2U, DEM_DEBOUNCE_TIME,              0,   0, 500U,  200U, 1U,  8U, ECOND_TEMP, SCOND_ENG  },
    /* 05 DEM_EVENT_SPEED_SENSOR     */
    { 0x030101UL, 2U, DEM_DEBOUNCE_COUNTER,          15, -15,   0U,    0U, 2U, 10U, ECOND_VOLT, SCOND_SPD  },
    /* 06 DEM_EVENT_PRESSURE_SENSOR  */
    { 0x040101UL, 2U, DEM_DEBOUNCE_COUNTER,          10, -10,   0U,    0U, 2U, 10U, ECOND_VOLT, SCOND_ENG  },
    /* 07 DEM_EVENT_COMM_TIMEOUT     */
    { 0x050101UL, 1U, DEM_DEBOUNCE_MONITOR_INTERNAL,  0,   0,   0U,    0U, 1U,  5U, ECOND_COMM, SCOND_INIT },
    /* 08 DEM_EVENT_CHECKSUM_ERROR   */
    { 0x060101UL, 1U, DEM_DEBOUNCE_MONITOR_INTERNAL,  0,   0,   0U,    0U, 1U,  5U, ECOND_ANY,  SCOND_INIT },
    /* 09 DEM_EVENT_ACTUATOR_1       */
    { 0x070101UL, 3U, DEM_DEBOUNCE_COUNTER,          20, -20,   0U,    0U, 3U, 12U, ECOND_VOLT, SCOND_ENG  },
    /* 10 DEM_EVENT_ACTUATOR_2       */
    { 0x070102UL, 3U, DEM_DEBOUNCE_COUNTER,          20, -20,   0U,    0U, 3U, 12U, ECOND_VOLT, SCOND_ENG  },
    /* 11 DEM_EVENT_ACTUATOR_3       */
    { 0x070103UL, 3U, DEM_DEBOUNCE_TIME,              0,   0, 300U,  100U, 2U, 12U, ECOND_VOLT, SCOND_ENG  },
    /* 12 DEM_EVENT_NVM_ERROR        */
    { 0x080101UL, 1U, DEM_DEBOUNCE_MONITOR_INTERNAL,  0,   0,   0U,    0U, 1U,  3U, ECOND_ANY,  SCOND_INIT },
    /* 13 DEM_EVENT_WATCHDOG         */
    { 0x090101UL, 1U, DEM_DEBOUNCE_MONITOR_INTERNAL,  0,   0,   0U,    0U, 1U,  3U, ECOND_ANY,  SCOND_ANY  },
    /* 14 DEM_EVENT_OVERCURRENT      */
    { 0x0A0101UL, 1U, DEM_DEBOUNCE_COUNTER,           5,  -5,   0U,    0U, 1U,  8U, ECOND_VOLT, SCOND_ENG  },
    /* 15 DEM_EVENT_UNDERVOLTAGE_ECU */
    { 0x0B0101UL, 1U, DEM_DEBOUNCE_TIME,              0,   0, 200U,  100U, 1U,  8U, ECOND_VOLT, SCOND_ENG  }
};
