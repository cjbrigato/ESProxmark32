#ifndef __I18N_H
#define __I18N_H


#define WELCOME_MESSAGE F("Welcome to the M5Stack SD Menu Loader!")
#define INIT_MESSAGE F("M5Stack SD Updater initializing...")
#define M5_SAM_MENU_SETTINGS "M5StackSam loaded with %d labels per page, max %d items\n"
#define SD_LOADING_MESSAGE F("Checking SD Card...")
#define INSERTSD_MESSAGE F("Insert SD")
#define GOTOSLEEP_MESSAGE F("Will go to sleep")
#define MOVINGFILE_MESSAGE F("Moving ")
#define FILESIZE_UNITS F(" bytes")

#define MENU_TITLE F("SD CARD LOADER")
#define MENU_SUBTITLE F("Applications")
#define MENU_BTN_INFO F("SELECT")
#define MENU_BTN_SET F("SET")
#define MENU_BTN_LOAD F("LOAD")
#define MENU_BTN_LAUNCH F("LAUNCH")
#define MENU_BTN_PAGE F(">>")
#define MENU_BTN_NEXT F(">")

#define ABOUT_THIS_MENU F("About This Launcher")

#define AUTHOR_PREFIX F("By ")
#define AUTHOR_SUFFIX F(" **")

#define APP_DOWNLOADER_MENUTITLE "M5Stack Apps Downloader"

#define DOWNLOADER_MODAL_NAME "Update binaries ?"
#define DOWNLOADER_MODAL_TITLE "This action will:"
#define DOWNLOADER_MODAL_BODY "  - Connect to WiFi\n\n  - Get app list from remote registry\n\n  - Download/overwrite files\n\n  - Restart this menu\n\n\n\n  THIS OPERATION IS INSECURE!!\n\n  YOU DO THIS AT YOUR OWN RISK!!"
#define DOWNLOADER_MODAL_ENDED "Synchronization complete"
#define DOWNLOADER_MODAL_TITLE_ERRORS_OCCURED "Some errors occured. "
#define DOWNLOADER_MODAL_BODY_ERRORS_OCCURED "  %d errors occured during the download\n\n  %d files were verified\n\n  %d files were updated\n\n  %d files were created\n\n\n\n  Reboot M5Stack?\n\n"
#define DOWNLOADER_MODAL_REBOOT "REBOOT"
#define DOWNLOADER_MODAL_RETRY "RETRY"
#define DOWNLOADER_MODAL_BACK "BACK"

#define DOWNLOADER_MODAL_YES "YES"
#define DOWNLOADER_MODAL_NO "NO"
#define DOWNLOADER_MODAL_CANCEL "CANCEL"

#define OVERALL_PROGRESS_TITLE "Overall progress: "
#define WGET_SKIPPING " [Checksum OK]"
#define WGET_UPDATING " [Outdated]"
#define WGET_CREATING " [New file]"
#define SYNC_FINISHED "Synch finished"
#define CLEANDIR_REMOVED "Removed %s\n"

#define WIFI_MSG_WAITING "Waiting for WiFi to connect"
#define WIFI_MSG_CONNECTING "Establishing connection to WiFi.."
#define WIFI_MSG_TIMEOUT "Timed out, will try again"
#define WIFI_MSG_CONNECTED "Connected to wifi :-)"

#define DEBUG_DIRNAME F("Listing directory: %s\n")
#define DEBUG_DIROPEN_FAILED F("Failed to open directory")
#define DEBUG_NOTADIR F("Not a directory")
#define DEBUG_DIRLABEL F("  DIR : ")
#define DEBUG_IGNORED F("  IGNORED FILE: ")
#define DEBUG_ABORTLISTING F("  ***Max files reached for M5StackSam Menu, please adjust M5SAM_LIST_MAX_COUNT for more (maximum is 255, sorry :-)")
#define DEBUG_FILELABEL F("  FILE: ")

#define DEBUG_SPIFFS_SCAN F("Scanning SPIFFS for binaries")
#define DEBUG_SPIFFS_MOUNTFAILED F("SPIFFS Mount Failed")
#define DEBUG_SPIFFS_WRITEFAILED F("- failed to open file for writing")
#define DEBUG_FILECOPY F("Starting File Copy for ")
#define DEBUG_FILECOPY_DONE F("Transfer finished")
#define DEBUG_WILL_RESTART F("Binary removed from SPIFFS, will now restart")
#define DEBUG_NOTHING_TODO F("No binary to transfer")
#define DEBUG_KEYPAD_NOTFOUND F("Keypad not installed")
#define DEBUG_KEYPAD_FOUND F("Keypad detected!")
#define DEBUG_JOYPAD_NOTFOUND F("No Joypad detected, disabling")
#define DEBUG_JOYPAD_FOUND F("Joypad detected!")



#endif