#include <M5Core2.h>
#include <SPIFFS.h>
#include <cmath>
#include <EEPROM.h>

int address_stretch = 1;
int address_vibration_threshold = 5;
int size = 10;

float accX = 0.0F;  
float accY = 0.0F;  
float accZ = 0.0F;

float reference_angle = 0.0F;
float maximum_angle = 0.0F;

int highscore = 0;

float gyroX = 0.0F;
float gyroY = 0.0F;
float gyroZ = 0.0F;

float maxgyroX = 0.0F;
float maxgyroY = 0.0F;
float maxgyroZ = 0.0F;

float acceleration = 0.0F;
float shaking_acceleration = 0.0F;

bool killUser = false;

float time_remaining = 0.0F;

int minutes_remaining = 0;
int seconds_remaining = 0;

bool focus_session = true;
bool was_pressed = false;

int max_i;
int max_j;


void setup() {
  M5.begin();        

  M5.IMU.Init(); 

  EEPROM.begin(size);

  M5.Lcd.fillScreen(BLACK); 

  M5.Lcd.setTextColor(WHITE, BLACK);  

  M5.Lcd.setTextSize(2);  

  // shaking calibration section. Due to too much error from the IMU sensor, we could not implement an automatic movement detection section.
  // This function is replaced by an activity that sollicitates user action in an awakening way: Violently shaking the device.

  M5.Lcd.setCursor(0, 20);  

  M5.Lcd.print("New user? Leftmost button for Y, rightmost button for N.");

  while (true){
    M5.update();
    if (M5.BtnC.wasPressed()) break;

    else if (M5.BtnA.wasPressed()) {
      for (int i = 0 ; i < 10 ; ++i ) {
        EEPROM.writeInt(i, 0);
        EEPROM.commit();
        was_pressed = true;
      }
      break;
    }
    }

  M5.Lcd.clear(BLACK);

  M5.Lcd.setCursor(0, 20); 
  if (was_pressed){
    was_pressed = false;
    M5.Lcd.printf("Please shake the device as you normally would");

    for (int i = 0 ; i <= 1000 ; ++i){
      M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);

      if (abs(gyroX) > maxgyroX) maxgyroX = abs(gyroX);
      if (abs(gyroY) > maxgyroY) maxgyroY = abs(gyroY);
      if (abs(gyroZ) > maxgyroZ) maxgyroZ = abs(gyroZ);

      shaking_acceleration = sqrt( maxgyroX * maxgyroX + maxgyroY * maxgyroY + maxgyroZ * maxgyroZ );

      EEPROM.writeInt(address_vibration_threshold, shaking_acceleration);

      EEPROM.commit();

      delay(10);


    }
  }


}


void loop() {
  if (focus_session){
      shaking_acceleration = EEPROM.readInt(address_vibration_threshold);

      max_i = 10; //Change max i for the study session duration (in seconds)

      max_i *= 2; 

      for (int i = 0; i <= max_i; ++i){
        M5.Lcd.clear(BLACK);

        M5.Lcd.setCursor(0, 20);  
    
        M5.Lcd.printf("30 Minutes Focus Session.");

        M5.Lcd.setCursor(0, 62);  
    
        M5.Lcd.printf("Don't forget to shake me  if you get up!");

        M5.Lcd.setCursor(0, 122);  

        time_remaining = (max_i-i)/2;

        minutes_remaining = int(time_remaining / 60);

        seconds_remaining = int( time_remaining - minutes_remaining * 60 );
    
        M5.Lcd.printf("Time remaining:");

        M5.Lcd.setCursor(0, 142);

        M5.Lcd.printf("%i min and %i s", minutes_remaining , seconds_remaining );

        M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);

        acceleration = sqrt( gyroX * gyroX + gyroY * gyroY + gyroZ * gyroZ );

        if (acceleration > shaking_acceleration){
          M5.Lcd.clear(BLACK);
          
          M5.Lcd.setCursor(0, 20);

          M5.Lcd.printf("Pause initiated! You have 5 minutes to come back to your desk, or touch any button to resume studying");

          max_j = 5; //Change max j for the pause duration (in seconds)

          max_j *= 20;

          for (int j = 0 ; j<= max_j ; ++j){
            M5.update();

            if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) break;

            M5.Lcd.setCursor(0, 122); 

            M5.Lcd.printf("Time remaining:");

            time_remaining = (max_j-j)/20;

            minutes_remaining = int(time_remaining / 60);

            seconds_remaining = int( time_remaining - minutes_remaining * 60 );

            M5.Lcd.setCursor(0, 142);

            M5.Lcd.printf("%i min and %i s", minutes_remaining , seconds_remaining );

            delay(50);
            
          }

        } 

        else{
          delay(500);

          if (i == max_i)
            killUser = true;
        }
        }

      if (killUser == true){
        killUser = false;

        M5.Lcd.clear(BLACK);

        M5.Lcd.setCursor(0, 20);
        
        M5.Lcd.printf("It s been 30 minutes without you moving.");

        M5.Lcd.setCursor(0, 72);

        M5.Lcd.printf("Shake the device to stop the alert.");

        while (true){
          M5.Axp.SetLDOEnable(3, true);  //Open the vibration.  

          M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);

          acceleration = sqrt( gyroX * gyroX + gyroY * gyroY + gyroZ * gyroZ );

          if (acceleration > shaking_acceleration){
            M5.Lcd.clear(BLACK);

            M5.Lcd.setCursor(0, 20);
        
            M5.Lcd.printf("Alert stopped. ");

            M5.Lcd.setCursor(0, 42);
        
            M5.Lcd.printf("Touch the leftmost button to start another Focus Session, ");

            M5.Lcd.setCursor(0, 62);
        
            M5.Lcd.printf("or the rightmost button to do a stretching exercise.");

            break;
            }

          delay(200);
          
          M5.Axp.SetLDOEnable(3, false);  //Close the vibration.  
          
          M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);

          acceleration = sqrt( gyroX * gyroX + gyroY * gyroY + gyroZ * gyroZ );

          if (acceleration > shaking_acceleration){
            M5.Lcd.clear(BLACK);

            M5.Lcd.setCursor(0, 20);
        
            M5.Lcd.printf("Alert stopped. ");

            M5.Lcd.setCursor(0, 42);
        
            M5.Lcd.printf("Touch the leftmost button to start another Focus Session, ");

            M5.Lcd.setCursor(0, 62);
        
            M5.Lcd.printf("or the rightmost button to do a stretching exercise.");

            break;
            }

          delay(200);
          
          M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);

          acceleration = sqrt( gyroX * gyroX + gyroY * gyroY + gyroZ * gyroZ );

          if (acceleration > shaking_acceleration){
            M5.Lcd.clear(BLACK);

            M5.Lcd.setCursor(0, 20);
        
            M5.Lcd.printf("Alert stopped. ");

            M5.Lcd.setCursor(0, 42);
        
            M5.Lcd.printf("Touch the leftmost button to start another Focus Session, ");

            M5.Lcd.setCursor(0, 82);
        
            M5.Lcd.printf("or the rightmost button to do a stretching exercise.");

            break;
            }
        }

        while (true){
          M5.update();
          if (M5.BtnC.wasPressed()) {
            focus_session = false;
            break;
          }
          

          else if (M5.BtnA.wasPressed()) break;
          }
      }
    }
    
    else{
      M5.Lcd.clear(BLACK);

      M5.Lcd.setCursor(0, 20);

      M5.Lcd.printf("Stretching session. ");

      M5.Lcd.setCursor(0, 42);

      M5.Lcd.printf("Attach the device to your upper chest and stand still. ");

      M5.Lcd.setCursor(0, 82);

      M5.Lcd.printf("You can stretch forward when you feel a vibration. ");

      M5.Lcd.setCursor(0, 122);

      M5.Lcd.printf("Whenever you feel comfortable, get up.");

      M5.Lcd.setCursor(0, 162);

      M5.Lcd.printf("Once you're ready, touch any button.");

      while (true){
        M5.update();
        if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) break;
      }

      for (int i=0 ; i<= 500 ; ++i){
        M5.IMU.getAccelData(&accX, &accY, &accZ); 

        if (accZ >= 0) reference_angle = 90 - accY *90;

        else reference_angle = accY *90 - 90;
        
        delay(10);
      }

      M5.Lcd.clear(BLACK);

      M5.Lcd.setCursor(0, 20);

      M5.Lcd.printf("Stretching session. ");

      M5.Lcd.setCursor(0, 42);

      M5.Lcd.printf("You can stretch now! ");

      M5.Axp.SetLDOEnable(3, true);  //Open the vibration.  

      delay(1000);

      M5.Axp.SetLDOEnable(3, false);  //Close the vibration.

      delay(5000);

      do{
        M5.IMU.getAccelData(&accX, &accY, &accZ);

        accY = 90 - accY *90 + reference_angle;

        if (accY > maximum_angle) maximum_angle = accY;
      }
      while ( accY >= (reference_angle + 5 ) );
      
      M5.Lcd.clear(BLACK);

      M5.Lcd.setCursor(0, 20);

      M5.Lcd.printf("Stretching session. ");

      M5.Lcd.setCursor(0, 42);

      M5.Lcd.printf("Good stretch! Your angle was %5.2f ", maximum_angle);

      M5.Lcd.setCursor(0, 82);

      highscore = EEPROM.readInt(address_stretch);

      if ( (int)maximum_angle > highscore) {
        M5.Lcd.printf("Congrats! New highscore: %d", (int)maximum_angle);

        EEPROM.writeInt(address_stretch, (int)maximum_angle);

        EEPROM.commit();
      }

      else {
        M5.Lcd.printf("Your highscore: %d", highscore);

        M5.Lcd.setCursor(0, 102);

        M5.Lcd.printf("This session's score: %d", (int)maximum_angle);
      }

      M5.Axp.SetLDOEnable(3, true);  //Open the vibration.  

      delay(1000);

      M5.Axp.SetLDOEnable(3, false);  //Close the vibration.

      M5.Lcd.setCursor(0, 122);

      M5.Lcd.printf("Study: leftmost button. Stretch: rightmost button");

      while (true){
        M5.update();
        if (M5.BtnC.wasPressed()) break;

        else if (M5.BtnA.wasPressed()) {
          focus_session = true;
          break;
        }
    }


    }



  }





















