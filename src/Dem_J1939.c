/**
 * @file    Dem_J1939.c
 * @brief   AUTOSAR CP R23-11 - DEM J1939Dcm interface stubs (§8.3.6).
 *
 *  All functions are compiled only when DEM_J1939_SUPPORT == STD_ON.
 *  Returns E_OK with zeroed outputs as a starting point.
 */

#include "Dem_Internal.h"

#if (DEM_J1939_SUPPORT == STD_ON)

Std_ReturnType Dem_J1939DcmSetDTCFilter(Dem_J1939DcmDTCStatusFilterType DTCStatusFilter,
                                          Dem_DTCOriginType DTCOrigin,
                                          uint8 ClientId)
{
    (void)DTCStatusFilter; (void)DTCOrigin; (void)ClientId;
    return E_OK;
}

Std_ReturnType Dem_J1939DcmGetNumberOfFilteredDTC(uint16 *NumberOfFilteredDTC,
                                                    uint8 ClientId)
{
    (void)ClientId;
    if (NumberOfFilteredDTC == NULL_PTR) { return E_NOT_OK; }
    *NumberOfFilteredDTC = 0U;
    return E_OK;
}

Std_ReturnType Dem_J1939DcmGetNextFilteredDTC(uint32 *J1939DTC,
                                               Dem_UdsStatusByteType *DTCStatus,
                                               uint8 ClientId)
{
    (void)ClientId;
    if ((J1939DTC == NULL_PTR) || (DTCStatus == NULL_PTR)) { return E_NOT_OK; }
    *J1939DTC  = 0U;
    *DTCStatus = 0U;
    return DEM_FILTERED_NO_MATCHING_DTC;
}

Std_ReturnType Dem_J1939DcmFirstDTCwithLampStatus(Dem_J1939DcmLampStatusType *LampStatus,
                                                    uint32 *J1939DTC,
                                                    Dem_UdsStatusByteType *DTCStatus,
                                                    uint8 ClientId)
{
    (void)ClientId;
    if ((LampStatus == NULL_PTR) || (J1939DTC == NULL_PTR) || (DTCStatus == NULL_PTR)) {
        return E_NOT_OK;
    }
    LampStatus->LampStatus      = 0U;
    LampStatus->FlashLampStatus = 0U;
    *J1939DTC  = 0U;
    *DTCStatus = 0U;
    return DEM_NO_SUCH_ELEMENT;
}

Std_ReturnType Dem_J1939DcmGetNextDTCwithLampStatus(Dem_J1939DcmLampStatusType *LampStatus,
                                                      uint32 *J1939DTC,
                                                      Dem_UdsStatusByteType *DTCStatus,
                                                      uint8 ClientId)
{
    (void)ClientId;
    if ((LampStatus == NULL_PTR) || (J1939DTC == NULL_PTR) || (DTCStatus == NULL_PTR)) {
        return E_NOT_OK;
    }
    *J1939DTC  = 0U;
    *DTCStatus = 0U;
    return DEM_FILTERED_NO_MATCHING_DTC;
}

Std_ReturnType Dem_J1939DcmClearDTC(uint32 DTCGroup,
                                     Dem_DTCOriginType DTCOrigin,
                                     uint8 ClientId)
{
    (void)DTCOrigin; (void)ClientId;
    /* Reuse UDS clear logic */
    return Dem_ClearDTC(ClientId);
}

Std_ReturnType Dem_J1939DcmSetFreezeFrameFilter(Dem_J1939DcmSetFreezeFrameFilterType FreezeFrameKind,
                                                  uint8 ClientId)
{
    (void)FreezeFrameKind; (void)ClientId;
    return E_OK;
}

Std_ReturnType Dem_J1939DcmGetNextFreezeFrame(uint32 *J1939DTC,
                                               Dem_J1939DcmLampStatusType *LampStatus,
                                               uint8 *DestBuffer, uint16 *BufSize,
                                               uint8 ClientId)
{
    (void)ClientId;
    if ((J1939DTC == NULL_PTR) || (LampStatus == NULL_PTR) ||
        (DestBuffer == NULL_PTR) || (BufSize == NULL_PTR)) {
        return E_NOT_OK;
    }
    *J1939DTC = 0U;
    *BufSize  = 0U;
    return DEM_FILTERED_NO_MATCHING_DTC;
}

Std_ReturnType Dem_J1939DcmGetNextSPNInFreezeFrame(uint32 *SPNSupported,
                                                     uint32 *SPNDataLength,
                                                     uint8 ClientId)
{
    (void)ClientId;
    if ((SPNSupported == NULL_PTR) || (SPNDataLength == NULL_PTR)) { return E_NOT_OK; }
    *SPNSupported  = 0U;
    *SPNDataLength = 0U;
    return DEM_FILTERED_NO_MATCHING_DTC;
}

Std_ReturnType Dem_J1939DcmSetRatioFilter(Dem_J1939DcmSetFreezeFrameFilterType RatioFilter,
                                           uint8 ClientId)
{
    (void)RatioFilter; (void)ClientId;
    return E_OK;
}

Std_ReturnType Dem_J1939DcmGetNextFilteredRatio(uint16 *SPN, uint16 *Numerator,
                                                  uint16 *Denominator, uint8 ClientId)
{
    (void)ClientId;
    if ((SPN == NULL_PTR) || (Numerator == NULL_PTR) || (Denominator == NULL_PTR)) {
        return E_NOT_OK;
    }
    *SPN = 0U; *Numerator = 0U; *Denominator = 0U;
    return DEM_FILTERED_NO_MATCHING_DTC;
}

Std_ReturnType Dem_J1939DcmReadDiagnosticReadiness1(
        Dem_J1939DcmDiagnosticReadiness1Type *DataValue, uint8 ClientId)
{
    (void)ClientId;
    if (DataValue == NULL_PTR) { return E_NOT_OK; }
    uint8 i;
    for (i = 0U; i < 4U; i++) { DataValue->OBDReadinessStatus[i] = 0U; }
    return E_OK;
}

Std_ReturnType Dem_J1939DcmReadDiagnosticReadiness2(
        Dem_J1939DcmDiagnosticReadiness2Type *DataValue, uint8 ClientId)
{
    (void)ClientId;
    if (DataValue == NULL_PTR) { return E_NOT_OK; }
    uint8 i;
    for (i = 0U; i < 4U; i++) { DataValue->OBDReadinessStatus[i] = 0U; }
    return E_OK;
}

Std_ReturnType Dem_J1939DcmReadDiagnosticReadiness3(
        Dem_J1939DcmDiagnosticReadiness3Type *DataValue, uint8 ClientId)
{
    (void)ClientId;
    if (DataValue == NULL_PTR) { return E_NOT_OK; }
    uint8 i;
    for (i = 0U; i < 4U; i++) { DataValue->OBDReadinessStatus[i] = 0U; }
    return E_OK;
}

#endif /* DEM_J1939_SUPPORT */
