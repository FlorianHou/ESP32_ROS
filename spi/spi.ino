

/* The ESP32 has four SPi buses, however as of right now only two of
 * them are available to use, HSPI and VSPI. Simply using the SPI API 
 * as illustrated in Arduino examples will use VSPI, leaving HSPI unused.
 * 
 * However if we simply intialise two instance of the SPI class for both
 * of these buses both can be used. However when just using these the Arduino
 * way only will actually be outputting at a time.
 * 
 * Logic analyser capture is in the same folder as this example as
 * "multiple_bus_output.png"
 * 
 * created 30/04/2018 by Alistair Symonds
 */
#include <SPI.h>

// Define ALTERNATE_PINS to use non-standard GPIO pins for SPI bus
// µ#define ALTERNATE_PINS
#ifdef ALTERNATE_PINS
  #define VSPI_MISO   15
  #define VSPI_MOSI   4
  #define VSPI_SCLK   4
  #define VSPI_SS     0

#else
  #define VSPI_MISO   MISO
  #define VSPI_MOSI   MOSI
  #define VSPI_SCLK   SCK
  #define VSPI_SS     SS

#endif

static const int spiClk = 1000000; // 1 MHz

//uninitalised pointers to SPI objects
SPIClass * vspi = NULL;

void setup() {
  //initialise two instances of the SPIClass attached to VSPI and HSPI respectively
  vspi = new SPIClass(VSPI);
  Serial.begin(115200);
  //clock miso mosi ss

#ifndef ALTERNATE_PINS
  //initialise vspi with default pins
  //SCLK = 18, MISO = 19, MOSI = 23, SS = 5
  vspi->begin();
#else
  //alternatively route through GPIO pins of your choice
  vspi->begin(VSPI_SCLK, VSPI_MISO, VSPI_MOSI, VSPI_SS); //SCLK, MISO, MOSI, SS
#endif


  //set up slave select pins as outputs as the Arduino API
  //doesn't handle automatically pulling SS low
  pinMode(VSPI_SS, OUTPUT); //VSPI SS
}

// the loop function runs over and over again until power down or reset
void loop() {
  //use the SPI buses
  vspiCommand();
  Serial.println(VSPI_MISO);
  delay(1000);
}

void vspiCommand() {
  byte data = B0111111; // junk data to illustrate usage
  Serial.println(data);

  //use it as you would the regular arduino SPI API
  vspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_SS, LOW); //pull SS slow to prep other end for transfer
  vspi->transfer(B00000110);
  delay(100);  
  digitalWrite(VSPI_SS, HIGH); //pull ss high to signify end of data transfer
  delay(100);
  digitalWrite(VSPI_SS, LOW); //pull SS slow to prep other end for transfer
  delay(100);  
  vspi->transfer(data);  
  digitalWrite(VSPI_SS, HIGH); //pull ss high to signify end of data transfer
  delay(100);  

  vspi->endTransaction();
}
