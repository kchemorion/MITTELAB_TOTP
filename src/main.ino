//Needed Header Files
#include <WiFi.h>
#include <NTPClient.h>
#include <TOTP.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// WiFi credentials
char ssid[] = "xxxxxxxxx";
char password[] = "xxxxxxxx";

// Telegram BOT credentials
#define BOT_TOKEN "xxxxxxxxxxx:xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
const unsigned long BOT_MTBS = 1000; // mean time between scan messages

//initializing settings
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime; // last time messages' scan has been done

//defining pin
#define ESP_INTR_FLAG_DEFAULT 0
#define BUTTONPIN 0

int buttonPress;
int ledStatus = 0;
const int ledPin = 13;

// hmacKey
uint8_t hmacKey[] = {0x35, 0x35, 0x45, 0x78, 0x74, 0x65, 0x6e, 0x73, 0x69, 0x6f, 0x6e, 0x24};

//global variables
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
TOTP totp = TOTP(hmacKey, 12);
String totpCode = String("");
String mittelabOTP = ("");
String newCode = String(totp.getCode(timeClient.getEpochTime()));
String nextTextMessage;
int Status;

String nextText(){
  for (int k= 0; k < 1; k++)
  {
  String chat_id = bot.messages[k].chat_id;
  nextTextMessage = bot.messages[k].text;
  }
  return nextTextMessage;
  delay(1000);
}

void sendMessage(String message){
    for (int k= 0; k < 1; k++)
  {
  String chat_id = bot.messages[k].chat_id;
  bot.sendMessage(chat_id, message, "");
  }
}

String generateOTP(){
  // update the time 
  timeClient.update();

  // generate the TOTP code and, if different from the previous one, print to screen
    if(totpCode!= newCode) {

    totpCode = String(newCode);
    Serial.print("TOTP code: ");
    Serial.println(newCode);
    return totpCode;
  }
}

void handleResponses(int numNewResponses){
    
    String password = nextText();

    if(password == mittelabOTP){
        sendMessage("Access Granted");
    } else{
        sendMessage("Incorrect OTP");
    }

    int Status = 3;

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

        if(buttonPress == 1){
          bot.sendMessage( bot.messages[i].chat_id,"button has been pressed, noe enter OTP","");
        }else{
          bot.sendMessage( bot.messages[i].chat_id,"waiting for button to be pressed","");
        }
        };
        Status = 1;
    }
    else
    {
      String chat_id = bot.messages[i].chat_id;
      String text = bot.messages[i].text;
      Status = 1;
      String from_name = bot.messages[i].from_name;
      if (from_name == "")
        from_name = "Guest";

      if (text == "/start")
      {
        String keyboardJson = "[[{ \"text\" : \"Go to Access Registration Page\", \"url\" : \"https://otp.mittelab.org\" }],[{ \"text\" : \"Open Lab\", \"callback_data\" : \"Press the open button\" }]]";
        bot.sendMessageWithInlineKeyboard(chat_id, "Choose from one of the following options", "", keyboardJson);
        Status = 0;
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

  if (millis() - bot_lasttime > BOT_MTBS){
  
  mittelabOTP = generateOTP();
  
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  int numNewResponses = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages)
    {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

  while (numNewResponses)
    {
      handleResponses(numNewResponses);
      numNewResponses = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }

}
