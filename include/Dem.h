/**
 * @file    Dem.h
 * @brief   AUTOSAR CP R23-11 - Diagnostic Event Manager public API (§8.3).
 *
 *  All optional API groups are guarded by their feature flag from Dem_Cfg.h.
 *  Include this header from any BSW module or SW-C that uses the DEM.
 */

#ifndef DEM_H
#define DEM_H

#include "Dem_Types.h"
#include "Dem_Cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * §8.3.1  Version info
 * ========================================================================= */
void Dem_GetVersionInfo(Std_VersionInfoType *versioninfo);

/* =========================================================================
 * §8.3.2  Lifecycle  (ECU State Manager <=> Dem)
 * ========================================================================= */
void           Dem_PreInit(void);
void           Dem_Init(const Dem_ConfigType *configPtr);
void           Dem_Shutdown(void);

/* =========================================================================
 * §8.3.3  BSW modules / SW-Components <=> Dem
 * ========================================================================= */

/* --- Always active -------------------------------------------------------- */
Std_ReturnType Dem_SetEventStatus(Dem_EventIdType EventId,
                                   Dem_EventStatusType EventStatus);

Std_ReturnType Dem_SetEventStatusWithMonitorData(Dem_EventIdType EventId,
                                                  Dem_EventStatusType EventStatus,
                                                  Dem_MonitorDataType MonitorData);

Std_ReturnType Dem_GetEventUdsStatus(Dem_EventIdType EventId,
                                      Dem_UdsStatusByteType *UDSStatusByte);

Std_ReturnType Dem_GetMonitorStatus(Dem_EventIdType EventId,
                                     Dem_MonitorStatusType *MonitorStatus);

Std_ReturnType Dem_GetDebouncingOfEvent(Dem_EventIdType EventId,
                                         Dem_DebouncingStateType *DebouncingState);

Std_ReturnType Dem_GetFaultDetectionCounter(Dem_EventIdType EventId,
                                             sint8 *FaultDetectionCounter);

Std_ReturnType Dem_ResetEventDebounceStatus(Dem_EventIdType EventId,
                                             Dem_DebounceResetStatusType DebounceResetStatus);

Std_ReturnType Dem_ResetEventStatus(Dem_EventIdType EventId);

Std_ReturnType Dem_ResetMonitorStatus(Dem_EventIdType EventId);

Std_ReturnType Dem_RestartOperationCycle(Dem_OperationCycleIdType OperationCycleId);

Std_ReturnType Dem_SelectDTC(uint8 ClientId,
                               uint32 DTC,
                               Dem_DTCFormatType DTCFormat,
                               Dem_DTCOriginType DTCOrigin);

Std_ReturnType Dem_ClearDTC(uint8 ClientId);

Std_ReturnType Dem_GetDTCSelectionResult(uint8 ClientId);

Std_ReturnType Dem_GetDTCSelectionResultForClearDTC(uint8 ClientId);

Std_ReturnType Dem_GetDTCOfEvent(Dem_EventIdType EventId,
                                  Dem_DTCFormatType DTCFormat,
                                  uint32 *DTCOfEvent);

Std_ReturnType Dem_GetEventMemoryOverflow(Dem_DTCOriginType DTCOrigin,
                                           boolean *isOverflow);

Std_ReturnType Dem_GetNumberOfEventMemoryEntries(Dem_DTCOriginType DTCOrigin,
                                                   uint8 *NumberOfEventMemoryEntries);

Std_ReturnType Dem_GetEventAvailable(Dem_EventIdType EventId,
                                      boolean *AvailableStatus);

Std_ReturnType Dem_SetEventAvailable(Dem_EventIdType EventId,
                                      boolean AvailableStatus);

Std_ReturnType Dem_SetEventConfirmationThresholdCounter(Dem_EventIdType EventId,
                                                         uint8 ThresholdCounter);

/* --- DEM_ENABLE_CONDITION_SUPPORT ---------------------------------------- */
#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
Std_ReturnType Dem_SetEnableCondition(uint8 EnableConditionID,
                                       boolean ConditionFulfilled);
#endif

/* --- DEM_STORAGE_CONDITION_SUPPORT --------------------------------------- */
#if (DEM_STORAGE_CONDITION_SUPPORT == STD_ON)
Std_ReturnType Dem_SetStorageCondition(uint8 StorageConditionID,
                                        boolean ConditionFulfilled);
#endif

/* --- DEM_SUPPRESSION_SUPPORT --------------------------------------------- */
#if (DEM_SUPPRESSION_SUPPORT == STD_ON)
Std_ReturnType Dem_GetDTCSuppression(Dem_EventIdType EventId,
                                      boolean *SuppressionStatus);

Std_ReturnType Dem_SetDTCSuppression(Dem_EventIdType EventId,
                                      boolean SuppressionStatus);
#endif

/* --- DEM_WIR_SUPPORT ----------------------------------------------------- */
#if (DEM_WIR_SUPPORT == STD_ON)
Std_ReturnType Dem_SetWIRStatus(Dem_EventIdType EventId,
                                  boolean WIRStatus);

Std_ReturnType Dem_GetIndicatorStatus(uint8 IndicatorId,
                                       Dem_IndicatorStatusType *IndicatorStatus);
#endif

/* --- DEM_FREEZE_FRAME_SUPPORT -------------------------------------------- */
#if (DEM_FREEZE_FRAME_SUPPORT == STD_ON)
Std_ReturnType Dem_GetEventFreezeFrameDataEx(Dem_EventIdType EventId,
                                              uint8 RecordNumber,
                                              uint16 DataIdentifier,
                                              uint8 *DestBuffer,
                                              uint16 *BufSize);
#endif

/* --- DEM_EXTENDED_DATA_SUPPORT ------------------------------------------- */
#if (DEM_EXTENDED_DATA_SUPPORT == STD_ON)
Std_ReturnType Dem_GetEventExtendedDataRecordEx(Dem_EventIdType EventId,
                                                 uint8 RecordNumber,
                                                 uint8 *DestBuffer,
                                                 uint16 *BufSize);
#endif

/* --- DEM_PRESTORAGE_SUPPORT ---------------------------------------------- */
#if (DEM_PRESTORAGE_SUPPORT == STD_ON)
Std_ReturnType Dem_PrestoreFreezeFrame(Dem_EventIdType EventId);
Std_ReturnType Dem_ClearPrestoredFreezeFrame(Dem_EventIdType EventId);
#endif

/* --- DEM_COMPONENT_SUPPORT ----------------------------------------------- */
#if (DEM_COMPONENT_SUPPORT == STD_ON)
Std_ReturnType Dem_GetComponentFailed(Dem_ComponentIdType ComponentId,
                                       boolean *ComponentFailed);

Std_ReturnType Dem_SetComponentAvailable(Dem_ComponentIdType ComponentId,
                                          boolean AvailableStatus);
#endif

/* =========================================================================
 * §8.3.4  DCM <=> Dem interface
 * ========================================================================= */

/* --- Access DTCs and Status Information ---------------------------------- */
Std_ReturnType Dem_GetTranslationType(uint8 ClientId,
                                       Dem_DTCTranslationFormatType *TranslationType);

Std_ReturnType Dem_GetDTCStatusAvailabilityMask(uint8 ClientId,
                                                  Dem_UdsStatusByteType *DTCStatusMask);

Std_ReturnType Dem_GetStatusOfDTC(uint8 ClientId,
                                    Dem_UdsStatusByteType *DTCStatus);

Std_ReturnType Dem_GetSeverityOfDTC(uint8 ClientId,
                                     Dem_DTCSeverityType *DTCSeverity);

Std_ReturnType Dem_GetFunctionalUnitOfDTC(uint8 ClientId,
                                           uint8 *DTCFunctionalUnit);

Std_ReturnType Dem_SetDTCFilter(uint8 ClientId,
                                  Dem_UdsStatusByteType DTCStatusMask,
                                  Dem_DTCKindType DTCKind,
                                  Dem_DTCFormatType DTCFormat,
                                  Dem_DTCOriginType DTCOrigin,
                                  Dem_FilterWithSeverityType FilterWithSeverity,
                                  Dem_DTCSeverityType DTCSeverityMask,
                                  Dem_FilterForFDCType FilterForFDC);

Std_ReturnType Dem_GetNumberOfFilteredDTC(uint8 ClientId,
                                           uint16 *NumberOfFilteredDTC);

Std_ReturnType Dem_GetNextFilteredDTC(uint8 ClientId,
                                       uint32 *DTC,
                                       Dem_UdsStatusByteType *DTCStatus);

Std_ReturnType Dem_GetNextFilteredDTCAndFDC(uint8 ClientId,
                                             uint32 *DTC,
                                             sint8 *DTCFaultDetectionCounter);

Std_ReturnType Dem_GetNextFilteredDTCAndSeverity(uint8 ClientId,
                                                   uint32 *DTC,
                                                   Dem_UdsStatusByteType *DTCStatus,
                                                   Dem_DTCSeverityType *DTCSeverity,
                                                   uint8 *DTCFunctionalUnit);

Std_ReturnType Dem_SetFreezeFrameRecordFilter(uint8 ClientId,
                                               uint8 RecordNumber,
                                               Dem_DTCOriginType DTCOrigin);

Std_ReturnType Dem_GetNextFilteredRecord(uint8 ClientId,
                                          uint32 *DTC,
                                          Dem_DTCFormatType DTCFormat,
                                          uint8 *RecordNumber);

Std_ReturnType Dem_GetDTCByOccurrenceTime(uint8 ClientId,
                                           Dem_DTCRequestType DTCRequest,
                                           uint32 *DTC);

Std_ReturnType Dem_SetDTCFilterByExtendedDataRecordNumber(uint8 ClientId,
                                                            uint8 EDRNumber);

Std_ReturnType Dem_SetDTCFilterByReadinessGroup(uint8 ClientId,
                                                  Dem_EventOBDReadinessGroupType ReadinessGroupNumber);

/* --- Access FreezeFrame and Extended Data -------------------------------- */
Std_ReturnType Dem_DisableDTCRecordUpdate(uint8 ClientId);
Std_ReturnType Dem_EnableDTCRecordUpdate(uint8 ClientId);

Std_ReturnType Dem_GetSizeOfExtendedDataRecordSelection(uint8 ClientId,
                                                          uint16 *SizeOfExtendedDataRecord);

Std_ReturnType Dem_GetSizeOfFreezeFrameSelection(uint8 ClientId,
                                                   uint16 *SizeOfFreezeFrame);

Std_ReturnType Dem_GetNextExtendedDataRecord(uint8 ClientId,
                                              uint8 *DestBuffer,
                                              uint16 *BufSize);

Std_ReturnType Dem_GetNextFreezeFrameData(uint8 ClientId,
                                           uint8 *DestBuffer,
                                           uint16 *BufSize);

Std_ReturnType Dem_SelectExtendedDataRecord(uint8 ClientId,
                                             uint8 ExtendedDataNumber);

Std_ReturnType Dem_SelectFreezeFrameData(uint8 ClientId,
                                          uint8 RecordNumber);

Std_ReturnType Dem_GetNumberOfFreezeFrameRecords(uint8 ClientId,
                                                   uint8 *NumberOfFilteredRecords);

/* --- DTC storage control ------------------------------------------------- */
Std_ReturnType Dem_DisableDTCSetting(uint32 DTCGroup);
Std_ReturnType Dem_EnableDTCSetting(uint32 DTCGroup);

/* =========================================================================
 * §8.3.5  OBD-specific Dcm <=> Dem
 * ========================================================================= */
#if (DEM_OBD_SUPPORT == STD_ON)
Std_ReturnType Dem_DcmGetInfoTypeValue08(uint8 ClientId, uint8 *Iumprdenominator,
                                          uint8 *Iumprdenominator_size);
Std_ReturnType Dem_DcmGetInfoTypeValue0B(uint8 ClientId, uint8 *Iumprdenominator,
                                          uint8 *Iumprdenominator_size);
Std_ReturnType Dem_DcmReadDataOfPID01(uint8 *PID01value);
Std_ReturnType Dem_DcmReadDataOfPID1C(uint8 *PID1Cvalue);
Std_ReturnType Dem_DcmReadDataOfPID21(uint8 *PID21value);
Std_ReturnType Dem_DcmReadDataOfPID30(uint8 *PID30value);
Std_ReturnType Dem_DcmReadDataOfPID31(uint8 *PID31value);
Std_ReturnType Dem_DcmReadDataOfPID41(uint8 *PID41value);
Std_ReturnType Dem_DcmReadDataOfPID4D(uint8 *PID4Dvalue);
Std_ReturnType Dem_DcmReadDataOfPID4E(uint8 *PID4Evalue);
Std_ReturnType Dem_DcmReadDataOfPID91(uint8 *PID91value);
Std_ReturnType Dem_DcmReadDataOfOBDFreezeFrame(uint8 PID, uint8 DataElementIndexOfPID,
                                                 uint8 *DestBuffer, uint16 *BufSize);
Std_ReturnType Dem_DcmGetDTCOfOBDFreezeFrame(uint8 FrameNumber, uint32 *DTC,
                                               Dem_DTCFormatType DTCFormat);
Std_ReturnType Dem_DcmGetAvailableOBDMIDs(uint8 MIDValue, uint32 *MIDMask);
Std_ReturnType Dem_DcmGetNumTIDsOfOBDMID(uint8 MIDValue, uint8 *NumberOfTIDs);
Std_ReturnType Dem_DcmGetDTRData(uint8 TIDindex, uint8 *TIDvalue, uint8 *UaSID,
                                   uint16 *Testvalue, uint16 *Lowlimvalue,
                                   uint16 *Upplimvalue);
Std_ReturnType Dem_DcmGetInfoTypeValue79(uint8 ClientId, uint8 *Iumprdenominator,
                                          uint8 *Iumprdenominator_size);
Std_ReturnType Dem_DcmReadDataOfPIDF501(uint8 *DataValueBuffer, uint8 *DataValueBufferSize);
#endif /* DEM_OBD_SUPPORT */

/* =========================================================================
 * §8.3.6  J1939Dcm <=> Dem
 * ========================================================================= */
#if (DEM_J1939_SUPPORT == STD_ON)
Std_ReturnType Dem_J1939DcmSetDTCFilter(Dem_J1939DcmDTCStatusFilterType DTCStatusFilter,
                                          Dem_DTCOriginType DTCOrigin,
                                          uint8 ClientId);
Std_ReturnType Dem_J1939DcmGetNumberOfFilteredDTC(uint16 *NumberOfFilteredDTC,
                                                    uint8 ClientId);
Std_ReturnType Dem_J1939DcmGetNextFilteredDTC(uint32 *J1939DTC,
                                               Dem_UdsStatusByteType *DTCStatus,
                                               uint8 ClientId);
Std_ReturnType Dem_J1939DcmFirstDTCwithLampStatus(Dem_J1939DcmLampStatusType *LampStatus,
                                                    uint32 *J1939DTC,
                                                    Dem_UdsStatusByteType *DTCStatus,
                                                    uint8 ClientId);
Std_ReturnType Dem_J1939DcmGetNextDTCwithLampStatus(Dem_J1939DcmLampStatusType *LampStatus,
                                                      uint32 *J1939DTC,
                                                      Dem_UdsStatusByteType *DTCStatus,
                                                      uint8 ClientId);
Std_ReturnType Dem_J1939DcmClearDTC(uint32 DTCGroup, Dem_DTCOriginType DTCOrigin,
                                     uint8 ClientId);
Std_ReturnType Dem_J1939DcmSetFreezeFrameFilter(Dem_J1939DcmSetFreezeFrameFilterType FreezeFrameKind,
                                                  uint8 ClientId);
Std_ReturnType Dem_J1939DcmGetNextFreezeFrame(uint32 *J1939DTC,
                                               Dem_J1939DcmLampStatusType *LampStatus,
                                               uint8 *DestBuffer, uint16 *BufSize,
                                               uint8 ClientId);
Std_ReturnType Dem_J1939DcmGetNextSPNInFreezeFrame(uint32 *SPNSupported,
                                                     uint32 *SPNDataLength,
                                                     uint8 ClientId);
Std_ReturnType Dem_J1939DcmSetRatioFilter(Dem_J1939DcmSetFreezeFrameFilterType RatioFilter,
                                           uint8 ClientId);
Std_ReturnType Dem_J1939DcmGetNextFilteredRatio(uint16 *SPN, uint16 *Numerator,
                                                  uint16 *Denominator, uint8 ClientId);
Std_ReturnType Dem_J1939DcmReadDiagnosticReadiness1(
        Dem_J1939DcmDiagnosticReadiness1Type *DataValue, uint8 ClientId);
Std_ReturnType Dem_J1939DcmReadDiagnosticReadiness2(
        Dem_J1939DcmDiagnosticReadiness2Type *DataValue, uint8 ClientId);
Std_ReturnType Dem_J1939DcmReadDiagnosticReadiness3(
        Dem_J1939DcmDiagnosticReadiness3Type *DataValue, uint8 ClientId);
#endif /* DEM_J1939_SUPPORT */

/* =========================================================================
 * §8.3.7  OBD-specific interfaces (SW-C / BSW side)
 * ========================================================================= */
#if (DEM_OBD_SUPPORT == STD_ON)
Std_ReturnType Dem_SetEventDisabled(Dem_EventIdType EventId);
Std_ReturnType Dem_GetDataOfPID21(uint8 *PID21value);
Std_ReturnType Dem_SetDataOfPID21(uint8 PID21value);
Std_ReturnType Dem_SetDataOfPID31(uint8 PID31value);
Std_ReturnType Dem_SetDataOfPID4D(uint8 PID4Dvalue);
Std_ReturnType Dem_SetDataOfPID4E(uint8 PID4Evalue);
Std_ReturnType Dem_GetCycleQualified(Dem_OperationCycleIdType OperationCycleId,
                                      boolean *isQualified);
Std_ReturnType Dem_SetCycleQualified(Dem_OperationCycleIdType OperationCycleId);
Std_ReturnType Dem_GetDTCSeverityAvailabilityMask(Dem_DTCSeverityType *DTCSeverityMask);
Std_ReturnType Dem_GetB1Counter(uint16 *B1Counter);
Std_ReturnType Dem_SetDTR(uint8 DTRId, sint16 TestResult, sint16 LowerLimit,
                           sint16 UpperLimit, Dem_DTRControlType Ctrlval);
#endif

#if (DEM_IUMPR_SUPPORT == STD_ON)
Std_ReturnType Dem_RepIUMPRFaultDetect(Dem_RatioIdType RatioID);
Std_ReturnType Dem_SetIUMPRDenCondition(Dem_IumprDenomCondIdType ConditionId,
                                         Dem_IumprDenomCondStatusType ConditionStatus);
Std_ReturnType Dem_GetIUMPRDenCondition(Dem_IumprDenomCondIdType ConditionId,
                                         Dem_IumprDenomCondStatusType *ConditionStatus);
Std_ReturnType Dem_RepIUMPRDenRelease(Dem_RatioIdType RatioID);
Std_ReturnType Dem_SetPtoStatus(boolean PtoStatus);
#endif

/* =========================================================================
 * §8.5  Scheduled function
 * ========================================================================= */
void Dem_MainFunction(void);

/* =========================================================================
 * NvM callbacks  (§7.11.5 - called by NvM module)
 * ========================================================================= */
Std_ReturnType Dem_NvM_InitBlockCallback(void);
Std_ReturnType Dem_NvM_JobFinishedCallback(uint8 ServiceId, Std_ReturnType JobResult);

/* =========================================================================
 * Missing type referenced in Dem.h (translation format)
 * ========================================================================= */
typedef uint8 Dem_DTCTranslationFormatType;
#define DEM_DTC_TRANSLATION_ISO15031_6      (0x00U)
#define DEM_DTC_TRANSLATION_ISO14229_1      (0x01U)
#define DEM_DTC_TRANSLATION_SAEJ2012_DA     (0x02U)
#define DEM_DTC_TRANSLATION_ISO11992_4      (0x03U)
#define DEM_DTC_TRANSLATION_J1939_73        (0x04U)

#ifdef __cplusplus
}
#endif

#endif /* DEM_H */
