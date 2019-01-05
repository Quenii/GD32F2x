/*!
    \file  usbd_flash_if.c
    \brief USB DFU device flash interface functions

    \version 2015-07-15, V1.0.0, firmware for GD32F20x
    \version 2017-06-05, V2.0.0, firmware for GD32F20x
    \version 2018-10-31, V2.1.0, firmware for GD32F20x
*/

/*
    Copyright (c) 2018, GigaDevice Semiconductor Inc.

    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "flash_if.h"
#include "dfu_mal.h"

static uint8_t  Flash_If_Init      (void);
static uint8_t  Flash_If_DeInit    (void);
static uint8_t  Flash_If_Erase     (uint32_t Addr);
static uint8_t  Flash_If_Write     (uint32_t Addr, uint32_t Len);
static uint8_t* Flash_If_Read      (uint32_t Addr, uint32_t Len);
static uint8_t  Flash_If_CheckAddr (uint32_t Addr);

/* USBD_DFU_FlashIf_Private_Variables */
DFU_MAL_Property_TypeDef DFU_Flash_cb =
{
    (const uint8_t *)FLASH_IF_STRING,

    Flash_If_Init,
    Flash_If_DeInit,
    Flash_If_Erase,
    Flash_If_Write,
    Flash_If_Read,
    Flash_If_CheckAddr,
    60, /* flash erase timeout in ms */
    80  /* flash programming timeout in ms (80us * ram buffer size (1024 bytes) */
};

/*!
    \brief      flash memory interface initialization routine.
    \param[in]  none
    \param[out] none
    \retval     MAL_OK if the operation is right, MAL_FAIL else
*/
static uint8_t  Flash_If_Init (void)
{
    /* unlock the internal flash */
    fmc_unlock();

    return MAL_OK;
}

/*!
    \brief      flash memory interface deinitialization routine.
    \param[in]  none
    \param[out] none
    \retval     MAL_OK if all operation is right, MAL_FAIL else
*/
static uint8_t  Flash_If_DeInit (void)
{
    /* lock the internal flash */
    fmc_lock();

    return MAL_OK;
}

/*!
    \brief      erase flash sector
    \param[in]  Addr: address to be written to.
    \param[out] none
    \retval     MAL_OK
*/
static uint8_t  Flash_If_Erase (uint32_t Addr)
{
    fmc_page_erase(Addr);

    return MAL_OK;
}

/*!
    \brief      flash memory write routine
    \param[in]  Addr: address to be written to
    \param[in]  Len: length of data to be written (in bytes).
    \param[out] none
    \retval     MAL_OK
*/
static uint8_t  Flash_If_Write (uint32_t Addr, uint32_t Len)
{
    uint32_t idx = 0;

    /* unlock the flash program erase controller */
    fmc_unlock();

    /* clear pending flags */
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR | FMC_FLAG_BANK0_WPERR  | FMC_FLAG_BANK0_END \
                  | FMC_FLAG_BANK1_PGERR | FMC_FLAG_BANK1_WPERR | FMC_FLAG_BANK1_END);

    /* not an aligned data */
    if (Len & 0x03) {
        for (idx = Len; idx < ((Len & 0xFFFC) + 4); idx++) {
            MAL_Buffer[idx] = 0xFF;
        }
    }

    /* data received are word multiple */
    for (idx = 0; idx < Len; idx += 4) {
        fmc_word_program(Addr, *(uint32_t *)(MAL_Buffer + idx));
        Addr += 4;
    }

    fmc_lock();

    return MAL_OK;
}

/*!
    \brief      flash memory read routine
    \param[in]  Addr: address to be read from
    \param[in]  Len: length of data to be read (in bytes)
    \param[out] none
    \retval     Pointer to the physical address where data should be read
*/
static uint8_t *Flash_If_Read (uint32_t Addr, uint32_t Len)
{
    return  (uint8_t *)(Addr);
}

/*!
    \brief      check if the address is an allowed address for this memory
    \param[in]  Addr: address to be checked.
    \param[out] none
    \retval     MAL_OK if the address is allowed, MAL_FAIL else.
*/
static uint8_t  Flash_If_CheckAddr (uint32_t Addr)
{
    if ((Addr >= FLASH_START_ADDR) && (Addr < FLASH_END_ADDR)) {
        return MAL_OK;
    } else {
        return MAL_FAIL;
    }
}
