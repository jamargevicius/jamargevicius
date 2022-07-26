/*  testSDcardTroubleshoot.ino
  
  by Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-microsd-card-arduino/
  
  This sketch was mofidied from: Examples > SD(esp32) > SD_Test
  .. further modified by JoeM to redo tests in loop()
*/

#include "FS.h"
#include "SD.h"
#include "SPI.h"

//****** VSPI pins on 30 or 38 pin modules
#define SCK  18
#define MISO  19
#define MOSI  23
#define CS  5
//*/

/****** HSPI pins on 30 pin modulule
#define SCK  14
#define MISO  12
#define MOSI  13
#define CS  15
*/

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);
    File root = fs.open(dirname);
    if(!root){Serial.println("Failed to open directory"); return; }
    if(!root.isDirectory()){Serial.println("Not a directory"); return; }
    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);  // had to correct this.  Was file.path(), but .path not supported
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path){
  Serial.printf("Creating Dir: %s\n", path);
  if(fs.mkdir(path)){Serial.println("Dir created");} 
  else {Serial.println("mkdir failed");}
}

void removeDir(fs::FS &fs, const char * path){
  Serial.printf("Removing Dir: %s\n", path);
  if(fs.rmdir(path)){Serial.println("Dir removed");} 
  else {Serial.println("rmdir failed");}
}

void readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\n", path);
  File file = fs.open(path);
  if(!file){Serial.println("Failed to open file for reading");return;}
  Serial.print("Read from file: ");
  while(file.available()){Serial.write(file.read());}
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\n", path);
  File file = fs.open(path, FILE_WRITE);
  if(!file){Serial.println("Failed to open file for writing");return;}
  if(file.print(message)){Serial.println("File written");} 
  else {Serial.println("Write failed");}
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\n", path);
  File file = fs.open(path, FILE_APPEND);
  if(!file){Serial.println("Failed to open file for appending");return;}
  if(file.print(message)){Serial.println("Message appended");} 
  else {Serial.println("Append failed");}
  file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {Serial.println("File renamed");} 
  else {Serial.println("Rename failed");}
}

void deleteFile(fs::FS &fs, const char * path){
  Serial.printf("Deleting file: %s\n", path);
  if(fs.remove(path)){Serial.println("File deleted");} 
  else {Serial.println("Delete failed");}
}

void testFileIO(fs::FS &fs, const char * path){
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if(file){
    len = file.size();
    size_t flen = len;
    start = millis();
    while(len){
      size_t toRead = len;
      if(toRead > 512){toRead = 512;}
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    Serial.printf("%u bytes read for %u ms\n", flen, end);
    file.close();
  } 
  else {Serial.println("Failed to open file for reading");}

  file = fs.open(path, FILE_WRITE);
  if(!file){Serial.println("Failed to open file for writing"); return;}

  size_t i;
  start = millis();
  for(i=0; i<2048; i++){file.write(buf, 512);}
  end = millis() - start;
  Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
  file.close();
}

void setup(){
  Serial.begin(115200);
  delay(5000); // wait 5 seconds for terminal to come up
  
  SPIClass spi = SPIClass(VSPI);
  spi.begin(SCK, MISO, MOSI, CS);

  if (!SD.begin(CS,spi,80000000)) {Serial.println("Card Mount Failed"); return;}
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE){Serial.println("No SD card attached"); return;}
  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){Serial.println("MMC");} 
  else if(cardType == CARD_SD){Serial.println("SDSC");} 
  else if(cardType == CARD_SDHC){Serial.println("SDHC");} 
  else {Serial.println("UNKNOWN");}

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  listDir(SD, "/", 0);
  createDir(SD, "/mydir");
  listDir(SD, "/", 0);
  removeDir(SD, "/mydir");
  listDir(SD, "/", 2);
  writeFile(SD, "/hello.txt", "Hello ");
  appendFile(SD, "/hello.txt", "World!\n");
  readFile(SD, "/hello.txt");
  deleteFile(SD, "/foo.txt");
  renameFile(SD, "/hello.txt", "/foo.txt");
  readFile(SD, "/foo.txt");
  testFileIO(SD, "/test.txt");
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
}

void loop(){
  delay(2000); // wait 2 seconds
  //SPIClass spi = SPIClass(VSPI);
  //spi.begin(SCK, MISO, MOSI, CS);

  Serial.println("\n\n ********  Here we go for another run **********\n");
  listDir(SD, "/", 0);
  createDir(SD, "/mydir");
  listDir(SD, "/", 0);
  removeDir(SD, "/mydir");
  listDir(SD, "/", 2);
  writeFile(SD, "/hello.txt", "Hello ");
  appendFile(SD, "/hello.txt", "World!\n");
  readFile(SD, "/hello.txt");
  deleteFile(SD, "/foo.txt");
  renameFile(SD, "/hello.txt", "/foo.txt");
  readFile(SD, "/foo.txt");
  testFileIO(SD, "/test.txt");
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
}

/***************** output
 *  SD Card Type: SDHC
SD Card Size: 7580MB
Listing directory: /
  DIR : /System Volume Information
  FILE: /data.txt  SIZE: 3087
  FILE: /test.txt  SIZE: 1048576
  FILE: /foo.txt  SIZE: 13
Creating Dir: /mydir
Dir created
Listing directory: /
  DIR : /System Volume Information
  FILE: /data.txt  SIZE: 3087
  FILE: /test.txt  SIZE: 1048576
  FILE: /foo.txt  SIZE: 13
  DIR : /mydir
Removing Dir: /mydir
Dir removed
Listing directory: /
  DIR : /System Volume Information
Listing directory: /System Volume Information
  FILE: /System Volume Information/IndexerVolumeGuid  SIZE: 76
  FILE: /System Volume Information/WPSettings.dat  SIZE: 12
  FILE: /data.txt  SIZE: 3087
  FILE: /test.txt  SIZE: 1048576
  FILE: /foo.txt  SIZE: 13
Writing file: /hello.txt
File written
Appending to file: /hello.txt
Message appended
Reading file: /hello.txt
Read from file: Hello World!
Deleting file: /foo.txt
File deleted
Renaming file /hello.txt to /foo.txt
File renamed
Reading file: /foo.txt
Read from file: Hello World!
1048576 bytes read for 1311 ms
1048576 bytes written for 4668 ms
Total space: 7572MB
Used space: 1MB

 ********  Here we go for another run **********

Listing directory: /
  DIR : /System Volume Information
  FILE: /data.txt  SIZE: 3087
  FILE: /test.txt  SIZE: 1048576
  FILE: /foo.txt  SIZE: 13
Creating Dir: /mydir
Dir created
Listing directory: /
  DIR : /System Volume Information
   ....etc

***************/
