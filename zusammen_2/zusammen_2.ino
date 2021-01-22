#include <WiFi.h>
#include <ros.h>
#include <std_msgs/String.h>
#include <geometry_msgs/Twist.h>
//WIFI
const char* ssid     = "robo";
const char* password = "Password";


//Pin
int L_SW = 0;
int L_Y = 32;
int L_X = 35;
int R_SW = 34;
int R_Y = 39;
int R_X = 36;
int wider = 33;
// Value
bool L_SW_val;
bool R_SW_val;
float L_Y_val;
float L_X_val;
float R_Y_val;
float R_X_val;
float wider_val;
float L_X_zen, L_Y_zen, R_X_zen, R_Y_zen;
unsigned int J_summ;
int samples = 50;
//Verstärkung
float faktor = 0.2;

  //say hello
char hello[30] = "Chef Gnasa, Master Lukas!!";

//ROS Server
IPAddress server(192,168,2,118);
//ROS Port
const uint16_t serverPort = 11411;

ros::NodeHandle nh;
//Make a chatter publisher
std_msgs::String str_msg;
geometry_msgs::Twist twist_msg;
//Name der Topic
ros::Publisher chatter("chatter", &str_msg);
ros::Publisher chatter_twist("cmd_vel", &twist_msg);


void setup() {
  Serial.begin(115200);
    Serial.println(analogRead(wider));
  Serial.print("Connecting to ");
  Serial.println(ssid);
  //WIFI*****************
      Serial.println(analogRead(wider));

  WiFi.begin(ssid, password);
      Serial.println(analogRead(wider));

  //while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    //Serial.print(".");
  //}
      Serial.println(analogRead(wider));

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("ESP Mac Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
    Serial.println(analogRead(wider));
  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("DNS: ");
  Serial.println(WiFi.dnsIP());
    Serial.println(analogRead(wider));

  // Set the connection to rosserial socket server
  nh.getHardware()->setConnection(server, serverPort);
  nh.initNode();
  //Get IP durch ROS
  Serial.print("IP = ");
  Serial.println(nh.getHardware()->getLocalIP());
    Serial.println(analogRead(wider));

  //Start to be polite
  nh.advertise(chatter);
  nh.advertise(chatter_twist);

  //Joystick*************************
  pinMode(L_SW, INPUT);
  pinMode(R_SW, INPUT);
  Serial.println("Hi, Florian!");
  Serial.println(analogRead(wider));

  //Kalibieren*****************************
  Serial.println("L: ");
  L_X_zen = kalibrieren(L_X);
  L_Y_zen = kalibrieren(L_Y);
  Serial.println(L_X_zen);
  
  Serial.println("R: ");
  R_X_zen = kalibrieren(R_X);
  R_Y_zen = kalibrieren(R_Y);
  Serial.println(R_X_zen);

}

void loop() {
  //wider
  Serial.print("wider: ");
  wider_val = analogRead(wider)*faktor/ 4096.;
  Serial.println(wider_val);
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
  L_X_val = -(analogRead(L_X)-L_X_zen)*wider_val/2048;
  Serial.println(L_X_val);
  //L_Y
  Serial.print("L_Y: ");
  L_Y_val = (analogRead(L_Y)-L_Y_zen)*wider_val/2048;
  Serial.println(L_Y_val);
  //R_X
  Serial.print("R_X: ");
  R_X_val = -(analogRead(R_X)-R_X_zen)*wider_val/2048;
  Serial.println(R_X_val);
  //R_Y
  Serial.print("R_Y: ");
  R_Y_val = (analogRead(R_Y)-R_Y_zen)*wider_val/2048;
  Serial.println(R_Y_val);

  delay(200);
}

float kalibrieren(int J_Pin) {
  Serial.println("***************Kalibrieren*********************");
  
    J_summ = 0;
    
    for (int i =0 ; i < samples; i++){
      J_summ += analogRead(J_Pin);
      delay(20);
    }
    //Zentierung
    float J_zen;
    J_zen = J_summ / float(samples);

    return J_zen;
}
