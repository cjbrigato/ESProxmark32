#include "common/common_firmware.h"
#include <M5StackUpdater.h>

#define tft M5.Lcd // syntax sugar, forward compat with other displays (i.e GO.Lcd)
#define M5_FS SD

const uint8_t extensionsCount = 5; // change this if you add / remove an extension
String allowedExtensions[extensionsCount] = {
    // do NOT remove jpg and json or the menu will crash !!!
    "jpg", "json", "mod", "mp3", "cert"
};

SDUpdater sdUpdater;

void SelfUpdate() {

  Serial.println( DEBUG_SPIFFS_SCAN );
  if( !SPIFFS.begin() ){
    Serial.println( DEBUG_SPIFFS_MOUNTFAILED );
  } else {
    File root = SPIFFS.open( "/" );
    if( !root ){
      Serial.println( DEBUG_DIROPEN_FAILED );
    } else {
      if( !root.isDirectory() ){
        Serial.println( DEBUG_NOTADIR );
      } else {
        File file = root.openNextFile();
        Serial.println( file.name() );
        String fileName = file.name();
        String destName = "";
        if( fileName.endsWith( ".bin" ) ) {
          destName = fileName;
        }
        // move allowed file types to their own folders
        for( uint8_t i=0; i<extensionsCount; i++)  {
          String ext = "." + allowedExtensions[i];
          if( fileName.endsWith( ext ) ) {  
            destName = "/" + allowedExtensions[i] + fileName;
          }
        }
        if( destName!="" ) {
          sdUpdater.displayUpdateUI( String( MOVINGFILE_MESSAGE ) + fileName );
          size_t fileSize = file.size();
          File destFile = M5_FS.open( destName, FILE_WRITE );
          if( !destFile ){
            Serial.println( DEBUG_SPIFFS_WRITEFAILED) ;
          } else {
            static uint8_t buf[512];
            size_t packets = 0;
            Serial.println( String( DEBUG_FILECOPY ) + fileName );
            
            while( file.read( buf, 512) ) {
              destFile.write( buf, 512 );
              packets++;
              sdUpdater.SDMenuProgress( (packets*512)-511, fileSize );
            }
            destFile.close();
            Serial.println();
            Serial.println( DEBUG_FILECOPY_DONE );
            SPIFFS.remove( fileName );

            //we profit the moment for cleaning some relicates
            if( M5_FS.exists( "/ESProxmark32beta" ) ) {
            SPIFFS.remove("/ESProxmark32beta");
            }

            Serial.println( DEBUG_WILL_RESTART );
            delay( 500 );
            // we overWrite ourself into the flash
            sdUpdater.updateFromFS( M5_FS, "/ESProxmark32.bin" );
            ESP.restart();
          }
        } else {
          Serial.println( DEBUG_NOTHING_TODO );
        } // aa
      } // aaaaa
    } // aaaaaaaaa
  } // aaaaaaaaaaaaah!
} // nooooooooooooooes!!