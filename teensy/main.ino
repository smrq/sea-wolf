#include <SPI.h>

const uint8_t digital_out[] = { 3, 4, 5, 6, 7, 8 };
const uint8_t analog_out_cs[] = { 0, 0, 1, 1, 2, 2 };
const uint8_t analog_out_ab[] = { 0, 1, 0, 1, 0, 1 };

void setup() {
	SPI.begin();
	while (!Serial);
}

uint16_t float_to_uint12(float value, float min_value, float max_value) {
	return (uint16_t)((value-min_value) / (max_value-min_value) * (1<<12));
}

void write_dac(uint8_t cs, uint8_t ab, float value) {
	uint16_t value_bits = float_to_uint12(value, 0f, 10f)
		| (ab << 15) // DAC_A (0) or DAC_B (1)
		| (1<<13)    // Gain = 1x
		| (1<<12);   // Output enabled
	SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
	digitalWrite(cs, LOW);
	SPI.transfer16(value_bits);
	digitalWrite(cs, HIGH);
}

void loop() {
	if (Serial.available()) {
		char type;
		while ((type = Serial.read()) != 0xFF) {
			uint8_t out = Serial.read();
			switch (type) {
				case 'a': {
					float value;
					Serial.readBytes(&value, sizeof(value));
					uint8_t cs = analog_out_cs[out];
					uint8_t ab = analog_out_ab[out];
					write_dac(cs, ab, value);
					break;
				}
				case 'd': {
					bool value = Serial.readByte();
					uint8_t pin = digital_out[out];
					digitalWrite(pin, value ? HIGH : LOW);
					break;
				}
			}
		}
	}
}

