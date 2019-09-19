#include "appmain.h"

#include "usb_cdc.h"
#include "proxmark3_arm.h"
#include "dbprint.h"
#include "pmflash.h"
#include "fpga.h"
#include "fpgaloader.h"
#include "string.h"
#include "legicrf.h"
#include "BigBuf.h"
#include "iso14443a.h"
#include "iso14443b.h"
#include "iso15693.h"
#include "thinfilm.h"
#include "felica.h"
#include "hitag2.h"
#include "hitagS.h"
#include "iclass.h"
#include "legicrfsim.h"
#include "epa.h"
#include "hfsnoop.h"
#include "lfops.h"
#include "lfsampling.h"
#include "mifarecmd.h"
#include "mifaredesfire.h"
#include "mifaresim.h"
#include "pcf7931.h"
#include "Standalone/standalone.h"
#include "util.h"
#include "ticks.h"
#include "Standalone/hf_colin_esp32_appmain.h"

#ifdef WITH_LCD
#include "LCD.h"
#endif

#ifdef WITH_SMARTCARD
#include "i2c.h"
#endif

#ifdef WITH_FPC_USART
#include "usart.h"
#endif

#ifdef WITH_FLASH
#include "flashmem.h"
#include "spiffs.h"
#endif

extern struct common_area common_area;
extern int button_status;

int ESP32PacketReceived(PacketCommandNG *packet) {
    /*
    if (packet->ng) {
        Dbprintf("received NG frame with %d bytes payload, with command: 0x%04x", packet->length, cmd);
    } else {
        Dbprintf("received OLD frame of %d bytes, with command: 0x%04x and args: %d %d %d", packet->length, packet->cmd, packet->oldarg[0], packet->oldarg[1], packet->oldarg[2]);
    }
    */

    switch (packet->cmd) {
        case ESP32CMD_HFCOLIN:
	    return 9998;
	     break;
        case ESP32CMD_QUIT:
	    return 9999;
	    break;
        case CMD_QUIT_SESSION:
            reply_via_fpc = false;
            reply_via_usb = false;
            break;
// always available
        case CMD_HF_DROPFIELD: {
            hf_field_off();
            break;
        }

        case CMD_HF_ISO14443A_SNIFF: {
            SniffIso14443a(packet->data.asBytes[0]);
            break;
        }
        case CMD_HF_ISO14443A_READER: {
            ReaderIso14443a(packet);
            break;
        }
        case CMD_HF_ISO14443A_SIMULATE: {
            struct p {
                uint8_t tagtype;
                uint8_t flags;
                uint8_t uid[10];
            } PACKED;
            struct p *payload = (struct p *) packet->data.asBytes;
            SimulateIso14443aTag(payload->tagtype, payload->flags, payload->uid);  // ## Simulate iso14443a tag - pass tag type & UID
            break;
        }
        case CMD_HF_ISO14443A_ANTIFUZZ: {
            iso14443a_antifuzz(packet->oldarg[0]);
            break;
        }
        case CMD_HF_MIFARE_READER: {
            ReaderMifare(packet->oldarg[0], packet->oldarg[1], packet->oldarg[2]);
            break;
        }
        case CMD_HF_MIFARE_READBL: {
            mf_readblock_t *payload = (mf_readblock_t *)packet->data.asBytes;
            MifareReadBlock(payload->blockno, payload->keytype, payload->key);
            break;
        }
        case CMD_HF_MIFARE_READSC: {
            MifareReadSector(packet->oldarg[0], packet->oldarg[1], packet->data.asBytes);
            break;
        }
        case CMD_HF_MIFARE_WRITEBL: {
            MifareWriteBlock(packet->oldarg[0], packet->oldarg[1], packet->data.asBytes);
            break;
        }
        case CMD_HF_MIFARE_ACQ_ENCRYPTED_NONCES: {
            MifareAcquireEncryptedNonces(packet->oldarg[0], packet->oldarg[1], packet->oldarg[2], packet->data.asBytes);
            break;
        }
        case CMD_HF_MIFARE_ACQ_NONCES: {
            MifareAcquireNonces(packet->oldarg[0], packet->oldarg[2]);
            break;
        }
        case CMD_HF_MIFARE_NESTED: {
            MifareNested(packet->oldarg[0], packet->oldarg[1], packet->oldarg[2], packet->data.asBytes);
            break;
        }
        case CMD_HF_MIFARE_CHKKEYS: {
            MifareChkKeys(packet->data.asBytes);
            break;
        }
        case CMD_HF_MIFARE_CHKKEYS_FAST: {
            MifareChkKeys_fast(packet->oldarg[0], packet->oldarg[1], packet->oldarg[2], packet->data.asBytes);
            break;
        }
        case CMD_HF_MIFARE_SIMULATE: {
            struct p {
                uint16_t flags;
                uint8_t exitAfter;
                uint8_t uid[10];
                uint16_t atqa;
                uint8_t sak;
            } PACKED;
            struct p *payload = (struct p *) packet->data.asBytes;
            Mifare1ksim(payload->flags, payload->exitAfter, payload->uid, payload->atqa, payload->sak);
            break;
        }
        // emulator
        case CMD_SET_DBGMODE: {
            DBGLEVEL = packet->data.asBytes[0];
            Dbprintf("Debug level: %d", DBGLEVEL);
            reply_ng(CMD_SET_DBGMODE, PM3_SUCCESS, NULL, 0);
            break;
        }
        case CMD_HF_MIFARE_EML_MEMCLR: {
            MifareEMemClr();
            reply_ng(CMD_HF_MIFARE_EML_MEMCLR, PM3_SUCCESS, NULL, 0);
            break;
        }
        case CMD_HF_MIFARE_EML_MEMSET: {
            struct p {
                uint8_t blockno;
                uint8_t blockcnt;
                uint8_t blockwidth;
                uint8_t data[];
            } PACKED;
            struct p *payload = (struct p *) packet->data.asBytes;
            MifareEMemSet(payload->blockno, payload->blockcnt, payload->blockwidth, payload->data);
            break;
        }
        case CMD_HF_MIFARE_EML_MEMGET: {
            struct p {
                uint8_t blockno;
                uint8_t blockcnt;
            } PACKED;
            struct p *payload = (struct p *) packet->data.asBytes;
            MifareEMemGet(payload->blockno, payload->blockcnt);
            break;
        }
        case CMD_HF_MIFARE_EML_LOAD: {
            MifareECardLoad(packet->oldarg[0], packet->oldarg[1]);
            break;
        }
        // Work with "magic Chinese" card
        case CMD_HF_MIFARE_CSETBL: {
            MifareCSetBlock(packet->oldarg[0], packet->oldarg[1], packet->data.asBytes);
            break;
        }
        case CMD_HF_MIFARE_CGETBL: {
            MifareCGetBlock(packet->oldarg[0], packet->oldarg[1], packet->data.asBytes);
            break;
        }
        case CMD_HF_MIFARE_CIDENT: {
            MifareCIdent();
            break;
        }
        case CMD_HF_MIFARE_SETMOD: {
            MifareSetMod(packet->data.asBytes);
            break;
        }
        case CMD_HF_MIFARE_NACK_DETECT: {
            DetectNACKbug();
            break;
        }
#ifdef WITH_HFSNIFF
        case CMD_HF_SNIFF: {
            HfSniff(packet->oldarg[0], packet->oldarg[1]);
            break;
        }
#endif
        case CMD_BUFF_CLEAR: {
            BigBuf_Clear();
            BigBuf_free();
            break;
        }
        case CMD_MEASURE_ANTENNA_TUNING: {
            MeasureAntennaTuning();
            break;
        }
        case CMD_MEASURE_ANTENNA_TUNING_HF: {
            if (packet->length != 1)
                reply_ng(CMD_MEASURE_ANTENNA_TUNING_HF, PM3_EINVARG, NULL, 0);
            switch (packet->data.asBytes[0]) {
                case 1: // MEASURE_ANTENNA_TUNING_HF_START
                    // Let the FPGA drive the high-frequency antenna around 13.56 MHz.
                    FpgaDownloadAndGo(FPGA_BITSTREAM_HF);
                    FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER_RX_XCORR);
                    reply_ng(CMD_MEASURE_ANTENNA_TUNING_HF, PM3_SUCCESS, NULL, 0);
                    break;
                case 2:
                    if (button_status == BUTTON_SINGLE_CLICK)
                        reply_ng(CMD_MEASURE_ANTENNA_TUNING_HF, PM3_EOPABORTED, NULL, 0);
                    uint16_t volt = MeasureAntennaTuningHfData();
                    reply_ng(CMD_MEASURE_ANTENNA_TUNING_HF, PM3_SUCCESS, (uint8_t *)&volt, sizeof(volt));
                    break;
                case 3:
                    FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
                    reply_ng(CMD_MEASURE_ANTENNA_TUNING_HF, PM3_SUCCESS, NULL, 0);
                    break;
                default:
                    reply_ng(CMD_MEASURE_ANTENNA_TUNING_HF, PM3_EINVARG, NULL, 0);
                    break;
            }
            break;
        }
        case CMD_LISTEN_READER_FIELD: {
            if (packet->length != sizeof(uint8_t))
                break;
            ListenReaderField(packet->data.asBytes[0]);
            break;
        }
        case CMD_FPGA_MAJOR_MODE_OFF: { // ## FPGA Control
            FpgaWriteConfWord(FPGA_MAJOR_MODE_OFF);
            SpinDelay(200);
            LED_D_OFF(); // LED D indicates field ON or OFF
            break;
        }
        case CMD_DOWNLOAD_BIGBUF: {
            LED_B_ON();
            uint8_t *mem = BigBuf_get_addr();
            uint32_t startidx = packet->oldarg[0];
            uint32_t numofbytes = packet->oldarg[1];

            // arg0 = startindex
            // arg1 = length bytes to transfer
            // arg2 = BigBuf tracelen
            //Dbprintf("transfer to client parameters: %" PRIu32 " | %" PRIu32 " | %" PRIu32, startidx, numofbytes, packet->oldarg[2]);

            for (size_t i = 0; i < numofbytes; i += PM3_CMD_DATA_SIZE) {
                size_t len = MIN((numofbytes - i), PM3_CMD_DATA_SIZE);
                int result = reply_old(CMD_DOWNLOADED_BIGBUF, i, len, BigBuf_get_traceLen(), mem + startidx + i, len);
                if (result != PM3_SUCCESS)
                    Dbprintf("transfer to client failed ::  | bytes between %d - %d (%d) | result: %d", i, i + len, len, result);
            }
            // Trigger a finish downloading signal with an ACK frame
            // iceman,  when did sending samplingconfig array got attached here?!?
            // arg0 = status of download transfer
            // arg1 = RFU
            // arg2 = tracelen?
            // asbytes = samplingconfig array
            reply_old(CMD_ACK, 1, 0, BigBuf_get_traceLen(), getSamplingConfig(), sizeof(sample_config));
            LED_B_OFF();
            break;
        }
        case CMD_DOWNLOAD_EML_BIGBUF: {
            LED_B_ON();
            uint8_t *mem = BigBuf_get_EM_addr();
            uint32_t startidx = packet->oldarg[0];
            uint32_t numofbytes = packet->oldarg[1];

            // arg0 = startindex
            // arg1 = length bytes to transfer
            // arg2 = RFU

            for (size_t i = 0; i < numofbytes; i += PM3_CMD_DATA_SIZE) {
                size_t len = MIN((numofbytes - i), PM3_CMD_DATA_SIZE);
                int result = reply_old(CMD_DOWNLOADED_EML_BIGBUF, i, len, 0, mem + startidx + i, len);
                if (result != PM3_SUCCESS)
                    Dbprintf("transfer to client failed ::  | bytes between %d - %d (%d) | result: %d", i, i + len, len, result);
            }
            // Trigger a finish downloading signal with an ACK frame
            reply_old(CMD_ACK, 1, 0, 0, 0, 0);
            LED_B_OFF();
            break;
        }
        case CMD_READ_MEM: {
            if (packet->length != sizeof(uint32_t))
                break;
            ReadMem(packet->data.asDwords[0]);
            break;
        }
        case CMD_SPIFFS_TEST: {
            test_spiffs();
            break;
        }
        case CMD_SPIFFS_CHECK: {
            rdv40_spiffs_check();
            break;
        }
        case CMD_SPIFFS_MOUNT: {
            rdv40_spiffs_lazy_mount();
            break;
        }
        case CMD_SPIFFS_UNMOUNT: {
            rdv40_spiffs_lazy_unmount();
            break;
        }
        case CMD_SPIFFS_PRINT_TREE: {
            rdv40_spiffs_safe_print_tree(true);
            break;
        }
        case CMD_SPIFFS_PRINT_FSINFO: {
            rdv40_spiffs_safe_print_fsinfo();
            break;
        }
        case CMD_SPIFFS_DOWNLOAD: {
            LED_B_ON();
            uint8_t filename[32];
            uint8_t *pfilename = packet->data.asBytes;
            memcpy(filename, pfilename, SPIFFS_OBJ_NAME_LEN);
            if (DBGLEVEL > 1) Dbprintf("> Filename received for spiffs dump : %s", filename);

            //uint32_t size = 0;
            //rdv40_spiffs_stat((char *)filename, (uint32_t *)size,RDV40_SPIFFS_SAFETY_SAFE);
            uint32_t size = packet->oldarg[1];
            //uint8_t buff[size];

            uint8_t *buff = BigBuf_malloc(size);
            rdv40_spiffs_read_as_filetype((char *)filename, (uint8_t *)buff, size, RDV40_SPIFFS_SAFETY_SAFE);

            // arg0 = filename
            // arg1 = size
            // arg2 = RFU

            for (size_t i = 0; i < size; i += PM3_CMD_DATA_SIZE) {
                size_t len = MIN((size - i), PM3_CMD_DATA_SIZE);
                int result = reply_old(CMD_SPIFFS_DOWNLOADED, i, len, 0, buff + i, len);
                if (result != PM3_SUCCESS)
                    Dbprintf("transfer to client failed ::  | bytes between %d - %d (%d) | result: %d", i, i + len, len, result);
            }
            // Trigger a finish downloading signal with an ACK frame
            reply_old(CMD_ACK, 1, 0, 0, 0, 0);
            LED_B_OFF();
            break;
        }
        case CMD_SPIFFS_STAT: {
            LED_B_ON();
            uint8_t filename[32];
            uint8_t *pfilename = packet->data.asBytes;
            memcpy(filename, pfilename, SPIFFS_OBJ_NAME_LEN);
            if (DBGLEVEL > 1) Dbprintf("> Filename received for spiffs STAT : %s", filename);
            int changed = rdv40_spiffs_lazy_mount();
            uint32_t size = size_in_spiffs((char *)filename);
            if (changed) rdv40_spiffs_lazy_unmount();
            reply_old(CMD_ACK, size, 0, 0, 0, 0);
            LED_B_OFF();
            break;
        }
        case CMD_SPIFFS_REMOVE: {
            LED_B_ON();
            uint8_t filename[32];
            uint8_t *pfilename = packet->data.asBytes;
            memcpy(filename, pfilename, SPIFFS_OBJ_NAME_LEN);
            if (DBGLEVEL > 1) Dbprintf("> Filename received for spiffs REMOVE : %s", filename);
            rdv40_spiffs_remove((char *) filename, RDV40_SPIFFS_SAFETY_SAFE);
            LED_B_OFF();
            break;
        }
        case CMD_SPIFFS_RENAME: {
            LED_B_ON();
            uint8_t srcfilename[32];
            uint8_t destfilename[32];
            uint8_t *pfilename = packet->data.asBytes;
            char *token;
            token = strtok((char *)pfilename, ",");
            strcpy((char *)srcfilename, token);
            token = strtok(NULL, ",");
            strcpy((char *)destfilename, token);
            if (DBGLEVEL > 1) Dbprintf("> Filename received as source for spiffs RENAME : %s", srcfilename);
            if (DBGLEVEL > 1) Dbprintf("> Filename received as destination for spiffs RENAME : %s", destfilename);
            rdv40_spiffs_rename((char *) srcfilename, (char *)destfilename, RDV40_SPIFFS_SAFETY_SAFE);
            LED_B_OFF();
            break;
        }
        case CMD_SPIFFS_COPY: {
            LED_B_ON();
            uint8_t srcfilename[32];
            uint8_t destfilename[32];
            uint8_t *pfilename = packet->data.asBytes;
            char *token;
            token = strtok((char *)pfilename, ",");
            strcpy((char *)srcfilename, token);
            token = strtok(NULL, ",");
            strcpy((char *)destfilename, token);
            if (DBGLEVEL > 1) Dbprintf("> Filename received as source for spiffs COPY : %s", srcfilename);
            if (DBGLEVEL > 1) Dbprintf("> Filename received as destination for spiffs COPY : %s", destfilename);
            rdv40_spiffs_copy((char *) srcfilename, (char *)destfilename, RDV40_SPIFFS_SAFETY_SAFE);
            LED_B_OFF();
            break;
        }
        case CMD_SPIFFS_WRITE: {
            LED_B_ON();
            uint8_t filename[32];
            uint32_t append = packet->oldarg[0];
            uint32_t size = packet->oldarg[1];
            uint8_t *data = packet->data.asBytes;

            //rdv40_spiffs_lazy_mount();

            uint8_t *pfilename = packet->data.asBytes;
            memcpy(filename, pfilename, SPIFFS_OBJ_NAME_LEN);
            data += SPIFFS_OBJ_NAME_LEN;

            if (DBGLEVEL > 1) Dbprintf("> Filename received for spiffs WRITE : %s with APPEND SET TO : %d", filename, append);
            if (!append) {
                rdv40_spiffs_write((char *) filename, (uint8_t *)data, size, RDV40_SPIFFS_SAFETY_SAFE);
            } else {
                rdv40_spiffs_append((char *) filename, (uint8_t *)data, size, RDV40_SPIFFS_SAFETY_SAFE);
            }
            reply_old(CMD_ACK, 1, 0, 0, 0, 0);
            LED_B_OFF();
            break;
        }
        case CMD_FLASHMEM_SET_SPIBAUDRATE: {
            FlashmemSetSpiBaudrate(packet->oldarg[0]);
            break;
        }
        case CMD_FLASHMEM_WRITE: {
            LED_B_ON();
            uint8_t isok = 0;
            uint16_t res = 0;
            uint32_t startidx = packet->oldarg[0];
            uint16_t len = packet->oldarg[1];
            uint8_t *data = packet->data.asBytes;

            if (!FlashInit()) {
                break;
            }

            if (startidx == DEFAULT_T55XX_KEYS_OFFSET) {
                Flash_CheckBusy(BUSY_TIMEOUT);
                Flash_WriteEnable();
                Flash_Erase4k(3, 0xC);
            } else if (startidx ==  DEFAULT_MF_KEYS_OFFSET) {
                Flash_CheckBusy(BUSY_TIMEOUT);
                Flash_WriteEnable();
                Flash_Erase4k(3, 0x9);
                Flash_CheckBusy(BUSY_TIMEOUT);
                Flash_WriteEnable();
                Flash_Erase4k(3, 0xA);
            } else if (startidx == DEFAULT_ICLASS_KEYS_OFFSET) {
                Flash_CheckBusy(BUSY_TIMEOUT);
                Flash_WriteEnable();
                Flash_Erase4k(3, 0xB);
            }

            res = Flash_Write(startidx, data, len);
            isok = (res == len) ? 1 : 0;

            reply_old(CMD_ACK, isok, 0, 0, 0, 0);
            LED_B_OFF();
            break;
        }
        case CMD_FLASHMEM_WIPE: {
            LED_B_ON();
            uint8_t page = packet->oldarg[0];
            uint8_t initalwipe = packet->oldarg[1];
            bool isok = false;
            if (initalwipe) {
                isok = Flash_WipeMemory();
                reply_old(CMD_ACK, isok, 0, 0, 0, 0);
                LED_B_OFF();
                break;
            }
            if (page < 3)
                isok = Flash_WipeMemoryPage(page);

            reply_old(CMD_ACK, isok, 0, 0, 0, 0);
            LED_B_OFF();
            break;
        }
        case CMD_FLASHMEM_DOWNLOAD: {

            LED_B_ON();
            uint8_t *mem = BigBuf_malloc(PM3_CMD_DATA_SIZE);
            uint32_t startidx = packet->oldarg[0];
            uint32_t numofbytes = packet->oldarg[1];
            // arg0 = startindex
            // arg1 = length bytes to transfer
            // arg2 = RFU

            if (!FlashInit()) {
                break;
            }

            for (size_t i = 0; i < numofbytes; i += PM3_CMD_DATA_SIZE) {
                size_t len = MIN((numofbytes - i), PM3_CMD_DATA_SIZE);
                Flash_CheckBusy(BUSY_TIMEOUT);
                bool isok = Flash_ReadDataCont(startidx + i, mem, len);
                if (!isok)
                    Dbprintf("reading flash memory failed ::  | bytes between %d - %d", i, len);

                isok = reply_old(CMD_FLASHMEM_DOWNLOADED, i, len, 0, mem, len);
                if (isok != 0)
                    Dbprintf("transfer to client failed ::  | bytes between %d - %d", i, len);
            }
            FlashStop();

            reply_old(CMD_ACK, 1, 0, 0, 0, 0);
            BigBuf_free();
            LED_B_OFF();
            break;
        }
        case CMD_FLASHMEM_INFO: {

            LED_B_ON();
            rdv40_validation_t *info = (rdv40_validation_t *)BigBuf_malloc(sizeof(rdv40_validation_t));

            bool isok = Flash_ReadData(FLASH_MEM_SIGNATURE_OFFSET, info->signature, FLASH_MEM_SIGNATURE_LEN);

            if (FlashInit()) {
                Flash_UniqueID(info->flashid);
                FlashStop();
            }
            reply_old(CMD_ACK, isok, 0, 0, info, sizeof(rdv40_validation_t));
            BigBuf_free();

            LED_B_OFF();
            break;
        }
        case CMD_VERSION: {
            SendVersion();
            break;
        }
        case CMD_STATUS: {
            SendStatus();
            break;
        }
        case CMD_STANDALONE: {
            RunMod();
            break;
        }
        case CMD_CAPABILITIES: {
            SendCapabilities();
            break;
        }
        case CMD_PING: {
            reply_ng(CMD_PING, PM3_SUCCESS, packet->data.asBytes, packet->length);
            break;
        }
        case CMD_DEVICE_INFO: {
            uint32_t dev_info = DEVICE_INFO_FLAG_OSIMAGE_PRESENT | DEVICE_INFO_FLAG_CURRENT_MODE_OS;
            if (common_area.flags.bootrom_present) {
                dev_info |= DEVICE_INFO_FLAG_BOOTROM_PRESENT;
            }
            reply_old(CMD_DEVICE_INFO, dev_info, 0, 0, 0, 0);
            break;
        }
        default: {
            Dbprintf("%s: 0x%04x", "unknown command:", packet->cmd);
            break;
        }
    }
    return 0;
}
