// Mittelab OTP ACCESS, v1.0


#include <WiFi.h>
#include <NTPClient.h>
#include <TOTP.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// change the following settings according to your WiFi network
char ssid[] = "Vodafone-A44066214";
char password[] = "bkcRYhattEy2HfgA";

// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "5098952632:AAGkJ3xONKbHnyk-jdwnj7XjJzqU6GBY09E"
const unsigned long BOT_MTBS = 1000; // mean time between scan messages

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime; // last time messages' scan has been done

//defining pin
#define BUTTONPIN 0
int buttonPress;
int ledStatus = 0;
const int ledPin = 13;

// enter your hmacKey (10 digits)
uint8_t hmacKey[] = {0x62, 0x6b, 0x63, 0x52, 0x59, 0x68, 0x61, 0x74, 0x74, 0x45, 0x79, 0x32, 0x48, 0x66, 0x67, 0x41};

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
TOTP totp = TOTP(hmacKey, 16);

String totpCode = String("");
String mittelabOTP = ("");
String newCode = String(totp.getCode(timeClient.getEpochTime()));

void handleNewMessages(int numNewMessages)
{
  for (int i = 0; i < numNewMessages; i++)
  {
    // Inline buttons with callbacks when pressed will raise a callback_query message
    if (bot.messages[i].type == "callback_query")
    {
      Serial.print("Call back button pressed by: ");
      Serial.println(bot.messages[i].from_id);
      Serial.print("Data on the button: ");
      Serial.println(bot.messages[i].text);
      bot.sendMessage(bot.messages[i].from_id, bot.messages[i].text, "");
    }
    else
    {
      String chat_id = bot.messages[i].chat_id;
      String text = bot.messages[i].text;

      String from_name = bot.messages[i].from_name;
      if (from_name == "")
        from_name = "Guest";

      if (text == "/start")
      {
        String keyboardJson = "[[{ \"text\" : \"Go to Access Registration Page\", \"url\" : \"https://otp.mittelab.org\" }],[{ \"text\" : \"Open Lab\", \"callback_data\" : \"Press the open button\" }]]";
        bot.sendMessageWithInlineKeyboard(chat_id, "Choose from one of the following options", "", keyboardJson);
        buttonPress = digitalRead(BUTTONPIN);
        
        for (int i = 0; i < 1; i++){
           String chat_id = bot.messages[i].chat_id;
           mittelabOTP = bot.messages[i].text;

           if (buttonPress){
           bot.sendMessage( bot.messages[i].chat_id,"button pressed, now enter OTP","");

           if (newCode = mittelabOTP){
             bot.sendMessage(chat_id, "Access Granted", "");
           }
        }
       }
      } 

    }
  }
}

void doorStatus (){
  
}

void setup() {
  pinMode(BUTTONPIN, INPUT);

  Serial.begin(9600);
  while (!Serial);

  Serial.println("TOTP demo");
  Serial.println();
  
  // connect to the WiFi network
  WiFi.begin(ssid, password);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Establishing connection to WiFi...");
  }
  Serial.print("Connected to WiFi with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // start the NTP client
  timeClient.begin();
  Serial.println("NTP client started");
  Serial.println();

  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);
}

void loop() {

  // update the time 
  timeClient.update();

  // generate the TOTP code and, if different from the previous one, print to screen
  
  if(totpCode!= newCode) {
    totpCode = String(newCode);
    Serial.print("TOTP code: ");
    Serial.println(newCode);
  }

  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}
