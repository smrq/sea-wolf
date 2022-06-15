void setup() {
	pinMode(12, OUTPUT);
	pinMode(13, OUTPUT);
	pinMode(14, OUTPUT);
	analogWriteResolution(12);

	while (!Serial);
}

void loop() {
	if (Serial.available()) {
		char type = Serial.read();
		uint8_t port = Serial.read();

		switch (type) {
			case 'a': {
				float value;
				Serial.readBytes(&value, sizeof(value));
				uint8_t pin = 14;
				analogWrite(pin, (int)(value / 3.3f * (1<<12)));
				break;
			}
			case 'd': {
				bool value = Serial.readByte();
				uint8_t pin = port == 0 ? 12 : 13;
				digitalWrite(pin, value ? HIGH : LOW);
				break;
			}
		}
	}
}