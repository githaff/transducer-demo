/*
 * A simple hardware test which receives audio from the audio shield
 * Line-In pins and send it to the Line-Out pins and headphone jack.
 *
 * This example code is in the public domain.
 */

#include <Audio.h>
#include <Wire.h>
#include <SD.h>
#include <SerialFlash.h>
#include <SPI.h>
#include <LedControl.h>

/*
// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=105.00823974609375,238.01909637451172
AudioAmplifier           amp2; //xy=414.00823974609375,196.01910400390625
AudioAmplifier           amp1;           //xy=558.0082473754883,269.019136428833
AudioOutputI2S           i2s2;           //xy=653.008228302002,133.01910591125488
AudioConnection          patchCord1(i2s1, 0, amp2, 0);
AudioConnection          patchCord2(i2s1, 1, amp1, 0);
AudioConnection          patchCord3(amp2, 0, i2s2, 0);
AudioConnection          patchCord4(amp1, 0, i2s2, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=139.00823974609375,731.0191144943237
// GUItool: end automatically generated code
*/
/*
// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=105,238
AudioMixer4              mixer1;         //xy=232,493
AudioAnalyzeFFT256      fft1024_1;      //xy=471,535
AudioConnection          patchCord1(i2s1, 0, mixer1, 0);
AudioConnection          patchCord2(i2s1, 1, mixer1, 1);
AudioConnection          patchCord3(mixer1, fft1024_1);
AudioControlSGTL5000     sgtl5000_1;     //xy=139,731
// GUItool: end automatically generated code
*/

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=105,238
AudioMixer4              mixer1;         //xy=232,493
AudioFilterStateVariable filter1;        //xy=322,371
AudioAmplifier           amp2;           //xy=434.00000381469727,173.00000381469727
AudioAnalyzeFFT256       fft256_1;      //xy=471,535
AudioAmplifier           amp1;           //xy=555.9999961853027,327.00000858306885
AudioOutputI2S           i2s2;           //xy=653,133
AudioConnection          patchCord1(i2s1, 0, mixer1, 0);
AudioConnection          patchCord2(i2s1, 1, mixer1, 1);
AudioConnection          patchCord3(mixer1, 0, filter1, 0);
AudioConnection          patchCord4(mixer1, fft1024_1);
AudioConnection          patchCord5(filter1, 0, amp1, 0);
AudioConnection          patchCord6(filter1, 2, amp2, 0);
AudioConnection          patchCord7(amp2, 0, i2s2, 0);
AudioConnection          patchCord8(amp1, 0, i2s2, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=139,731
// GUItool: end automatically generated code




const int myInput = AUDIO_INPUT_LINEIN;


LedControl lc = LedControl(0,1,2,3);

void test_led_matrix() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      delay(1);
      lc.setLed(0, i, j, true);
    }
    for (int j = 0; j < 8; j++) {
      delay(1);
      lc.setLed(1, i, j, true);
    }
    for (int j = 0; j < 8; j++) {
      delay(1);
      lc.setLed(2, i, j, true);
    }
  }
}

void setup() {
  Serial.begin(9600);
  delay(3000);
  
  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(12);

  // Enable the audio shield, select input, and enable output
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.volume(0.5);

  lc.shutdown(0, false);
  lc.shutdown(1, false);
  lc.shutdown(2, false);

  lc.setIntensity(0, 8);
  lc.setIntensity(1, 8);
  lc.setIntensity(2, 8);

  lc.clearDisplay(0);
  lc.clearDisplay(1);
  lc.clearDisplay(2);

  test_led_matrix();

  while (!Serial);
  
  lc.clearDisplay(0);
  lc.clearDisplay(1);
  lc.clearDisplay(2);

  amp_dump_regs();
}

void amp_dump_regs() {
  int reg = 1;
  byte val = 0;

  Wire.begin();
  Wire.setSCL(19);
  Wire.setSDA(18);

  Serial.println("Amplifier registers:");
  for (reg = 1; reg < 8; reg++) {
    Wire.beginTransmission(0x58);
    Wire.write(reg);
    Wire.endTransmission();

    Wire.requestFrom(0x58, 1);
    while(Wire.available()) {
      val = Wire.read();
    }
    Serial.print("[");
    Serial.print(reg, HEX);
    Serial.print("] = ");
    Serial.println(val, HEX);
  }
}

//elapsedMillis volmsec=0;
float scalePotToGain(int potVal) {
  float lineOut = potVal / 1024.0;

  if (lineOut > 0.99)
    return 1.0;
  else if (lineOut < 0.01)
    return 0;
  else
    return lineOut;
}

int scalePotToFreq(int potVal) {
  double scale = potVal / 1024.0;
  if (scale > 1.0)
    scale = 1.0;
  else if (scale < 0)
    scale = 0;

  double freq = scale * scale * 3970.0 + 30.0;
  
  return (int)freq;
}

float scalePotToReso(int potVal) {
  float q = 0.7 + potVal * 4.3 / 1024.0;
  if (q > 5.0)
    q = 5.0;
  else if (q < 0.7)
    q = 0.7;
 
  return q;
}

void loop() {
  int potR, potL, potC, potLong;
  float gainR, gainL, q;
  int filterFreq;

  potR = analogRead(3);
  potL = analogRead(8);
  potC = analogRead(2);
  potLong = analogRead(0);

  gainR = scalePotToGain(potR);
  gainL = scalePotToGain(potL);
  q = scalePotToReso(potC);
  filterFreq = scalePotToFreq(potLong);

  amp1.gain(gainL);
  amp2.gain(gainR);
  filter1.frequency(filterFreq);
  filter1.resonance(q);

  lc.clearDisplay(0);
  lc.clearDisplay(1);
  lc.clearDisplay(2);
  float n;
  int mat, col;
  int hight;
  int i, j;
  if (fft256_1.available()) {
    for (mat = 0, col = 0; mat < 3; mat++) {
      for (i = 0; i < 8; i++, col++) {
        n = fft256_1.read(col);
        hight = 100.0 * n;
        for (j = 0; j < hight; j++) {
          lc.setLed(mat, 7 - j, i, true);
        }
      }
    }
  }

  delay(50);
}
