const int dac_pins[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
const int cs_pins[] = { 12, 13 };
const int analog_pin = 14;

void setup() {
	for (int i = 0; i < 12; ++i) {
		pinMode(dac_pins[i], OUTPUT);
	}
	for (int i = 0; i < sizeof(cs_pins); ++i) {
		pinMode(cs_pins[i], OUTPUT);
	}
	pinMode(analogPin, OUTPUT);

	analogWriteResolution(12);

	while (!Serial);
}

uint16_t float_to_uint12(float value, float min_value, float max_value) {
	return (uint16_t)((value-min_value) / (max_value-min_value) * (1<<12));
}

void write_analog(int pin, float value) {
	uint16_t value_bits = float_to_uint12(value, 0f, 3.3f);
	if (pin == analog_pin) {
		analogWrite(pin, value_bits);
	} else {
		digitalWrite(pin, LOW);
		for (int i = 0; i < 12; ++i) {
			digitalWrite(dac_pins[i], value_bits & (1<<i) ? HIGH : LOW);
		}
		digitalWrite(pin, HIGH);
	}
}

void loop() {
	if (Serial.available()) {
		char type;
		while ((type = Serial.read()) != 0xFF) {
			uint8_t pin = Serial.read();
			switch (type) {
				case 'a': {
					float value;
					Serial.readBytes(&value, sizeof(value));
					write_analog(pin, value);
					break;
				}
				case 'd': {
					bool value = Serial.readByte();
					digitalWrite(pin, value ? HIGH : LOW);
					break;
				}
			}
		}
	}
}

