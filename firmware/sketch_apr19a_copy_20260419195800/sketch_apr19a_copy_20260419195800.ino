#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <WiFi.h>
#include <SpotifyEsp32.h>
#include <SPI.h>


#define TFT_CS   1
#define TFT_RST  2
#define TFT_DC   3
#define TFT_SCLK 4
#define TFT_MOSI 5


#define BTN_PLAY 6
#define BTN_SKIP 7
#define BTN_PREV 8


char* SSID = "INSERT_SSID_HERE";
char* PASSWORD = "INSERT_PASSWORD_HERE";

const char* CLIENT_ID = "CLIENT_ID";
const char* CLIENT_SECRET = "CLIENT_SECRET";

String lastArtist = "";
String lastTrackname = "";

Spotify sp(CLIENT_ID, CLIENT_SECRET);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

String truncateText(String text, int maxLen) {
    if (text.length() <= maxLen) return text;
    return text.substring(0, maxLen - 3) + "...";
}

void setup() {
    Serial.begin(115200);

    // TFT init
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);
    tft.fillScreen(ST77XX_BLACK);

    Serial.println("TFT Initialized!");

    // Buttons 
    pinMode(BTN_PLAY, INPUT_PULLUP);
    pinMode(BTN_SKIP, INPUT_PULLUP);
    pinMode(BTN_PREV, INPUT_PULLUP);

    // WiFi
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConnected!");

    // Show IP
    tft.setCursor(0, 0);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);
    tft.write(WiFi.localIP().toString().c_str());

    delay(2000);

    // Spotify Auth
    sp.begin();
    while (!sp.is_auth()) {
        sp.handle_client();
    }
    Serial.println("Connected to Spotify!");

    tft.fillScreen(ST77XX_BLACK);
}

void loop() {

    // Button Controls
    if (digitalRead(BTN_PLAY) == LOW) {
        sp.start_resume_playback();
        delay(300);
    }
    if (digitalRead(BTN_SKIP) == LOW) {
        sp.skip();
        delay(300);
    }
    if (digitalRead(BTN_PREV) == LOW) {
        sp.previous();
        delay(300);
    }

    // Get Spotify Data
    String currentArtist = sp.current_artist_names();
    String currentTrackname = sp.current_track_name();
    bool isPlaying = sp.is_playing();

    // Only update screen if something changed
    if ((currentArtist != lastArtist && currentArtist != "Something went wrong" && !currentArtist.isEmpty()) ||
        (currentTrackname != lastTrackname && currentTrackname != "Something went wrong" && currentTrackname != "null")) {

        // Clear screen
        tft.fillScreen(ST77XX_BLACK);

        // Header
        tft.fillRect(0, 0, 160, 20, ST77XX_GREEN);
        tft.setCursor(5, 5);
        tft.setTextColor(ST77XX_BLACK);
        tft.setTextSize(1);
        tft.write("Spotify ESP32");

        // Artist
        lastArtist = currentArtist;
        tft.setCursor(10, 30);
        tft.setTextColor(ST77XX_CYAN);
        tft.write("Artist:");
        tft.setCursor(10, 45);
        tft.setTextColor(ST77XX_WHITE);
        tft.write(truncateText(lastArtist, 22).c_str());

        // Track
        lastTrackname = currentTrackname;
        tft.setCursor(10, 70);
        tft.setTextColor(ST77XX_YELLOW);
        tft.write("Track:");
        tft.setCursor(10, 85);
        tft.setTextColor(ST77XX_WHITE);
        tft.write(truncateText(lastTrackname, 22).c_str());

        // Play status
        tft.setCursor(10, 110);
        tft.setTextColor(isPlaying ? ST77XX_GREEN : ST77XX_RED);
        if (isPlaying) {
            tft.write("Playing");
        } else {
            tft.write("Paused");
        }

        // Serial debug
        Serial.println("Artist: " + lastArtist);
        Serial.println("Track: " + lastTrackname);
    }

    delay(2000); // Avoid API rate limits
}