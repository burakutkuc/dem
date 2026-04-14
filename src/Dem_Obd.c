/**
 * @file    Dem_Obd.c
 * @brief   AUTOSAR CP R23-11 - DEM OBD-specific API stubs (§8.3.5 + §8.3.7).
 *
 *  All functions are compiled only when DEM_OBD_SUPPORT == STD_ON.
 *  Each function returns E_OK with zeroed output parameters as a starting
 *  point; real implementations must fill in OBD counters and PID data.
 */

#include "Dem_Internal.h"

#if (DEM_OBD_SUPPORT == STD_ON)

/* =========================================================================
 * §8.3.5  OBD-specific Dcm <=> Dem interfaces
 * ========================================================================= */

Std_ReturnType Dem_DcmGetInfoTypeValue08(uint8 ClientId, uint8 *Iumprdenominator,
                                          uint8 *Iumprdenominator_size)
{
    (void)ClientId;
    if ((Iumprdenominator == NULL_PTR) || (Iumprdenominator_size == NULL_PTR)) {
        return E_NOT_OK;
    }
    *Iumprdenominator      = 0U;
    *Iumprdenominator_size = 0U;
    return E_OK;
}

Std_ReturnType Dem_DcmGetInfoTypeValue0B(uint8 ClientId, uint8 *Iumprdenominator,
                                          uint8 *Iumprdenominator_size)
{
    (void)ClientId;
    if ((Iumprdenominator == NULL_PTR) || (Iumprdenominator_size == NULL_PTR)) {
        return E_NOT_OK;
    }
    *Iumprdenominator      = 0U;
    *Iumprdenominator_size = 0U;
    return E_OK;
}

Std_ReturnType Dem_DcmReadDataOfPID01(uint8 *PID01value)
{
    if (PID01value == NULL_PTR) { return E_NOT_OK; }
    /* PID $01: MIL status + DTC count — stub returns MIL off, 0 DTCs */
    PID01value[0] = 0x00U;
    PID01value[1] = 0x00U;
    PID01value[2] = 0x00U;
    PID01value[3] = 0x00U;
    return E_OK;
}

Std_ReturnType Dem_DcmReadDataOfPID1C(uint8 *PID1Cvalue)
{
    if (PID1Cvalue == NULL_PTR) { return E_NOT_OK; }
    *PID1Cvalue = 0x09U; /* OBD requirements on this vehicle */
    return E_OK;
}

Std_ReturnType Dem_DcmReadDataOfPID21(uint8 *PID21value)
{
    if (PID21value == NULL_PTR) { return E_NOT_OK; }
    PID21value[0] = 0U;
    PID21value[1] = 0U;
    return E_OK;
}

Std_ReturnType Dem_DcmReadDataOfPID30(uint8 *PID30value)
{
    if (PID30value == NULL_PTR) { return E_NOT_OK; }
    *PID30value = 0U;
    return E_OK;
}

Std_ReturnType Dem_DcmReadDataOfPID31(uint8 *PID31value)
{
    if (PID31value == NULL_PTR) { return E_NOT_OK; }
    PID31value[0] = 0U;
    PID31value[1] = 0U;
    return E_OK;
}

Std_ReturnType Dem_DcmReadDataOfPID41(uint8 *PID41value)
{
    if (PID41value == NULL_PTR) { return E_NOT_OK; }
    PID41value[0] = 0U;
    PID41value[1] = 0U;
    PID41value[2] = 0U;
    PID41value[3] = 0U;
    return E_OK;
}

Std_ReturnType Dem_DcmReadDataOfPID4D(uint8 *PID4Dvalue)
{
    if (PID4Dvalue == NULL_PTR) { return E_NOT_OK; }
    PID4Dvalue[0] = 0U;
    PID4Dvalue[1] = 0U;
    return E_OK;
}

Std_ReturnType Dem_DcmReadDataOfPID4E(uint8 *PID4Evalue)
{
    if (PID4Evalue == NULL_PTR) { return E_NOT_OK; }
    PID4Evalue[0] = 0U;
    PID4Evalue[1] = 0U;
    return E_OK;
}

Std_ReturnType Dem_DcmReadDataOfPID91(uint8 *PID91value)
{
    if (PID91value == NULL_PTR) { return E_NOT_OK; }
    *PID91value = 0U;
    return E_OK;
}

Std_ReturnType Dem_DcmReadDataOfOBDFreezeFrame(uint8 PID, uint8 DataElementIndexOfPID,
                                                 uint8 *DestBuffer, uint16 *BufSize)
{
    (void)PID;
    (void)DataElementIndexOfPID;
    if ((DestBuffer == NULL_PTR) || (BufSize == NULL_PTR)) { return E_NOT_OK; }
    *BufSize = 0U;
    return E_OK;
}

Std_ReturnType Dem_DcmGetDTCOfOBDFreezeFrame(uint8 FrameNumber, uint32 *DTC,
                                               Dem_DTCFormatType DTCFormat)
{
    (void)FrameNumber;
    (void)DTCFormat;
    if (DTC == NULL_PTR) { return E_NOT_OK; }
    *DTC = 0U;
    return E_OK;
}

Std_ReturnType Dem_DcmGetAvailableOBDMIDs(uint8 MIDValue, uint32 *MIDMask)
{
    (void)MIDValue;
    if (MIDMask == NULL_PTR) { return E_NOT_OK; }
    *MIDMask = 0U;
    return E_OK;
}

Std_ReturnType Dem_DcmGetNumTIDsOfOBDMID(uint8 MIDValue, uint8 *NumberOfTIDs)
{
    (void)MIDValue;
    if (NumberOfTIDs == NULL_PTR) { return E_NOT_OK; }
    *NumberOfTIDs = 0U;
    return E_OK;
}

Std_ReturnType Dem_DcmGetDTRData(uint8 TIDindex, uint8 *TIDvalue, uint8 *UaSID,
                                   uint16 *Testvalue, uint16 *Lowlimvalue,
                                   uint16 *Upplimvalue)
{
    (void)TIDindex;
    if ((TIDvalue == NULL_PTR) || (UaSID == NULL_PTR) ||
        (Testvalue == NULL_PTR) || (Lowlimvalue == NULL_PTR) ||
        (Upplimvalue == NULL_PTR)) {
        return E_NOT_OK;
    }
    *TIDvalue    = 0U;
    *UaSID       = 0U;
    *Testvalue   = 0U;
    *Lowlimvalue = 0U;
    *Upplimvalue = 0U;
    return E_OK;
}

Std_ReturnType Dem_DcmGetInfoTypeValue79(uint8 ClientId, uint8 *Iumprdenominator,
                                          uint8 *Iumprdenominator_size)
{
    (void)ClientId;
    if ((Iumprdenominator == NULL_PTR) || (Iumprdenominator_size == NULL_PTR)) {
        return E_NOT_OK;
    }
    *Iumprdenominator      = 0U;
    *Iumprdenominator_size = 0U;
    return E_OK;
}

Std_ReturnType Dem_DcmReadDataOfPIDF501(uint8 *DataValueBuffer,
                                         uint8 *DataValueBufferSize)
{
    if ((DataValueBuffer == NULL_PTR) || (DataValueBufferSize == NULL_PTR)) {
        return E_NOT_OK;
    }
    *DataValueBufferSize = 0U;
    return E_OK;
}

/* =========================================================================
 * §8.3.7  OBD-specific SW-C / BSW interfaces
 * ========================================================================= */

Std_ReturnType Dem_SetEventDisabled(Dem_EventIdType EventId)
{
    if ((EventId == DEM_EVENT_INVALID) || (EventId > DEM_MAX_NUMBER_OF_EVENTS)) {
        return E_NOT_OK;
    }
    Dem_EventRuntime[EventId - 1U].Available = FALSE;
    return E_OK;
}

Std_ReturnType Dem_GetDataOfPID21(uint8 *PID21value)
{
    if (PID21value == NULL_PTR) { return E_NOT_OK; }
    PID21value[0] = 0U;
    PID21value[1] = 0U;
    return E_OK;
}

Std_ReturnType Dem_SetDataOfPID21(uint8 PID21value)  { (void)PID21value; return E_OK; }
Std_ReturnType Dem_SetDataOfPID31(uint8 PID31value)  { (void)PID31value; return E_OK; }
Std_ReturnType Dem_SetDataOfPID4D(uint8 PID4Dvalue)  { (void)PID4Dvalue; return E_OK; }
Std_ReturnType Dem_SetDataOfPID4E(uint8 PID4Evalue)  { (void)PID4Evalue; return E_OK; }

Std_ReturnType Dem_GetCycleQualified(Dem_OperationCycleIdType OperationCycleId,
                                      boolean *isQualified)
{
    (void)OperationCycleId;
    if (isQualified == NULL_PTR) { return E_NOT_OK; }
    *isQualified = FALSE;
    return E_OK;
}

Std_ReturnType Dem_SetCycleQualified(Dem_OperationCycleIdType OperationCycleId)
{
    (void)OperationCycleId;
    return E_OK;
}

Std_ReturnType Dem_GetDTCSeverityAvailabilityMask(Dem_DTCSeverityType *DTCSeverityMask)
{
    if (DTCSeverityMask == NULL_PTR) { return E_NOT_OK; }
    *DTCSeverityMask = DEM_SEVERITY_CHECK_IMMEDIATELY |
                       DEM_SEVERITY_CHECK_AT_NEXT_HALT |
                       DEM_SEVERITY_MAINTENANCE_ONLY;
    return E_OK;
}

Std_ReturnType Dem_GetB1Counter(uint16 *B1Counter)
{
    if (B1Counter == NULL_PTR) { return E_NOT_OK; }
    *B1Counter = 0U;
    return E_OK;
}

Std_ReturnType Dem_SetDTR(uint8 DTRId, sint16 TestResult, sint16 LowerLimit,
                           sint16 UpperLimit, Dem_DTRControlType Ctrlval)
{
    (void)DTRId;
    (void)TestResult;
    (void)LowerLimit;
    (void)UpperLimit;
    (void)Ctrlval;
    return E_OK;
}

/* =========================================================================
 * §8.3.7  IUMPR  (compiled only when DEM_IUMPR_SUPPORT also ON)
 * ========================================================================= */

#if (DEM_IUMPR_SUPPORT == STD_ON)
Std_ReturnType Dem_RepIUMPRFaultDetect(Dem_RatioIdType RatioID)
{
    (void)RatioID; return E_OK;
}

Std_ReturnType Dem_SetIUMPRDenCondition(Dem_IumprDenomCondIdType ConditionId,
                                         Dem_IumprDenomCondStatusType ConditionStatus)
{
    (void)ConditionId; (void)ConditionStatus; return E_OK;
}

Std_ReturnType Dem_GetIUMPRDenCondition(Dem_IumprDenomCondIdType ConditionId,
                                         Dem_IumprDenomCondStatusType *ConditionStatus)
{
    (void)ConditionId;
    if (ConditionStatus != NULL_PTR) {
        *ConditionStatus = DEM_IUMPR_DEN_STATUS_NOT_REACHED;
    }
    return E_OK;
}

Std_ReturnType Dem_RepIUMPRDenRelease(Dem_RatioIdType RatioID)
{
    (void)RatioID; return E_OK;
}

Std_ReturnType Dem_SetPtoStatus(boolean PtoStatus)
{
    (void)PtoStatus; return E_OK;
}
#endif /* DEM_IUMPR_SUPPORT */

#endif /* DEM_OBD_SUPPORT */
