#define LED_PIN 7
#define GUITAR_INPUT A0
#define SAMPLE_RATE 8000 // 8 kHz
#define BUFFER_SIZE_B 1024 // In bytes.
#define BUFFER_SIZE_S 512 // In samples.
#define DISTORTION_CONSTANT 450.0
#define DISTORTION_POT_OUTPUT 2
#define DISTORTION_POT A2
#define VOLUME_POT_OUTPUT 3
#define VOLUME_POT A3

// Audio I/O controls
uint16_t* audioBuffer = nullptr;
uint64_t renderCursor = 0u;
uint64_t writeCursor = 0u;
uint16_t renderedSample = 0u;
// Sample rate controls
uint16_t renderedSampleCount = 0u;
uint64_t elapsedTime = 0u;
// Distortion
uint16_t lowerThreshold = 0u;
uint16_t upperThreshold = 0u;

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  pinMode(DISTORTION_POT_OUTPUT, OUTPUT);
  pinMode(VOLUME_POT_OUTPUT, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(DISTORTION_POT_OUTPUT, HIGH);
  digitalWrite(VOLUME_POT_OUTPUT, HIGH);
  SetPWMRes10();
  audioBuffer = (uint16_t*)malloc(BUFFER_SIZE_B);
  if (audioBuffer != nullptr) 
  {
    memset(audioBuffer, 0, BUFFER_SIZE_B);
  }
}
void loop() 
{
  AddToBuffer(analogRead(GUITAR_INPUT));
  if (millis() - elapsedTime >= 1000)
  {
    elapsedTime = millis();
    renderedSampleCount = 0u;
  }
  Render();
}
void SetPWMRes10() 
{
  DDRB |= _BV(PB1); // Set 9th pin as output
  TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11); // Mode 14: Fast PWM, TOP=ICR1
  TCCR1B = _BV(WGM13) | _BV(WGM12)| _BV(CS10); // Prescaler 1
  ICR1 = 0x03FF; // TOP counter value (Relieving OCR1A*)
}
void AddToBuffer(uint16_t sample) 
{
  *(audioBuffer + (writeCursor % BUFFER_SIZE_S)) = sample;
  writeCursor++;
}
void Render()
{
  if (renderedSampleCount < SAMPLE_RATE && writeCursor > renderCursor)
  {
    renderedSample = *(audioBuffer + (renderCursor % BUFFER_SIZE_S));    
    Distort(renderedSample, analogRead(DISTORTION_POT));
    renderedSample *= (double)analogRead(VOLUME_POT) / 1023.0;
    OCR1A  = renderedSample; // 9th pin
    renderedSampleCount++;
    renderCursor++;
  }
}
void Distort(uint16_t& sample, double distortionPotValue)
{
  lowerThreshold = DISTORTION_CONSTANT * distortionPotValue / 1023.0;
  upperThreshold = 1023 - lowerThreshold;
  if (sample > upperThreshold)
  {
    sample = upperThreshold;
  }
  else if (sample < lowerThreshold)
  {
    sample = lowerThreshold;
  }
}
