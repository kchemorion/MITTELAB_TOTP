//Needed Header Files
#include <WiFi.h>
#include <NTPClient.h>
#include <TOTP.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// WiFi credentials
char ssid[] = "Vodafone-A44066214";
char password[] = "bkcRYhattEy2HfgA";

// Telegram BOT credentials
#define BOT_TOKEN "5098952632:AAGkJ3xONKbHnyk-jdwnj7XjJzqU6GBY09E"
const unsigned long BOT_MTBS = 1000; // mean time between scan messages

//initializing settings
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime; // last time messages' scan has been done

//defining pin and relay
#define BUTTONPIN 0
int buttonPress;
int ledStatus = 0;
const int ledPin = 13;

// hmacKey
uint8_t hmacKey[] = {0x62, 0x6b, 0x63, 0x52, 0x59, 0x68, 0x61, 0x74, 0x74, 0x45, 0x79, 0x32, 0x48, 0x66, 0x67, 0x41};

//global variables
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
TOTP totp = TOTP(hmacKey, 16);
String totpCode = String("");
String mittelabOTP = ("");
String newCode = String(totp.getCode(timeClient.getEpochTime()));

void handleResponses(int numNewResponses){
     // update the time 
  timeClient.update();

  // generate the TOTP code and, if different from the previous one, print to screen
  
  if(totpCode!= newCode) {

    totpCode = String(newCode);
    Serial.print("TOTP code: ");
    Serial.println(newCode);

  for (int j= 0; j < 3; j++)
  {
    String chat_id = bot.messages[j].chat_id;
    mittelabOTP = bot.messages[j].text;

    String from_name = bot.messages[j].from_name;
      if (from_name == "")
        from_name = "Guest";

    if(newCode == mittelabOTP){
        bot.sendMessage(chat_id, "Access Granted", "");
    } else{
        bot.sendMessage(chat_id, "Incorrect OTP", "");
    }

  }

}

}

void handleNewMessages(int numNewMessages)
{
  for (int i = 0; i < numNewMessages; i++)
  {
    // Inline buttons with callbacks when pressed will raise a callback_query message
    if (bot.messages[i].type == "callback_query")
    {
        if (bot.messages[i].text == "Press the open button"){
        buttonPress = digitalRead(BUTTONPIN);
        printf("buttonStatus=%d", buttonPress);
        bot.sendMessage( bot.messages[i].chat_id,"button pressed, now enter OTP","");
        };
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
      } 

    }
  }
}

void setup(){
 pinMode(BUTTONPIN, INPUT);

  Serial.begin(9600);
  while (!Serial);

  Serial.println("Mittelab TOTP");
  Serial.println();
  
  // connect to the WiFi network
  WiFi.begin(ssid, password);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED) {

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
}
void loop(){
    if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    int numNewResponses = numNewMessages;

    while (numNewMessages)
    {
      handleNewMessages(numNewMessages);
      handleResponses(numNewResponses);
      
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }

}
