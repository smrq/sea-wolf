import mido
import struct
import serial

def serialize_analog(out, value):
	return struct.pack('<cBf', 'a', out, value)

def serialize_digital(out value):
	return struct.pack('<cB?', 'd', out, value)

with serial.Serial('/dev/ttyACM0', baudrate=1000000) as serial_out:
	with mido.open_input() as midi_in:
		for msg in midi_in:
			print(msg)
			serialized = serialize_digital(msg.channel, msg.type == 'note_on')
			print(serialized)
			serial_out.write(serialized)
