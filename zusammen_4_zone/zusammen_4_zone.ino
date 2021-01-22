#include <WiFi.h>
#include <ros.h>
#include <std_msgs/String.h>
#include <geometry_msgs/Twist.h>
#include <SPI.h>



//SPI******************
#define VSPI_MISO   MISO
#define VSPI_MOSI   MOSI
#define VSPI_SCLK   SCK
#define VSPI_SS     SS
static const int spiClk = 1000000; // 1 MHz
//uninitalised pointers to SPI objects
SPIClass * vspi = NULL;
int nummer[10] = {63,6,91,79,102,109,125,7,127,111};

//WIFI
const char* ssid     = "QUST";
const char* password = "xxxxxxxxxxxxxxxxx";

//Pin
const int L_SW = 0;
const int L_Y = 32;
const int L_X = 35;
const int R_SW = 34;
const int R_Y = 39;
const int R_X = 36;
const int wider = 33;
const int led = 2;

// Value
bool L_SW_val = 0;
bool R_SW_val = 0;
bool notfall = 0;
int L_Y_val;
int L_X_val;
int R_Y_val;
int R_X_val;
float wider_val;
int L_X_zen, L_Y_zen, R_X_zen, R_Y_zen;

float L_X_corr_plus;
float L_X_corr_minus;

float L_Y_corr_plus;
float L_Y_corr_minus;

float R_X_corr_plus;
float R_X_corr_minus;

float R_Y_corr_plus;
float R_Y_corr_minus;

  //Menge des aufgenommenden Werten
int samples = 100;
  //MAX_SPEED!!!
const float max_x_speed = 0.1;
const float max_y_speed = 0.05;
const float max_r_speed = 0.2618;

//say hello
char hello[30] = "Chef Gnasa, Master Lukas!!";

//ROS Server
IPAddress server(192,168,1,109);
//ROS Port
const uint16_t serverPort = 11411;
ros::NodeHandle nh;
//Make a chatter publisher
std_msgs::String str_msg;
geometry_msgs::Twist twist_msg;
//Name der Topic
ros::Publisher chatter_twist("cmd_vel", &twist_msg);


void setup()
{   
  Serial.begin(115200);
  //SPI*****************************
  //initialise two instances of the SPIClass attached to VSPI and HSPI respectively
  vspi = new SPIClass(VSPI);
  //clock miso mosi ss
  //alternatively route through GPIO pins of your choice
  vspi->begin(VSPI_SCLK, VSPI_MISO, VSPI_MOSI, VSPI_SS); //SCLK, MISO, MOSI, SS
  //set up slave select pins as outputs as the Arduino API
  //doesn't handle automatically pulling SS low
  pinMode(VSPI_SS, OUTPUT); //VSPI SS
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  //WIFI*****************
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("ESP Mac Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("DNS: ");
  Serial.println(WiFi.dnsIP());
  // Set the connection to rosserial socket server
  nh.getHardware()->setConnection(server, serverPort);
  nh.initNode();
  //Get IP durch ROS
  Serial.print("IP = ");
  Serial.println(nh.getHardware()->getLocalIP());
  //Start to be polite
  nh.advertise(chatter_twist);

  //Joystick*************************
  pinMode(L_SW, INPUT);
  pinMode(R_SW, INPUT);
  Serial.println("Hi, Florian!");
  analogReadResolution(12);
  analogSetWidth(12);
  
  //LED***************************
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  //Kalibieren*****************************
  for(int i=0; i<5; i++){
  digitalWrite(led, HIGH);
  delay(200);
  digitalWrite(led, LOW);
  delay(100);
  }

  digitalWrite(led, HIGH);
  delay(1000);
  Serial.println("L: ");
  L_X_zen = kalibrieren(L_X);
  //Korrktor
  L_X_corr_plus = 2048./(4096 - L_X_zen);
  L_X_corr_minus = 2048./L_X_zen;

  L_Y_zen = kalibrieren(L_Y);
  //Korrktor
  L_Y_corr_plus = 2048./(4096 - L_Y_zen);
  L_Y_corr_minus = 2048./L_Y_zen;
  Serial.println(L_Y_zen);
  
  Serial.println("R: ");
  R_X_zen = kalibrieren(R_X);
  //Korrktor
  R_X_corr_plus = 2048./(4096 - R_X_zen);
  R_X_corr_minus = 2048./R_X_zen;
  
  R_Y_zen = kalibrieren(R_Y);
  //Korrktor
  R_Y_corr_plus = 2048./(4096 - R_Y_zen);
  R_Y_corr_minus = 2048./R_Y_zen;
  Serial.println(R_Y_zen);
  digitalWrite(led, LOW);
}

void loop()
{ 
  //Joystick*************************************
  //wider
  Serial.print("wider: ");
  wider_val = analogRead(wider)/4096.;
  int nummer_zeiger = analogRead(wider)/4096. * 10;
  Serial.println(nummer_zeiger);
  vspiCommand(nummer[nummer_zeiger]);
  
  if (L_SW_val){
    notfall = false; 
    digitalWrite(led, LOW);

}
  if (R_SW_val){
    notfall = true;
    digitalWrite(led, HIGH);
}
  
  // L_SW
  Serial.print("L_SW: ");
  L_SW_val =! digitalRead(L_SW);
  Serial.println(L_SW_val);
  //R_SW
  Serial.print("R_SW: ");
  R_SW_val =! digitalRead(R_SW);
  Serial.println(R_SW_val);

  if(!notfall){
  //L_X
  L_X_val = death_zone(analogRead(L_X), L_X_zen);
  //L_Y
  L_Y_val = death_zone(analogRead(L_Y), L_Y_zen);
  //R_X
  R_X_val = death_zone(analogRead(R_X),R_X_zen);
  //R_Y
  R_Y_val = death_zone(analogRead(R_Y),R_Y_zen);
  }
  else{
    L_X_val = 0.;
    L_Y_val = 0.;
    R_X_val = 0.;
    R_Y_val = 0.;
  }

  Serial.print("L_X: ");
  Serial.println(L_X_val);
  Serial.print("L_Y: ");
  Serial.println(L_Y_val);
  Serial.print("R_X: ");
  Serial.println(R_X_val);
  Serial.print("R_Y: ");
  Serial.println(R_Y_val);


    
    //ROS***********************************************
    if (nh.connected()) {
      Serial.println("Connected");
    // Say hello
    str_msg.data = hello;
    //twist_msg
    Serial.println(max_x_speed);
    Serial.println(wider_val);
    Serial.println(L_X_val/2048.);
    twist_msg.linear.x = -max_x_speed * wider_val * value_remap(L_X_corr_plus,L_X_corr_minus, L_X_val/2048.);
    twist_msg.linear.y = max_y_speed * wider_val * value_remap(R_Y_corr_plus,R_Y_corr_minus,R_Y_val/2048.);
    twist_msg.angular.z = max_r_speed * wider_val * value_remap(L_Y_corr_plus,L_Y_corr_minus,L_Y_val/2048.);
    chatter_twist.publish( &twist_msg );
    } else {
      Serial.println("Not Connected");
    }
    nh.spinOnce();
    // Loop exproximativity
    delay(200);
}

int kalibrieren(int J_Pin) {
  Serial.println("***************Kalibrieren*********************");
  
    unsigned int J_summ = 0;
    for (int i =0 ; i < samples; i++){
      J_summ += analogRead(J_Pin);
      delay(20);
    }
    //Zentrierung
    return J_summ / samples;
}

int death_zone(int joy_raw, int joy_zen){
  int joy_cali = joy_raw - joy_zen;
  if(abs(joy_cali) <= 200){
    return 0;
  }
  else{
    return joy_cali;
  }
}

float value_remap(float korr_plus, float korr_minus, float value_raw){
  if (value_raw > 0){
    return value_raw*korr_plus;
  }
  if (value_raw < 0){
    return value_raw*korr_minus;
  }
  else{
    return value_raw;
  }
}

void vspiCommand(int data) {
//  byte data = B0111111; // junk data to illustrate usage
//  Serial.println(data);
  //use it as you would the regular arduino SPI API
  vspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_SS, LOW); //pull SS slow to prep other end for transfer
  vspi->transfer(data);
  digitalWrite(VSPI_SS, HIGH); //pull ss high to signify end of data transfer
  vspi->endTransaction();
}
