#include"TFT_eSPI.h"
#include"Seeed_FS.h"
#include"RawImage.h"
#include <project_52099_inferencing.h>
#include"LIS3DHTR.h"

TFT_eSPI tft;
LIS3DHTR<TwoWire> lis;
#define CONVERT_G_TO_MS2    9.80665f
ei_impulse_result_classification_t currentClassification[EI_CLASSIFIER_LABEL_COUNT];
const char* maxConfidenceLabel;

void runClassifier()
{
  float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };
  for (size_t ix = 0; ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ix += 3) {
    uint64_t next_tick = micros() + (EI_CLASSIFIER_INTERVAL_MS * 1000);
    lis.getAcceleration(&buffer[ix], &buffer[ix + 1], &buffer[ix + 2]);
    buffer[ix + 0] *= CONVERT_G_TO_MS2;
    buffer[ix + 1] *= CONVERT_G_TO_MS2;
    buffer[ix + 2] *= CONVERT_G_TO_MS2;
    delayMicroseconds(next_tick - micros());
  }
  signal_t signal;
  int err = numpy:: signal_from_buffer(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
  ei_impulse_result_t result = { 0 };

  err = run_classifier(&signal, &result, false);
  float maxValue = 0;
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    ei_impulse_result_classification_t classification_t = result.classification[ix];
    ei_printf("    %s: %.5f\n", classification_t.label, classification_t.value);
    float value = classification_t.value;
    if (value > maxValue) {
      maxValue = value;
      maxConfidenceLabel = classification_t.label;
    }
    currentClassification[ix] = classification_t;
  }
}
float getLabelConfidence(char *label)
{
  float value = 0;
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    ei_impulse_result_classification_t classification = currentClassification[ix];
    if (label == classification.label) {
      value = classification.value;
    }
  }
  return value;
}

void setup(){
  tft.begin();
   if (!SD.begin(SDCARD_SS_PIN, SDCARD_SPI)) {
        while (1);
    }
  lis.begin(Wire1);
  lis.setOutputDataRate(LIS3DHTR_DATARATE_100HZ);
  lis.setFullScaleRange(LIS3DHTR_RANGE_4G);

  Serial.begin(115200);
  tft.setRotation(3);
  tft.fillScreen(0xFFFF);
  tft.setTextSize(2);
  drawImage<uint8_t>("logo.bmp", 0,0);

}



void loop(){

  Serial.println("entro 1");
  runClassifier();
  Serial.println("entro 2");
  if ((getLabelConfidence("square") > 0.6)) {
    Serial.println("square");
    tft.fillScreen(0xFFFF);
    drawImage<uint8_t>("star.bmp", 0,0);
  } else {
    if ((getLabelConfidence("triangle") > 0.6)) {
      Serial.println("triangle");
      tft.fillScreen(0xFFFF);
      drawImage<uint8_t>("triangle.bmp", 0,0);
    } else {
      if ((getLabelConfidence("circle") > 0.6)) {
        Serial.println("circle");
        tft.fillScreen(0xFFFF);
        drawImage<uint8_t>("circle.bmp", 0,0);
      } else {
        if ((getLabelConfidence("umbrella") > 0.6)) {
          Serial.println("umbrella");
          tft.fillScreen(0xFFFF);
          drawImage<uint8_t>("umbrella.bmp", 0,0);
        } else {
          Serial.println("main");
          tft.fillScreen(0xFFFF);
          drawImage<uint8_t>("logo.bmp", 0,0);
        }
      }
    }
  }

}
