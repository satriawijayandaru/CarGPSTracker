#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <WiFi.h>
#include "FS.h"
#include "SD_MMC.h"

int showDebug = 1;

//WIFI CONFIGURATION
const char* ssid     = "XDA";
const char* password = "namaguee";

//GPS CONFIGURATION
static const uint32_t GPSBaud = 9600;
// The TinyGPS++ object
TinyGPSPlus gps;
SoftwareSerial ss(14, 15);      //USING SOFTWARE SERIAL
//#define ss Serial2            //USING HARDWARE SERIAL (PREFERRED IF POSSIBLE)

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
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

void createDir(fs::FS &fs, const char * path) {
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path)) {
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char * path) {
  Serial.printf("Removing Dir: %s\n", path);
  if (fs.rmdir(path)) {
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

void readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
}

void renameFile(fs::FS &fs, const char * path1, const char * path2) {
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char * path) {
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void testFileIO(fs::FS &fs, const char * path) {
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if (file) {
    len = file.size();
    size_t flen = len;
    start = millis();
    while (len) {
      size_t toRead = len;
      if (toRead > 512) {
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    Serial.printf("%u bytes read for %u ms\n", flen, end);
    file.close();
  } else {
    Serial.println("Failed to open file for reading");
  }


  file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  size_t i;
  start = millis();
  for (i = 0; i < 2048; i++) {
    file.write(buf, 512);
  }
  end = millis() - start;
  Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
  file.close();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }


  if (!SD_MMC.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD_MMC.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD_MMC card attached");
    return;
  }

  Serial.print("SD_MMC Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);

  listDir(SD_MMC, "/", 0);
  createDir(SD_MMC, "/GPS_LOG");
  listDir(SD_MMC, "/", 2);
  writeFile(SD_MMC, "/GPS_LOG.txt", "GPS ");
  appendFile(SD_MMC, "/GPS_LOG.txt", "Start!\n");
  readFile(SD_MMC, "/GPS_LOG.txt");
  //    deleteFile(SD_MMC, "/foo.txt");
  //    renameFile(SD_MMC, "/hello.txt", "/foo.txt");
  //    readFile(SD_MMC, "/foo.txt");
  //    testFileIO(SD_MMC, "/test.txt");
  Serial.printf("Total space: %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024));

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Connecting to GPS Module");
  Serial.print("Baud Rate = ");
  Serial.println(GPSBaud);
  ss.begin(GPSBaud);
  //  changeFrequency();
  //  delay(100);
  //  ss.flush();
}

void loop() {
  WiFiClient client;
  GPSgetData();
  //  Serial.println(millis());
}

void GPSgetData() {
  // This sketch displays information every time a new sentence is correctly encoded.
  while (ss.available() > 0) {
    gps.encode(ss.read());
    if (gps.location.isUpdated()) {
      if (showDebug == 1) {
        String test = "test";
        char Buf[20];
        test.toCharArray(Buf, 20);
        writeFile(SD_MMC, "/GPS_LOG.txt", "GPS ");
        appendFile(SD_MMC, "/GPS_LOG.txt", Buf);
        // Latitude in degrees (double)
        Serial.print("Latitude= ");
        Serial.print(gps.location.lat(), 6);
        // Longitude in degrees (double)
        Serial.print(" Longitude= ");
        Serial.println(gps.location.lng(), 6);

        // Raw latitude in whole degrees
        Serial.print("Raw latitude = ");
        Serial.print(gps.location.rawLat().negative ? "-" : "+");
        Serial.println(gps.location.rawLat().deg);
        // ... and billionths (u16/u32)
        Serial.println(gps.location.rawLat().billionths);

        // Raw longitude in whole degrees
        Serial.print("Raw longitude = ");
        Serial.print(gps.location.rawLng().negative ? "-" : "+");
        Serial.println(gps.location.rawLng().deg);
        // ... and billionths (u16/u32)
        Serial.println(gps.location.rawLng().billionths);

        // Raw date in DDMMYY format (u32)
        Serial.print("Raw date DDMMYY = ");
        Serial.println(gps.date.value());

        // Year (2000+) (u16)
        Serial.print("Year = ");
        Serial.println(gps.date.year());
        // Month (1-12) (u8)
        Serial.print("Month = ");
        Serial.println(gps.date.month());
        // Day (1-31) (u8)
        Serial.print("Day = ");
        Serial.println(gps.date.day());

        // Raw time in HHMMSSCC format (u32)
        Serial.print("Raw time in HHMMSSCC = ");
        Serial.println(gps.time.value());

        // Hour (0-23) (u8)
        Serial.print("Hour = ");
        Serial.println(gps.time.hour());
        // Minute (0-59) (u8)
        Serial.print("Minute = ");
        Serial.println(gps.time.minute());
        // Second (0-59) (u8)
        Serial.print("Second = ");
        Serial.println(gps.time.second());
        // 100ths of a second (0-99) (u8)
        Serial.print("Centisecond = ");
        Serial.println(gps.time.centisecond());

        // Raw speed in 100ths of a knot (i32)
        Serial.print("Raw speed in 100ths/knot = ");
        Serial.println(gps.speed.value());
        // Speed in knots (double)
        Serial.print("Speed in knots/h = ");
        Serial.println(gps.speed.knots());
        // Speed in miles per hour (double)
        Serial.print("Speed in miles/h = ");
        Serial.println(gps.speed.mph());
        // Speed in meters per second (double)
        Serial.print("Speed in m/s = ");
        Serial.println(gps.speed.mps());
        // Speed in kilometers per hour (double)
        Serial.print("Speed in km/h = ");
        Serial.println(gps.speed.kmph());

        // Raw course in 100ths of a degree (i32)
        Serial.print("Raw course in degrees = ");
        Serial.println(gps.course.value());
        // Course in degrees (double)
        Serial.print("Course in degrees = ");
        Serial.println(gps.course.deg());

        // Raw altitude in centimeters (i32)
        Serial.print("Raw altitude in centimeters = ");
        Serial.println(gps.altitude.value());
        // Altitude in meters (double)
        Serial.print("Altitude in meters = ");
        Serial.println(gps.altitude.meters());
        // Altitude in miles (double)
        Serial.print("Altitude in miles = ");
        Serial.println(gps.altitude.miles());
        // Altitude in kilometers (double)
        Serial.print("Altitude in kilometers = ");
        Serial.println(gps.altitude.kilometers());
        // Altitude in feet (double)
        Serial.print("Altitude in feet = ");
        Serial.println(gps.altitude.feet());

        // Number of satellites in use (u32)
        Serial.print("Number os satellites in use = ");
        Serial.println(gps.satellites.value());

        // Horizontal Dim. of Precision (100ths-i32)
        Serial.print("HDOP = ");
        Serial.println(gps.hdop.value());
        Serial.println();
      }
    }
  }
}

void dataFormatting() {
  // Latitude in degrees (double)
//  Serial.print(gps.location.lat(), 6);
  // Longitude in degrees (double)
//  Serial.println(gps.location.lng(), 6);
}

void sendPacket(byte *packet, byte len) {
  for (byte i = 0; i < len; i++)
  {
    ss.write(packet[i]); // GPS is HardwareSerial
  }
}

void changeFrequency() {
  byte packet[] = {
    0xB5, //
    0x62, //
    0x06, //
    0x08, //
    0x06, // length
    0x00, //
    0x64, // measRate, hex 64 = dec 100 ms
    0x00, //
    0x01, // navRate, always =1
    0x00, //
    0x01, // timeRef, stick to GPS time (=1)
    0x00, //
    0x7A, // CK_A
    0x12, // CK_B
  };
  sendPacket(packet, sizeof(packet));
}
