/**
 * @file    Dem_FreezeFrameProvider.h
 * @brief   External FreezeFrame provider contract for this DEM implementation.
 *
 * FreezeFrame is stored in primary memory as a fixed 16-byte blob
 * (DEM_FREEZE_FRAME_SIZE). DEM will request the data from the upper layer
 * via the ReadDataElement callback declared in Dem_Cbk.h.
 *
 * Default layout (little-endian for multi-byte values):
 *  Byte0      : UdsStatusByte
 *  Byte1..4   : Timestamp (uint32)
 *  Byte5..6   : BatteryVoltage_mV (uint16)
 *  Byte7..8   : Temp1_centiDegC (sint16)  (°C * 100)
 *  Byte9..10  : Temp2_centiDegC (sint16)  (°C * 100)
 *  Byte11     : OccurrenceCounter
 *  Byte12..15 : reserved
 */

#ifndef DEM_FREEZEFRAMEPROVIDER_H
#define DEM_FREEZEFRAMEPROVIDER_H

#include "Dem_Types.h"

/* Data element IDs used with Dem_ReadDataElement(...) */
#define DEM_FF_DEID_UDS_STATUS           (0x0001U) /* out: uint8  */
#define DEM_FF_DEID_TIMESTAMP            (0x0002U) /* out: uint32 */
#define DEM_FF_DEID_BATTERY_MV           (0x0003U) /* out: uint16 */
#define DEM_FF_DEID_TEMP1_CENTIDEGC      (0x0004U) /* out: sint16 */
#define DEM_FF_DEID_TEMP2_CENTIDEGC      (0x0005U) /* out: sint16 */
#define DEM_FF_DEID_OCCURRENCE_COUNTER   (0x0006U) /* out: uint8  */

/* Helper: pack little-endian */
static inline void Dem_FF_PackU16LE(uint8 *dst, uint16 v)
{
    dst[0] = (uint8)(v & 0xFFU);
    dst[1] = (uint8)((v >> 8U) & 0xFFU);
}

static inline void Dem_FF_PackS16LE(uint8 *dst, sint16 v)
{
    Dem_FF_PackU16LE(dst, (uint16)v);
}

static inline void Dem_FF_PackU32LE(uint8 *dst, uint32 v)
{
    dst[0] = (uint8)(v & 0xFFU);
    dst[1] = (uint8)((v >> 8U) & 0xFFU);
    dst[2] = (uint8)((v >> 16U) & 0xFFU);
    dst[3] = (uint8)((v >> 24U) & 0xFFU);
}

#endif /* DEM_FREEZEFRAMEPROVIDER_H */

