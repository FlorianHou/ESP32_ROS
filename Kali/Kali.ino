int L_X_zen, L_Y_zen, R_X_zen, R_Y_zen;
int L_SW = 33;
int L_Y = 32;
int L_X = 35;
int R_SW = 34;
int R_Y = 39;
int R_X = 36;

//unsigned int J_X_summ;
//unsigned int J_Y_summ;

void setup() {
  Serial.begin(115200);
  //Kalibieren
  Serial.println("L: ");
  L_X_zen, L_Y_zen = kalibrieren(L_X, L_Y);

  Serial.println("R: ");
  R_X_zen, R_Y_zen = kalibrieren(R_X, R_Y);
}

void loop() {


}

int kalibrieren(int J_X, int J_Y) {
  Serial.println("***************Kalibrieren*********************");
    int J_X_summ=0, J_Y_summ=0;
    for (int i =0 ; i < 50; i++){
      J_X_summ += analogRead(J_X);
      J_Y_summ += analogRead(J_Y);
      delay(20);
    }
    //Zentierung
    int J_X_zen;
    J_X_zen = J_X_summ / 50;
    int J_Y_zen;
    J_Y_zen = J_Y_summ / 50;
    
    return J_X_zen, J_Y_zen;
}
