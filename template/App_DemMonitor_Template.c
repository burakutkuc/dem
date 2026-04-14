/**
 * @file    App_DemMonitor_Template.c
 * @brief   SW-C upper-layer template — shows how a diagnostic monitor uses the DEM API.
 *
 *  This file is NOT part of the DEM module itself.
 *  It demonstrates correct call sequences from the SW-Component side:
 *
 *    1. System initialisation (Ignition ON)
 *    2. Periodic monitor task (10 ms)  — individual sensor checks
 *    3. Enable / storage condition management
 *    4. System shutdown (Ignition OFF)
 *
 *  Each DEM call is annotated with what happens inside the DEM as a result.
 *  Replace HAL_Read*() calls with your real hardware abstraction.
 */

#include "Dem.h"
#include "Dem_Cbk.h"
#include "Dem_FreezeFrameProvider.h"
#include "Dem_Notify.h"

/* =========================================================================
 * External provider callbacks (example implementations)
 * ========================================================================= */

/* Forward declarations for HAL stubs (defined below) */
static float HAL_ReadVoltage(void);
static float HAL_ReadTemperature1(void);
static float HAL_ReadTemperature2(void);

#if (DEM_CB_ENABLE_READ_DATA_ELEMENT == STD_ON)
Std_ReturnType ReadDataElement(Dem_EventIdType EventId,
                               uint16 DataElementId,
                               uint8 *DestBuffer,
                               uint16 *BufSize)
{
    (void)EventId;
    if ((DestBuffer == NULL_PTR) || (BufSize == NULL_PTR)) {
        return E_NOT_OK;
    }

    switch (DataElementId) {
        case DEM_FF_DEID_BATTERY_MV: {
            uint16 mv = (uint16)(HAL_ReadVoltage() * 1000.0f);
            if (*BufSize < (uint16)sizeof(mv)) { return E_NOT_OK; }
            *((uint16*)DestBuffer) = mv;
            *BufSize = (uint16)sizeof(mv);
            return E_OK;
        }

        case DEM_FF_DEID_TEMP1_CENTIDEGC: {
            sint16 t = (sint16)(HAL_ReadTemperature1() * 100.0f);
            if (*BufSize < (uint16)sizeof(t)) { return E_NOT_OK; }
            *((sint16*)DestBuffer) = t;
            *BufSize = (uint16)sizeof(t);
            return E_OK;
        }

        case DEM_FF_DEID_TEMP2_CENTIDEGC: {
            sint16 t = (sint16)(HAL_ReadTemperature2() * 100.0f);
            if (*BufSize < (uint16)sizeof(t)) { return E_NOT_OK; }
            *((sint16*)DestBuffer) = t;
            *BufSize = (uint16)sizeof(t);
            return E_OK;
        }

        default:
            *BufSize = 0U;
            return E_NOT_OK;
    }
}
#endif

#if (DEM_CB_ENABLE_EVENT_DATA_CHANGED == STD_ON)
void EventDataChanged(Dem_EventIdType EventId)
{
    (void)EventId;
    /* Example: application could mirror FF/EDR to log, or trigger diagnostics upload */
}
#endif

#if (DEM_CB_ENABLE_CLEAR_EVENT_ALLOWED == STD_ON)
Std_ReturnType ClearEventAllowed(Dem_EventIdType EventId)
{
    (void)EventId;
    /* Example policy: always allow */
    return E_OK;
}
#endif

#if (DEM_CB_ENABLE_CLEAR_DTC_NOTIFICATION == STD_ON)
void ClearDtcNotification(uint32 Dtc, Dem_DTCOriginType Origin)
{
    (void)Dtc; (void)Origin;
}
#endif

#if (DEM_CB_ENABLE_INIT_MONITOR_FOR_EVENT == STD_ON)
void InitMonitorForEvent(Dem_EventIdType EventId, Dem_InitMonitorReasonType Reason)
{
    (void)EventId; (void)Reason;
}
#endif

/* =========================================================================
 * Notify drain example (upper layer)
 * ========================================================================= */

void App_Dem_NotifyDrainTask(void)
{
    Dem_NotifyEvent ev;

    while (Dem_Notify_Pop(&ev)) {
        switch (ev.Type) {
            case DEM_NOTIFY_UDS_STATUS_CHANGED:
                /* Example: set application flag / update telemetry */
                /* ev.EventId, ev.OldUdsStatus, ev.NewUdsStatus */
                break;

            case DEM_NOTIFY_DTC_CLEARED:
                /* Example: refresh cached DTC list */
                /* ev.Dtc, ev.Origin */
                break;

            case DEM_NOTIFY_EVENT_DATA_CHANGED:
            default:
                break;
        }
    }
}

/* =========================================================================
 * HAL stubs — replace with real sensor read functions
 * ========================================================================= */
static float HAL_ReadVoltage(void)          { return 12.0f; }
static float HAL_ReadTemperature1(void)     { return 25.0f; }
static float HAL_ReadTemperature2(void)     { return 26.0f; }
static float HAL_ReadSpeed(void)            { return 0.0f;  }
static float HAL_ReadPressure(void)         { return 1.0f;  }
static uint8 HAL_IsCommOk(void)             { return TRUE;  }
static uint8 HAL_IsChecksumOk(void)         { return TRUE;  }
static uint8 HAL_ReadActuator1Current(void) { return 5U;    }
static uint8 HAL_ReadActuator2Current(void) { return 5U;    }
static uint8 HAL_ReadActuator3Pos(void)     { return 128U;  }
static uint8 HAL_IsNvmOk(void)             { return TRUE;  }
static uint8 HAL_IsWatchdogOk(void)        { return TRUE;  }
static float HAL_ReadEcuVoltage(void)       { return 12.0f; }

/* =========================================================================
 * 1. System Initialisation  (call once at Ignition ON)
 * ========================================================================= */

/**
 * @brief  Initialise DEM after NvM read has completed.
 *
 *  Step 1 — Dem_PreInit()
 *    DEM zeros all RAM structures (runtime event state, primary memory).
 *    UDS status bytes are set to default: TNCLC | TNCTOC (0x50).
 *
 *  Step 2 — Dem_Init()
 *    DEM restores persistent data from NvM:
 *      primary memory entries, UDS status bytes, debounce counters.
 *    If NvM CRC is invalid → default data is used.
 *
 *  Step 3 — Dem_RestartOperationCycle()
 *    DEM clears TFTOC bit for every event (bit 1 of UDS status byte).
 *    Sets TNCTOC (bit 6) for all events → "test not complete this cycle".
 *    Aging update: events that passed last cycle get their aging counter
 *    incremented; if aging threshold reached → CDTC cleared + entry removed.
 *
 *  Step 4 — Dem_SetEnableCondition / Dem_SetStorageCondition
 *    DEM stores the condition states in RAM. All subsequent
 *    Dem_SetEventStatus() calls check these before processing.
 */
void App_Dem_SystemInit(void)
{
    /* Step 1: RAM init before NvM is ready */
    Dem_PreInit();

    /* Step 2: Restore NvM data → primary memory populated */
    Dem_Init(NULL_PTR);

    /* Step 3: Start ignition cycle → TFTOC cleared, aging ticked */
    Dem_RestartOperationCycle(DEM_OPCYCLE_IGNITION);

    /* Step 4: Hardware conditions are valid at startup */
    Dem_SetEnableCondition(DEM_ENABLECOND_VOLTAGE_OK,   TRUE);
    Dem_SetEnableCondition(DEM_ENABLECOND_COMM_OK,      TRUE);
    Dem_SetEnableCondition(DEM_ENABLECOND_TEMP_VALID,   TRUE);
    Dem_SetEnableCondition(DEM_ENABLECOND_ALWAYS,       TRUE);

    Dem_SetStorageCondition(DEM_STORAGECOND_ENGINE_RUNNING, TRUE);
    Dem_SetStorageCondition(DEM_STORAGECOND_SPEED_VALID,    FALSE); /* not yet */
    Dem_SetStorageCondition(DEM_STORAGECOND_INIT_DONE,      TRUE);
    Dem_SetStorageCondition(DEM_STORAGECOND_ALWAYS,         TRUE);
}

/* =========================================================================
 * 2. Individual monitor functions
 * ========================================================================= */

/**
 * @brief  Monitor 1: Battery voltage too low (EventId=1, counter-based).
 *
 *  DEM_EVENT_STATUS_PREFAILED path:
 *    DEM: debounce counter += 1
 *    When counter reaches CounterThreshFail (10):
 *      → EventStatus qualified as FAILED
 *      → TF=1, TFTOC=1, TFSLC=1, TNCTOC=0
 *      → ConfirmCounter++ → at threshold: CDTC=1, PDTC=1
 *      → Dem_Memory_StoreEvent():
 *          new entry written to PrimaryMemory[] with OccurrenceCounter=1
 *          FreezeFrame snapshot captured (UDS status, timestamp, occurrence)
 *          ExtendedData written (occurrence, aging, failed cycle counters)
 *
 *  DEM_EVENT_STATUS_PREPASSED path:
 *    DEM: debounce counter -= 1
 *    When counter reaches CounterThreshPass (-10):
 *      → EventStatus qualified as PASSED
 *      → TF=0, TNCTOC cleared
 *      → HealingCounter++ → at AgingThreshold: WIR bit cleared
 */
static void App_Dem_CheckVoltageSensor(void)
{
    float v = HAL_ReadVoltage();

    if (v < 9.0f) {
        Dem_SetEventStatus(DEM_EVENT_VOLTAGE_LOW, DEM_EVENT_STATUS_PREFAILED);
    } else if (v > 9.5f) {
        Dem_SetEventStatus(DEM_EVENT_VOLTAGE_LOW, DEM_EVENT_STATUS_PREPASSED);
    }
    /* hysteresis band [9.0, 9.5]: neither call → debounce counter unchanged */

    if (v > 16.0f) {
        Dem_SetEventStatus(DEM_EVENT_VOLTAGE_HIGH, DEM_EVENT_STATUS_PREFAILED);
    } else if (v < 15.5f) {
        Dem_SetEventStatus(DEM_EVENT_VOLTAGE_HIGH, DEM_EVENT_STATUS_PREPASSED);
    }
}

/**
 * @brief  Monitor 3+4: Temperature sensors (time-based debounce).
 *
 *  PREFAILED path:
 *    DEM: TimeCounter += DEM_MAIN_PERIOD_MS (10 ms per tick)
 *    When TimeCounter >= TimeThreshFailMs (500 ms → 50 ticks):
 *      → Qualified as FAILED → same UDS / memory flow as above
 *
 *  Direction change (sensor recovers):
 *    DEM resets TimeCounter to 0, switches direction to PASS.
 */
static void App_Dem_CheckTemperatureSensors(void)
{
    float t1 = HAL_ReadTemperature1();
    float t2 = HAL_ReadTemperature2();

    Dem_SetEventStatus(DEM_EVENT_TEMP_SENSOR_1,
        (t1 > 120.0f) ? DEM_EVENT_STATUS_PREFAILED : DEM_EVENT_STATUS_PREPASSED);

    Dem_SetEventStatus(DEM_EVENT_TEMP_SENSOR_2,
        (t2 > 120.0f) ? DEM_EVENT_STATUS_PREFAILED : DEM_EVENT_STATUS_PREPASSED);
}

/**
 * @brief  Monitor 5: Speed sensor (counter-based).
 */
static void App_Dem_CheckSpeedSensor(void)
{
    float spd = HAL_ReadSpeed();
    /* Invalid range: negative or above physical maximum */
    Dem_SetEventStatus(DEM_EVENT_SPEED_SENSOR,
        ((spd < 0.0f) || (spd > 300.0f))
        ? DEM_EVENT_STATUS_PREFAILED
        : DEM_EVENT_STATUS_PREPASSED);
}

/**
 * @brief  Monitor 6: Pressure sensor (counter-based).
 */
static void App_Dem_CheckPressureSensor(void)
{
    float p = HAL_ReadPressure();
    Dem_SetEventStatus(DEM_EVENT_PRESSURE_SENSOR,
        ((p < 0.5f) || (p > 5.0f))
        ? DEM_EVENT_STATUS_PREFAILED
        : DEM_EVENT_STATUS_PREPASSED);
}

/**
 * @brief  Monitor 7: CAN/LIN communication timeout (monitor-internal debounce).
 *
 *  Monitor-internal path:
 *    DEM: PREFAILED/FAILED → immediately qualified as FAILED
 *         (no counter/timer; SW-C is responsible for debounce)
 */
static void App_Dem_CheckCommTimeout(void)
{
    if (!HAL_IsCommOk()) {
        Dem_SetEventStatus(DEM_EVENT_COMM_TIMEOUT, DEM_EVENT_STATUS_FAILED);
    } else {
        Dem_SetEventStatus(DEM_EVENT_COMM_TIMEOUT, DEM_EVENT_STATUS_PASSED);
    }
}

/**
 * @brief  Monitor 8: Checksum / data integrity (monitor-internal).
 */
static void App_Dem_CheckChecksum(void)
{
    Dem_SetEventStatus(DEM_EVENT_CHECKSUM_ERROR,
        HAL_IsChecksumOk()
        ? DEM_EVENT_STATUS_PASSED
        : DEM_EVENT_STATUS_FAILED);
}

/**
 * @brief  Monitors 9–11: Actuator diagnostics.
 */
static void App_Dem_CheckActuators(void)
{
    uint8 a1 = HAL_ReadActuator1Current();
    uint8 a2 = HAL_ReadActuator2Current();
    uint8 a3 = HAL_ReadActuator3Pos();

    Dem_SetEventStatus(DEM_EVENT_ACTUATOR_1,
        (a1 > 20U) ? DEM_EVENT_STATUS_PREFAILED : DEM_EVENT_STATUS_PREPASSED);

    Dem_SetEventStatus(DEM_EVENT_ACTUATOR_2,
        (a2 > 20U) ? DEM_EVENT_STATUS_PREFAILED : DEM_EVENT_STATUS_PREPASSED);

    Dem_SetEventStatus(DEM_EVENT_ACTUATOR_3,
        ((a3 < 10U) || (a3 > 240U))
        ? DEM_EVENT_STATUS_PREFAILED
        : DEM_EVENT_STATUS_PREPASSED);
}

/**
 * @brief  Monitor 12–13: NvM and watchdog (monitor-internal).
 */
static void App_Dem_CheckSystemHealth(void)
{
    Dem_SetEventStatus(DEM_EVENT_NVM_ERROR,
        HAL_IsNvmOk() ? DEM_EVENT_STATUS_PASSED : DEM_EVENT_STATUS_FAILED);

    Dem_SetEventStatus(DEM_EVENT_WATCHDOG,
        HAL_IsWatchdogOk() ? DEM_EVENT_STATUS_PASSED : DEM_EVENT_STATUS_FAILED);
}

/**
 * @brief  Monitor 14: Overcurrent (counter-based, fast threshold = 5).
 */
static void App_Dem_CheckOvercurrent(void)
{
    float v = HAL_ReadVoltage();
    Dem_SetEventStatus(DEM_EVENT_OVERCURRENT,
        (v < 6.0f) ? DEM_EVENT_STATUS_PREFAILED : DEM_EVENT_STATUS_PREPASSED);
}

/**
 * @brief  Monitor 15: ECU under-voltage (time-based, 200 ms).
 */
static void App_Dem_CheckEcuUndervoltage(void)
{
    float v = HAL_ReadEcuVoltage();
    Dem_SetEventStatus(DEM_EVENT_UNDERVOLTAGE_ECU,
        (v < 7.0f) ? DEM_EVENT_STATUS_PREFAILED : DEM_EVENT_STATUS_PREPASSED);
}

/* =========================================================================
 * 3. Periodic monitor task — called every 10 ms by the OS
 * ========================================================================= */

/**
 * @brief  10 ms monitor task.
 *
 *  Dem_MainFunction() is called first:
 *    DEM: increments Dem_TimestampCounter.
 *        For each time-based event that has a non-zero TimeCounter:
 *          re-applies the current direction → may qualify FAILED/PASSED.
 *
 *  Then each monitor checks its sensor and calls Dem_SetEventStatus().
 */
void App_Dem_MonitorTask_10ms(void)
{
    /* Tick: advance time-based debounce counters inside DEM */
    Dem_MainFunction();

    /* Run all 15 monitor checks */
    App_Dem_CheckVoltageSensor();
    App_Dem_CheckTemperatureSensors();
    App_Dem_CheckSpeedSensor();
    App_Dem_CheckPressureSensor();
    App_Dem_CheckCommTimeout();
    App_Dem_CheckChecksum();
    App_Dem_CheckActuators();
    App_Dem_CheckSystemHealth();
    App_Dem_CheckOvercurrent();
    App_Dem_CheckEcuUndervoltage();
}

/* =========================================================================
 * 4. Condition management callbacks
 * ========================================================================= */

/**
 * @brief  Called when the engine starts running.
 *
 *  DEM effect:
 *    Dem_StorageConditions[DEM_STORAGECOND_ENGINE_RUNNING] = TRUE
 *    From this point, confirmed events with this storage condition
 *    can be written to PrimaryMemory[].
 */
void App_Dem_EngineStarted(void)
{
    Dem_SetStorageCondition(DEM_STORAGECOND_ENGINE_RUNNING, TRUE);
}

/**
 * @brief  Called when the engine stops.
 *
 *  DEM effect:
 *    Storage condition cleared → new entries blocked.
 *    Existing memory entries are NOT removed.
 */
void App_Dem_EngineOff(void)
{
    Dem_SetStorageCondition(DEM_STORAGECOND_ENGINE_RUNNING, FALSE);
}

/**
 * @brief  Called when vehicle reaches a valid speed.
 */
void App_Dem_SpeedValid(boolean isValid)
{
    Dem_SetStorageCondition(DEM_STORAGECOND_SPEED_VALID, isValid);
}

/**
 * @brief  Called when supply voltage is out of range (e.g. during cranking).
 *
 *  DEM effect:
 *    Enable condition cleared → Dem_SetEventStatus() calls return E_NOT_OK
 *    and debounce is NOT advanced. Prevents false faults during cranking.
 */
void App_Dem_VoltageOutOfRange(boolean outOfRange)
{
    Dem_SetEnableCondition(DEM_ENABLECOND_VOLTAGE_OK, (boolean)(!outOfRange));
}

/* =========================================================================
 * 5. System Shutdown  (call at Ignition OFF)
 * ========================================================================= */

/**
 * @brief  Persist DEM state to NvM and shut down.
 *
 *  Dem_RestartOperationCycle():
 *    DEM clears TFTOC for all events.
 *    Aging counters incremented for events with TF=0.
 *
 *  Dem_Shutdown():
 *    DEM packs all runtime data into Dem_NvData struct.
 *    Calls NvM_WriteBlock() (or stub) to persist.
 *    Sets Dem_State = DEM_UNINITIALIZED.
 */
void App_Dem_SystemShutdown(void)
{
    /* End the ignition cycle: age counters tick, TFTOC cleared */
    Dem_RestartOperationCycle(DEM_OPCYCLE_IGNITION);

    /* Persist: primary memory + UDS status + debounce counters → NvM */
    Dem_Shutdown();
}

/* =========================================================================
 * 6. Optional: query event status from application
 * ========================================================================= */

/**
 * @brief  Example: read UDS status byte and check if DTC is confirmed.
 *
 *  Returns TRUE if the voltage-low DTC is currently confirmed.
 */
boolean App_Dem_IsVoltageLowConfirmed(void)
{
    Dem_UdsStatusByteType status = 0U;
    (void)Dem_GetEventUdsStatus(DEM_EVENT_VOLTAGE_LOW, &status);
    return (boolean)((status & DEM_UDS_STATUS_CDTC) != 0U);
}

/**
 * @brief  Example: read fault detection counter for a sensor.
 *
 *  FDC ranges from -128 (about to pass) to +127 (about to confirm fail).
 */
sint8 App_Dem_GetSpeedSensorFDC(void)
{
    sint8 fdc = 0;
    (void)Dem_GetFaultDetectionCounter(DEM_EVENT_SPEED_SENSOR, &fdc);
    return fdc;
}
