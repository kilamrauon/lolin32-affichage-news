//#include <M5StickC.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <Wire.h>
//#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiManager.h>
#include <esp_deep_sleep.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define LOGICIEL "lolin32_franceinfo_ssd1306_3"
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const String feedUrl = "https://www.francetvinfo.fr/titres.rss";
const int updateInterval = 300000; // 5 minutes
unsigned long previousMillis = 0; // Stocke le temps écoulé depuis la dernière exécution de la fonction

int currentNewsIndex = 0;
int numNews = 0;
String title = "";
String description = "";
const int buttonPin = 14; // Broche du bouton1
const int button2Pin = 12; // Broche du bouton1

bool t10 = 1; //affiche les titres si =0 sinon affiche les descriptions

String ssid, password;

void updateNews() {
  HTTPClient http;

  Serial.println("Updating news...");

  if (http.begin(feedUrl)) {
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      String response = http.getString();
      int itemStart = response.indexOf("<item>");
      int itemEnd = response.indexOf("</item>", itemStart);
      int itemDataStart, itemDataEnd, titleStart, titleEnd, descriptionStart, descriptionEnd;
      String itemData;

      numNews = 0;

      while (itemStart != -1 && itemEnd != -1) {
        numNews++;

        itemData = response.substring(itemStart, itemEnd);
        itemDataStart = itemData.indexOf("<title>");
        itemDataEnd = itemData.indexOf("</title>", itemDataStart);
        title = itemData.substring(itemDataStart + 7, itemDataEnd);
        title = removeAccents(title);

        descriptionStart = itemData.indexOf("<description>");
        descriptionEnd = itemData.indexOf("</description>", descriptionStart);
        description = itemData.substring(descriptionStart + 22, descriptionEnd);
        description.replace("&lt;", "<");
        description.replace("&gt;", ">");
        description.replace("&quot;", "\"");
        description.replace("&apos;", "'");
        description.replace("&nbsp;", " ");
        description = removeAccents(description);
        //description = ":" + description;


        if (currentNewsIndex == numNews - 1) {
          /*    M5.Lcd.fillScreen(BLACK);
              M5.Lcd.setCursor(0, 0);
              M5.Lcd.setTextSize(1);
              M5.Lcd.setTextColor(GREEN);
              M5.Lcd.print(title);
              M5.Lcd.setTextSize(1);
              M5.Lcd.setTextColor(WHITE);
              //      M5.Lcd.setCursor(0, 10);
              M5.Lcd.print(description);*/
          Serial.println(title);
          display.clearDisplay();
          display.setCursor(0, 0);
          display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
          if (t10)
          {
            display.print(title);
            scrollText(title, 1);
          }
          // display.print(":");
          else
          {
            display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
            display.print(description);
            scrollText(description, 1);
            Serial.println(description);
          }

        }

        itemStart = response.indexOf("<item>", itemEnd);
        itemEnd = response.indexOf("</item>", itemStart);
      }
    }

    http.end();
  }

  Serial.println("News update complete.");
  display.display();
}

String removeAccents(String str) {

  const char* accents[] = {"À", "Á", "Â", "Ã", "Ç", "È", "É", "Ê", "Ë", "Ì", "Í", "Î", "Ï", "Ñ", "Ò", "Ó", "Ô", "Õ", "Ù", "Ú", "Û", "Ü", "à", "á", "â", "ã", "ç", "è", "é", "ê", "ë", "ì", "í", "î", "ï", "ñ", "ò", "ó", "ô", "õ", "ù", "ú", "û", "ü", "œ", "’"};
  const char* noAccents[] = {"A", "A", "A", "A", "C", "E", "E", "E", "E", "I", "I", "I", "I", "N", "O", "O", "O", "O", "U", "U", "U", "U", "a", "a", "a", "a", "c", "e", "e", "e", "e", "i", "i", "i", "i", "n", "o", "o", "o", "o", "u", "u", "u", "u", "oe", "'" };
  for (int i = 0; i < 45; i++) {
    str.replace(accents[i], noAccents[i]);
  }
  str.replace("]]>", " "); //pour ne pas avoir à reprendre le programme-affiche en fin de news

  return str;
}

void scrollText(String text, int textSize) {
  int largeur = 21, hauteur = 7;// largeur et hauteur de l'ecran en caractères
  int textLength = text.length(); // Longueur du texte
  String texteDefilant;
  int ligne = 0;
  int nbLigne = textLength / 21;
  Serial.println(textLength);
  Serial.println(nbLigne);

  for (int i = 0; i <= nbLigne - hauteur; i++)
  {
    display.clearDisplay();
    display.setCursor(0, 0);
    texteDefilant = text.substring(i * largeur, (i + hauteur) * largeur + 21);
    Serial.println(i);
    Serial.println(texteDefilant);
    display.print(texteDefilant);

    display.display();
    if (digitalRead(buttonPin) == LOW || digitalRead(button2Pin) == LOW) return; // Si 1 des 2 boutons est enfoncé on sort de la fonction
    delay(1000);
  }
  // if (digitalRead(buttonPin) == LOW) return; // Si le bouton est enfoncé

}

void setup() {
  // M5.begin();
  Serial.begin(115200);

  /* WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
    }
    Serial.println("Connected");*/
  WiFiManager wifiManager;

  // Uncomment and run it once, if you want to erase all the stored information
  //wifiManager.resetSettings();

  //variables ssid et password
  ssid = wifiManager.getWiFiSSID();
  password = wifiManager.getWiFiPass();

  // set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  // fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("AutoConnectAP");
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();

  // if you get here you have connected to the WiFi
  Serial.println("Connected.");



  /** Enable the debug via Serial port
     none debug or 0
     basic debug or 1
  */
  Serial.print("Nom du programme : ");
  Serial.println(LOGICIEL);


  /* M5.Lcd.setRotation(3);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(WHITE);*/
  pinMode(buttonPin, INPUT_PULLUP); // Mettre la broche du bouton en entrée avec une résistance de pull-up interne
  pinMode(button2Pin, INPUT_PULLUP); // Mettre la broche du bouton en entrée avec une résistance de pull-up interne
  Wire.begin(27, 26);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.display();
  //display.dim(true);
  display.setTextSize(0); // Draw 2X-scale text

  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  display.ssd1306_command(0x81); display.ssd1306_command(1); //définit la luminosité de l'écran
  updateNews();
}

void loop() {

  /*  if (M5.BtnB.wasPressed()) {
      currentNewsIndex = 0;
      updateNews();
    }*/
  //Serial.print(digitalRead(buttonPin));
  if (digitalRead(buttonPin) == LOW) { // Si le bouton est enfoncé
    currentNewsIndex++;
    if (currentNewsIndex >= numNews) {
      currentNewsIndex = 0;
    }
    updateNews();
  }

  unsigned long currentMillis = millis();
  if (digitalRead(button2Pin) == LOW) { // Si le bouton est enfoncé
    // currentNewsIndex = 0;
    t10 = !t10;
    updateNews();

  }


  if (currentMillis - previousMillis >= updateInterval) {
    // Si l'intervalle de temps s'est écoulé, lance la fonction et met à jour le temps de la dernière exécution
    previousMillis = currentMillis;
    currentNewsIndex = 0;
    updateNews();
  }
  esp_deep_sleep_enable_ext0_wakeup(buttonPin, LOW); // Réveil lorsque le bouton est enfoncé
  esp_deep_sleep_start();

  //delay(5000);
  // M5.update();
}
