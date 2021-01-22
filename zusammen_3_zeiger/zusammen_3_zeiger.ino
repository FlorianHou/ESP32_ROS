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
const char* ssid     = "robo";
const char* password = "xxxxxxxxxxxxxxxxxxxxxx";

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
float L_Y_val;
float L_X_val;
float R_Y_val;
float R_X_val;
float wider_val;
float L_X_zen, L_Y_zen, R_X_zen, R_Y_zen;
unsigned int J_summ;

//Menge des aufgenommenden Werten
int samples = 100;
//Verstärkung oder MAX_SPEED!!!
const float faktor_raw =  0.5;
float faktor = 0;
//say hello
char hello[30] = "Chef Gnasa, Master Lukas!!";

//ROS Server
IPAddress server(192,168,2,165);
//ROS Port
const uint16_t serverPort = 11411;
ros::NodeHandle nh;
//Make a chatter publisher
std_msgs::String str_msg;
geometry_msgs::Twist twist_msg;
//Name der Topic
ros::Publisher chatter("chatter", &str_msg);
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
  nh.advertise(chatter);
  nh.advertise(chatter_twist);

  //Joystick*************************
  faktor = faktor_raw;
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
  L_Y_zen = kalibrieren(L_Y);
  Serial.println(L_X_zen);
  
  Serial.println("R: ");
  R_X_zen = kalibrieren(R_X);
  R_Y_zen = kalibrieren(R_Y);
  Serial.println(R_X_zen);
  digitalWrite(led, LOW);
}

void loop()
{ 
  if (L_SW_val){
    faktor = faktor_raw; 
    digitalWrite(led, LOW);

  }
  if (R_SW_val){
    faktor = 0.0;
    digitalWrite(led, HIGH);
  }


  //Joystick*************************************
  //wider
  Serial.print("wider: ");
  wider_val = analogRead(wider)*faktor/ 4096.;
  Serial.println(wider_val);
  vspiCommand(nummer[int(wider_val*10/faktor)]);

  // L_SW
  Serial.print("L_SW: ");
  L_SW_val =! digitalRead(L_SW);
  Serial.println(L_SW_val);
  //R_SW
  Serial.print("R_SW: ");
  R_SW_val =! digitalRead(R_SW);
  Serial.println(R_SW_val);
  //L_X
  Serial.print("L_X: ");
  L_X_val = -(analogRead(L_X)-L_X_zen)*wider_val/2048.;
  Serial.println(L_X_val);
  //L_Y
  Serial.print("L_Y: ");
  L_Y_val = (analogRead(L_Y)-L_Y_zen)*wider_val/2048.;
  Serial.println(L_Y_val);
  //R_X
  Serial.print("R_X: ");
  R_X_val = -(analogRead(R_X)-R_X_zen)*wider_val/2048.;
  Serial.println(R_X_val);
  //R_Y
  Serial.print("R_Y: ");
  R_Y_val = (analogRead(R_Y)-R_Y_zen)*wider_val/2048.;
  Serial.println(R_Y_val);

    
    //ROS***********************************************
    if (nh.connected()) {
      Serial.println("Connected");
    // Say hello
    str_msg.data = hello;
    //twist_msg
    twist_msg.linear.x = L_X_val*2;
    twist_msg.linear.y = R_Y_val*2;
    twist_msg.angular.z = L_Y_val;
    chatter.publish( &str_msg );
    chatter_twist.publish( &twist_msg );
    } else {
      Serial.println("Not Connected");
    }
    nh.spinOnce();
    // Loop exproximativity
    delay(200);
}

float kalibrieren(int J_Pin) {
  Serial.println("***************Kalibrieren*********************");
  
    J_summ = 0;
    
    for (int i =0 ; i < samples; i++){
      J_summ += analogRead(J_Pin);
      delay(20);
    }
    //Zentrierung
    float J_zen;
    J_zen = J_summ / float(samples);

    return J_zen;
}

void vspiCommand(int data) {
//  byte data = B0111111; // junk data to illustrate usage
  Serial.println(data);
  //use it as you would the regular arduino SPI API
  vspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_SS, LOW); //pull SS slow to prep other end for transfer
  vspi->transfer(data);
  digitalWrite(VSPI_SS, HIGH); //pull ss high to signify end of data transfer
  vspi->endTransaction();
}
