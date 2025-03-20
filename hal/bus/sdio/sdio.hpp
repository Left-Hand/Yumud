// /**
//  * Marlin 3D Printer Firmware
//  *
//  * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
//  * Copyright (c) 2016 Bob Cousins bobcousins42@googlemail.com
//  * Copyright (c) 2017 Victor Perez
//  *
//  * This program is free software: you can redistribute it and/or modify
//  * it under the terms of the GNU General Public License as published by
//  * the Free Software Foundation, either version 3 of the License, or
//  * (at your option) any later version.
//  *
//  * This program is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU General Public License for more details.
//  *
//  * You should have received a copy of the GNU General Public License
//  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
//  *
//  */
// #pragma once

// #include "core/platform.hpp"


// // ------------------------
// // Types
// // ------------------------

// typedef struct {
//   uint32_t CardType;      // Card Type
//   uint32_t CardVersion;   // Card version
//   uint32_t Class;         // Class of the card class
//   uint32_t RelCardAdd;    // Relative Card Address
//   uint32_t BlockNbr;      // Card Capacity in blocks
//   uint32_t BlockSize;     // One block size in bytes
//   uint32_t LogBlockNbr;   // Card logical Capacity in blocks
//   uint32_t LogBlockSize;  // Logical block size in bytes
// } SDIO_CardInfoTypeDef;

// // ------------------------
// // Public functions
// // ------------------------

// inline uint32_t SDIO_GetCardState();

// bool SDIO_CmdGoIdleState();
// bool SDIO_CmdSendCID();
// bool SDIO_CmdSetRelAdd(uint32_t *rca);
// bool SDIO_CmdSelDesel(uint32_t address);
// bool SDIO_CmdOperCond();
// bool SDIO_CmdSendCSD(uint32_t argument);
// bool SDIO_CmdSendStatus(uint32_t argument);
// bool SDIO_CmdReadSingleBlock(uint32_t address);
// bool SDIO_CmdWriteSingleBlock(uint32_t address);
// bool SDIO_CmdAppCommand(uint32_t rsa);

// bool SDIO_CmdAppSetBusWidth(uint32_t rsa, uint32_t argument);
// bool SDIO_CmdAppOperCommand(uint32_t sdType);
// bool SDIO_CmdAppSetClearCardDetect(uint32_t rsa);

// void SDIO_SendCommand(uint16_t command, uint32_t argument);
// uint8_t SDIO_GetCommandResponse();
// uint32_t SDIO_GetResponse(uint32_t response);
// bool SDIO_GetCmdError();
// bool SDIO_GetCmdResp1(uint8_t command);
// bool SDIO_GetCmdResp2();
// bool SDIO_GetCmdResp3();
// bool SDIO_GetCmdResp6(uint8_t command, uint32_t *rca);
// bool SDIO_GetCmdResp7();
