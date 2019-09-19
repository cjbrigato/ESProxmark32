#include "common/common_firmware.h"

#include "libpm3/comms.h"
#include "libpm3/crc16.h"
#include "libpm3/util.h"

//#include <BluetoothSerial.h>
// extern BluetoothSerial Serial2;

communication_arg_t conn;
capabilities_t pm3_capabilities;

static int reply_ng_internal(uint16_t cmd, int16_t status, uint8_t *data,
                             size_t len, bool ng) {
  PacketResponseNGRaw txBufferNG;
  size_t txBufferNGLen;

  // Compose the outgoing command frame
  txBufferNG.pre.magic = RESPONSENG_PREAMBLE_MAGIC;
  txBufferNG.pre.cmd = cmd;
  txBufferNG.pre.status = status;
  txBufferNG.pre.ng = ng;
  if (len > PM3_CMD_DATA_SIZE) {
    len = PM3_CMD_DATA_SIZE;
    // overwrite status
    txBufferNG.pre.status = PM3_EOVFLOW;
  }
  txBufferNG.pre.length = len;

  // Add the (optional) content to the frame, with a maximum size of
  // PM3_CMD_DATA_SIZE
  if (data && len) {
    for (size_t i = 0; i < len; i++) {
      txBufferNG.data[i] = data[i];
    }
  }

  PacketResponseNGPostamble *tx_post =
      (PacketResponseNGPostamble *)((uint8_t *)&txBufferNG +
                                    sizeof(PacketResponseNGPreamble) + len);
  // Note: if we send to both FPC & USB, we'll set CRC for both if any of them
  // require CRC
  // if ((reply_via_fpc && reply_with_crc_on_fpc) || ((reply_via_usb) &&
  // reply_with_crc_on_usb)) {
  uint8_t first, second;
  compute_crc(CRC_14443_A, (uint8_t *)&txBufferNG,
              sizeof(PacketResponseNGPreamble) + len, &first, &second);
  tx_post->crc = (first << 8) + second;
  //} else {
  //    tx_post->crc = RESPONSENG_POSTAMBLE_MAGIC;
  //}
  txBufferNGLen = sizeof(PacketResponseNGPreamble) + len +
                  sizeof(PacketResponseNGPostamble);

  int resultfpc = PM3_EUNDEF;
  int resultusb = PM3_EUNDEF;
  // Send frame and make sure all bytes are transmitted

  resultusb = Serial2.write((uint8_t *)&txBufferNG, txBufferNGLen);

  return PM3_SUCCESS;
}

int reply_ng(uint16_t cmd, int16_t status, uint8_t *data, size_t len) {
  return reply_ng_internal(cmd, status, data, len, true);
}

static void SendCommandNG_internal(uint16_t cmd, uint8_t *data, size_t len,
                                   bool ng) {
  /*
    if (!session.pm3_present) {
        PrintAndLogEx(NORMAL, "Sending bytes to proxmark failed - offline");
        return;
    }
*/

  PacketCommandNGRaw txBufferNG;
  size_t txBufferNGLen;
  if (len > PM3_CMD_DATA_SIZE) {
    aprintf("(WARNING Sending %d bytes of payload is too much, abort", len);
    return;
  }

#ifdef DEBUG_SENDNG
  uint8_t *ptrPacketStarting = (uint8_t *)&txBufferNG;
  aprintf("PacketStarting Status : ");
  for (int i = 0; i < txBufferNGLen; i++) {
    aprintf("d%d", ptrPacketStarting[i]);
  }
  aprintf("\r\n");
#endif

  txBufferNG.pre.magic = COMMANDNG_PREAMBLE_MAGIC;
  txBufferNG.pre.length = len;
  txBufferNG.pre.cmd = cmd;

#ifdef DEBUG_SENDNG
  aprintf("cmd : d%d \r\n", cmd);
  aprintf("txBufferNG.pre.cmd : d%d \r\n", txBufferNG.pre.cmd);
  uint8_t *ptrPacketDoing = (uint8_t *)&txBufferNG;
  aprintf("PacketDoing Status : ");
  for (int i = 0; i < txBufferNGLen; i++) {
    aprintf("d%d", ptrPacketDoing[i]);
  }
  aprintf("\r\n");
#endif

  txBufferNG.pre.ng = ng;
  if (len > 0 && data) memcpy(&txBufferNG.data, data, len);

#ifdef DEBUG_SENDNG
  uint8_t *ptrPacket0 = (uint8_t *)&txBufferNG;
  aprintf("Packet0 Status : ");
  for (int i = 0; i < txBufferNGLen; i++) {
    aprintf("d%d", ptrPacket0[i]);
  }
  aprintf("\r\n");
#endif

  PacketCommandNGPostamble *tx_post =
      (PacketCommandNGPostamble *)((uint8_t *)&txBufferNG +
                                   sizeof(PacketCommandNGPreamble) + len);

#ifdef DEBUG_SENDNG
  uint8_t *ptrPacket1 = (uint8_t *)&txBufferNG;
  aprintf("Packet1 Status : ");
  for (int i = 0; i < txBufferNGLen; i++) {
    aprintf("d%d", ptrPacket1[i]);
  }
  aprintf("\r\n");
#endif

  // if ((conn.send_via_fpc_usart && conn.send_with_crc_on_fpc) ||
  // ((!conn.send_via_fpc_usart) && conn.send_with_crc_on_usb)) {
  uint8_t first, second;
  compute_crc(CRC_14443_A, (uint8_t *)&txBufferNG,
              sizeof(PacketCommandNGPreamble) + len, &first, &second);
  tx_post->crc = (first << 8) + second;
  // } else {
  //       tx_post->crc = COMMANDNG_POSTAMBLE_MAGIC;
  // }

  txBufferNGLen =
      sizeof(PacketCommandNGPreamble) + len + sizeof(PacketCommandNGPostamble);

#ifdef DEBUG_SENDNG
  uint8_t *ptrPacket = (uint8_t *)&txBufferNG;
  aprintf("Packet to send : ");
  for (int i = 0; i < txBufferNGLen; i++) {
    aprintf("d%d", ptrPacket[i]);
  }
  aprintf("\r\n");
#endif

  // aprintf("-\nTo transmitlen : %d\n", txBufferNGLen);
  // aprintf("Avaialable for write : %d\n", Serial2.availableForWrite());

  Serial2.write((uint8_t *)&txBufferNG, txBufferNGLen);
}

void SendCommandNG(uint16_t cmd, uint8_t *data, size_t len) {
  conn.last_command = cmd;
  SendCommandNG_internal(cmd, data, len, true);
}

static void PacketResponseReceived(PacketResponseNG *packet) {
  switch (packet->cmd) {
    // First check if we are handling a debug message
    case CMD_DEBUG_PRINT_STRING: {
      char s[PM3_CMD_DATA_SIZE + 1];
      memset(s, 0x00, sizeof(s));

      size_t len;
      uint16_t flag;
      if (packet->ng) {
        struct d {
          uint16_t flag;
          uint8_t buf[PM3_CMD_DATA_SIZE - sizeof(uint16_t)];
        } PACKED;
        struct d *data = (struct d *)&packet->data.asBytes;
        len = packet->length - sizeof(data->flag);
        flag = data->flag;
        memcpy(s, data->buf, len);
      } else {
        len = MIN(packet->oldarg[0], PM3_CMD_DATA_SIZE);
        flag = packet->oldarg[1];
        memcpy(s, packet->data.asBytes, len);
      }

      if (flag & FLAG_LOG) {
        printString(s);
      } else {
        if (flag & FLAG_INPLACE) printString("\r");
        printString(s);

        if (flag & FLAG_NEWLINE) printString("\r\n");
      }

      // fflush(stdout);
      break;
    }
    case CMD_DEBUG_PRINT_INTEGERS: {
      // PrintAndLogEx(NORMAL, "#db# %" PRIx64 ", %" PRIx64 ", %" PRIx64 "",
      // packet->oldarg[0], packet->oldarg[1], packet->oldarg[2]);
      break;
    }
    // iceman:  hw status - down the path on device, runs printusbspeed which
    // starts sending a lot of CMD_DOWNLOAD_BIGBUF packages which is not dealt
    // with. I wonder if simply ignoring them will work. lets try it.
    default: {
      // storeReply(packet);
      break;
    }
  }
}

static unsigned long communication_delay(void) {
  return (2 * (12000000 / 115200));
}

PacketResponseNG receive_pm3cmd(void) {
  static uint32_t pacount = 0;
  uint32_t rxlen;
  bool commfailed = false;
  PacketResponseNG rx;
  PacketResponseNGRaw rx_raw;

  rxlen = 0;
  bool ACK_received = false;
  bool error = false;
  int res = 0;

  Serial2.setTimeout(1000);

  if (Serial2.available() <= sizeof(PacketResponseNGPreamble)) {
    return rx;
  }

  rxlen = Serial2.readBytes((uint8_t *)&rx_raw.pre,
                            sizeof(PacketResponseNGPreamble));

  Serial.printf("----NEW PACKET START----\r\n");

  if ((rxlen == sizeof(PacketResponseNGPreamble))) {
    Serial.printf("Debug: Seems long enough for NG frame (preamble)\r\n");
    rx.magic = rx_raw.pre.magic;
    uint16_t length = rx_raw.pre.length;
    rx.ng = rx_raw.pre.ng;
    rx.status = rx_raw.pre.status;
    rx.cmd = rx_raw.pre.cmd;
    if (rx.magic == RESPONSENG_PREAMBLE_MAGIC) {
      Serial.printf("Debug: It got the RESPONSENG MAGIC PREAMBLE :)\r\n");
      if (length > PM3_CMD_DATA_SIZE) {
        aprintf(
            "WARNING Received packet frame with incompatible length: "
            "0x%04x\n\n",
            length);
        error = true;
      }
      if ((!error) && (length > 0)) {  // Get the variable length payload

        rxlen = Serial2.readBytes((uint8_t *)&rx_raw.data, length);
        Serial.printf("rxlen :%d of %d\r\n", rxlen, length);

        if ((rxlen != length)) {
          aprintf(
              "WARNING, Received packet frame with variable part too short? "
              "%d/%d\r\n",
              rxlen, length);
          error = true;
        } else {
          if (rx.ng) {  // Received a valid NG frame
            Serial.printf("Debug:packet is NG flagged\r\n");

            memcpy(&rx.data, &rx_raw.data, length);
            rx.length = length;
            if ((rx.cmd == conn.last_command) && (rx.status == PM3_SUCCESS)) {
              ACK_received = true;
            }
          } else {
            Serial.printf("Debug:packet is NOT ng flagged (mix ?)\r/n");
            uint64_t arg[3];
            if (length < sizeof(arg)) {
              aprintf(
                  "WARNING, Received MIX packet frame with incompatible "
                  "length: 0x%04x\r\n",
                  length);
              error = true;
            }
            if (!error) {  // Received a valid MIX frame
              Serial.printf("Debug : We consider it valid MIX FRAME\r\n");
              memcpy(arg, &rx_raw.data, sizeof(arg));
              rx.oldarg[0] = arg[0];
              rx.oldarg[1] = arg[1];
              rx.oldarg[2] = arg[2];
              memcpy(&rx.data, ((uint8_t *)&rx_raw.data) + sizeof(arg),
                     length - sizeof(arg));
              rx.length = length - sizeof(arg);
              if (rx.cmd == CMD_ACK) {
                ACK_received = true;
              }
            }
          }
        }
      }
      if (!error) {  // Get the postamble
        Serial.printf(
            "Debug: We consider it near valid NG frame\r\n   --> Get the "
            "postamble\r\n");
        rxlen = Serial2.readBytes((uint8_t *)&rx_raw.foopost,
                                  sizeof(PacketResponseNGPostamble));
        Serial.printf("   --> (foopost) rxlen :%d of %d\r\n", rxlen,
                      sizeof(PacketResponseNGPostamble));
        if ((rxlen != sizeof(PacketResponseNGPostamble))) {
          aprintf("WARN Received packet frame without postamble\r\n");
          // printString("--- :( Received packet frame without postamble");
          error = true;
        }
      }
      if (!error) {  // Check CRC, accept MAGIC as placeholder
        Serial.printf(
            "Debug: We consider postamble size-correct so we'll compute "
            "CRC\r\n   (if a NG_MAGIC is not provided)\r\n");
        rx.crc = rx_raw.foopost.crc;
        if (rx.crc != RESPONSENG_POSTAMBLE_MAGIC) {
          Serial.printf("Debug: a Magic was NOT provided. Checkink crc\r\n");
          uint8_t first, second;
          compute_crc(CRC_14443_A, (uint8_t *)&rx_raw,
                      sizeof(PacketResponseNGPreamble) + length, &first,
                      &second);
          if ((first << 8) + second != rx.crc) {
            aprintf("WARNING Received packet frame with invalid CRC\r\n");
            Serial.printf(
                "Debug : rx.crc = %04X <> computed = %02X%02X <> NGMAGIC = "
                "%04X\r\n",
                rx.crc, first, second, RESPONSENG_POSTAMBLE_MAGIC);
            uint8_t *ptrPacket = (uint8_t *)&rx_raw;
            Serial.printf("Debug: Packet was : ");
            for (int i = 0; i < sizeof(rx_raw); i++) {
              Serial.printf("%02X:", ptrPacket[i]);
            }
            Serial.printf("\r\n");
            error = true;
          }
        }
      }
      if (!error) {
        Serial.printf(
            "Debug: We consider the NG or MIX frame was correct so pass it for "
            "treatment\r\n");
        PacketResponseReceived(&rx);
        Serial.printf(
            "Debug: Were done with this MIX OR NG packet "
            "!\r\n------------PACKET-END----------\r\n");
      }
    } else {
      Serial.printf("Debug: Old Style reply received\r\n");
      PacketResponseOLD rx_old;
      memcpy(&rx_old, &rx_raw.pre, sizeof(PacketResponseNGPreamble));
 /*     uint8_t *ptrPacket = (uint8_t *)&rx_old;
            Serial.printf("Debug: Packet was : ");
            for (int i = 0; i < sizeof(PacketResponseNGPreamble) + rxlen; i++) {
              Serial.printf("%02X:", ptrPacket[i]);
            }
            Serial.printf("\r\n");
      while (!Serial2.available()) {
         delay(100);
         Serial.printf("Debug: delaying receive...\r\n");
        }*/
      rxlen = Serial2.readBytes(
          ((uint8_t *)&rx_old) + sizeof(PacketResponseNGPreamble),
          sizeof(PacketResponseOLD) - sizeof(PacketResponseNGPreamble));
          Serial.printf("rxlen :%d of %d\r\n", rxlen, sizeof(PacketResponseOLD) - sizeof(PacketResponseNGPreamble));
      if (rxlen !=
          sizeof(PacketResponseOLD) - sizeof(PacketResponseNGPreamble)) {
        aprintf(
            "WARNING Received packet OLD frame with payload too short? "
            "%d/%d\r\n",
            rxlen,
            sizeof(PacketResponseOLD) - sizeof(PacketResponseNGPreamble));
            
        error = true;
        aprintf("error status: %d\n", error);
      }
      if (!error) {
        rx.ng = false;
        rx.magic = 0;
        rx.status = 0;
        rx.crc = 0;
        rx.cmd = rx_old.cmd;
        rx.oldarg[0] = rx_old.arg[0];
        rx.oldarg[1] = rx_old.arg[1];
        rx.oldarg[2] = rx_old.arg[2];
        rx.length = PM3_CMD_DATA_SIZE;
        memcpy(&rx.data, &rx_old.d, rx.length);
        PacketResponseReceived(&rx);
        Serial.printf(
            "Debug: Were done with this OLD packet "
            "!\r\n------------PACKET-END----------\r\n");
        if (rx.cmd == CMD_ACK) {
          ACK_received = true;
          Serial.printf("Debug: cmd was ACK BTW\r\n");
        }
      } else {
        if (rxlen > 0) {
          aprintf("WARNING Received packet frame preamble too short: %d/%d",
                  rxlen, sizeof(PacketResponseNGPreamble));
          error = true;
        }
        error = true;
      }
    }
      if (error) {
        Serial.printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
        Serial.printf("ERROR : We're going to FLUSH\r\n");
        Serial.printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
        while (Serial2.available()) {
          Serial2.read();
        }
      }
    }
    PacketResponseNG dummyrx;
    return dummyrx;
  }

  /*-------------------This part seems optional at this moment and in this
   * context*/
  /* Still we keep it for later FreeRTOS VTask implementation of this command

  // TODO if error, shall we resync ?

  pthread_mutex_lock(&txBufferMutex);

  if (connection->block_after_ACK)
  {
   // if we just received an ACK, wait here until a new command is to be
  transmitted
   // This is only working on OLD frames, and only used by flasher and flashmem
   if (ACK_received)
   {

     while (!txBuffer_pending)
     {
       pthread_cond_wait(&txBufferSig, &txBufferMutex);
     }
   }
  }

  if (txBuffer_pending)
  {

   if (txBufferNGLen)
   { // NG packet
     res = uart_send(sp, (uint8_t *)&txBufferNG, txBufferNGLen);
     if (res == PM3_EIO)
     {
       commfailed = true;
     }
     conn.last_command = txBufferNG.pre.cmd;
     txBufferNGLen = 0;
   }
   else
   {
     res = uart_send(sp, (uint8_t *)&txBuffer, sizeof(PacketCommandOLD));
     if (res == PM3_EIO)
     {
       commfailed = true;
     }
     conn.last_command = txBuffer.cmd;
   }

   txBuffer_pending = false;

   // main thread doesn't know send failed...

   // tell main thread that txBuffer is empty
   pthread_cond_signal(&txBufferSig);
  }

  pthread_mutex_unlock(&txBufferMutex);
  }

  // when thread dies, we close the serial port.
  uart_close(sp);
  sp = NULL;

  pthread_exit(NULL);
  return NULL;*/