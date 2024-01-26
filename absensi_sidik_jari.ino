#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFiClient.h> 
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_GFX.h>          
#include <Adafruit_SSD1306.h>     
#include <Adafruit_Fingerprint.h>  


#define Finger_Rx D4
#define Finger_Tx D3 

SoftwareSerial mySerial(Finger_Rx, Finger_Tx);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

const char *ssid = "Farhan_Abadi";
const char *password = "farhanabadi2020";

String postData;
String link = "http://192.168.1.11/biometricattendance/getdata.php";
int FingerID = 0;
uint8_t id;

LiquidCrystal_I2C lcd(0x3f, 16, 2);

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  connectToWiFi();
  finger.begin(57600);
  Serial.println("\n\nTes deteksi sidik jari dengan Adafruit");

  if (finger.verifyPassword()) {
    Serial.println("Sensor sidik jari terdeteksi!");
    lcd.clear();
    lcd.print("Sensor Siap");
  } else {
    Serial.println("Tidak dapat menemukan sensor sidik jari :(");
    lcd.clear();
    lcd.print("Not Sensor");
    while (1);
  }

  finger.getTemplateCount();
  Serial.print("Sensor berisi "); Serial.print(finger.templateCount); Serial.println(" template");
  Serial.println("Menunggu sidik jari valid...");
  CariSidikJari();

}

void CariSidikJari(){
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Tempelkan");
  lcd.setCursor(0, 1);
  lcd.print("sidik jari valid...");
}

void loop() {
  if(WiFi.status() != WL_CONNECTED){
    connectToWiFi();
  }

  FingerID = getFingerprintID();  
  delay(50);  
  DisplayFingerprintID();
  ChecktoAddID();
  ChecktoDeleteID();
}

void DisplayFingerprintID(){
  if (FingerID > 0){
    SendFingerprintID( FingerID );
  }
  else if (FingerID == 0){
  
  }
  else if (FingerID == -1){
 
  }
  //---------------------------------------------
  //Didn't find the scanner or there an error
  else if (FingerID == -2){
  
  }
}

void SendFingerprintID(int finger) {
  WiFiClient client;
  HTTPClient http;  
  postData = "FingerID=" + String(finger); 
  http.begin(client, link); 
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");   
  
  int httpCode = http.POST(postData);   
  String payload = http.getString();    
  
  // Serial.println(httpCode);   Code berhasil 200
  // Serial.println(payload);    http 
  // Serial.println(postData);   informasi
  // Serial.println(finger);     code sidik jari

  if (payload.substring(0, 5) == "login") {
    String user_name = payload.substring(5);
     Serial.println(user_name);
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("Welcom");
     lcd.setCursor(0, 1);
     lcd.print(user_name);
     delay(4000);
     CariSidikJari();
  //     for (int i = 0; i < user_name.length() + 16; i++) {
  //     lcd.clear();
  //     lcd.setCursor(0, 0);
  //     lcd.print("Welcome");
  //     String displayText = user_name.substring(i, i + 16);
  //     lcd.setCursor(0, 1);
  //     lcd.print(displayText);
  //     delay(500);
  // }
  }
  else if (payload.substring(0, 6) == "logout") {
    String user_name = payload.substring(6);
    Serial.println(user_name);
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("Good Bye");
     lcd.setCursor(0, 1);
     lcd.print(user_name);
     delay(4000);
     CariSidikJari();
  }
  delay(1000);
  
  postData = "";
  http.end();  
}

int getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      //Serial.println("No finger detected");
      return 0;
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
      return -2;
    case FINGERPRINT_IMAGEFAIL:
      //Serial.println("Imaging error");
      return -2;
    default:
      //Serial.println("Unknown error");
      return -2;
  }
  // OK success!
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      //Serial.println("Image too messy");
      return -1;
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
      return -2;
    case FINGERPRINT_FEATUREFAIL:
      //Serial.println("Could not find fingerprint features");
      return -2;
    case FINGERPRINT_INVALIDIMAGE:
      //Serial.println("Could not find fingerprint features");
      return -2;
    default:
      //Serial.println("Unknown error");
      return -2;
  }
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    // Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    // Serial.println("Communication error");
    return -2;
  } else if (p == FINGERPRINT_NOTFOUND) {
     Serial.println("Did not find a match");
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("Sidik Jari");
     lcd.setCursor(0, 1);
     lcd.print("Tidak Terdaftar");
     delay(3000);
     CariSidikJari();
    return -1;
    
  } else {
    Serial.println("Unknown error");
    return -2;
  }   
  // found a match!
  // Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  // Serial.print(" with confidence of "); Serial.println(finger.confidence); 

  return finger.fingerID;
}


void ChecktoDeleteID() {
  WiFiClient client;
  HTTPClient http;    
  postData = "DeleteID=check"; 

  http.begin(client, link);  
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");    
  
  int httpCode = http.POST(postData);   
  String payload = http.getString();   

  if (payload.substring(0, 6) == "del-id") {
    String del_id = payload.substring(6);
    Serial.println(del_id);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sidik Jari");
    lcd.setCursor(0, 1);
    lcd.print("Terhapus id: " + del_id);
    deleteFingerprint(del_id.toInt());
  }
  
  http.end();  
}

uint8_t deleteFingerprint(int id) {
  uint8_t p = -1;
  
  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.print("Terhapus!\n");
    delay(4000);
    CariSidikJari();
    return p;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Kesalahan komunikasi");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Tidak dapat menghapus di lokasi tersebut");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Kesalahan menulis ke flash");
    return p;
  } else {
    Serial.print("Kesalahan tidak diketahui: 0x"); 
    Serial.println(p, HEX);
    return p;
  }   
}

void ChecktoAddID() {
  WiFiClient client;
  HTTPClient http;    
  postData = "Get_Fingerid=get_id"; 

  http.begin(client, link);  
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");    
  
  int httpCode = http.POST(postData);   
  String payload = http.getString();   

  if (payload.substring(0, 6) == "add-id") {
    String add_id = payload.substring(6);
    Serial.println(add_id);
    id = add_id.toInt();
    getFingerprintEnroll();
  }
  
  http.end();  
}

// Daftra Sidik Jari
uint8_t getFingerprintEnroll() {

  int p = -1;

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sidik diambil");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Mencari....");
      lcd.setCursor(0, 1);
      lcd.print("Sidik Jari...");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Invalid");
      delay(4000);
      CariSidikJari();
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      delay(4000);
      CariSidikJari();
      break;
    default:
      Serial.println("Unknown error");
      delay(4000);
      CariSidikJari();
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Sidik Jari DiTemukan");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Ditemukan");
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Sidik Jari");
      break;
    case FINGERPRINT_IMAGEMESS:
         Serial.println("IMAGEMESS");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Angkat Jari Anda");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ulangi Lagi");
  lcd.setCursor(0, 1);
  lcd.print("Sidik Jari");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sidik Jari Diambil");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Mencari....");
      lcd.setCursor(0, 1);
      lcd.print("Sidik Jari...");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Cetakannya cocok!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
     lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Disimpan! :)");
    confirmAdding();
      delay(3000);
     CariSidikJari();
    return p;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
}

void confirmAdding() {
  WiFiClient client;
  HTTPClient http;    
  postData = "confirm_id=" + String(id); 

  http.begin(client, link); 
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");    
  
  int httpCode = http.POST(postData);   
  String payload = http.getString();   

  delay(1000);
  Serial.println(payload);
  
  http.end();  
}

void connectToWiFi() {
  WiFi.mode(WIFI_OFF);        
  delay(1000);
  WiFi.mode(WIFI_STA);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Menghubungkan ke");
  lcd.setCursor(0, 1);
  lcd.print(ssid);
  Serial.print("Menghubungkan ke ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Terhubung");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Terhubung");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  
  Serial.print("Alamat IP: ");
  Serial.println(WiFi.localIP());
}
