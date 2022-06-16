import mido
import struct
import serial

def serialize(commands):
	return b''.join(serialize_one(cmd) for cmd in commands) + b'\xFF'

def serialize_one(command):
	if command['type'] == 'analog':
		return struct.pack('<cBf', b'a', command['pin'], command['value'])
	if command['type'] == 'digital':
		return struct.pack('<cB?', b'd', command['pin'], command['value'])
	raise ValueError('invalid command type')

def midi_note_to_cv(note):
	return note * (1/12)

def open_serial_with_fallback(tty_name):
	try:
		return serial.Serial(tty_name, baudrate=1000000)
	except serial.serialutil.SerialException as err:
		print(err)
		print('Running in disconnected mode.')
		return SerialShim(tty_name)

class SerialShim:
	def __init__(self, tty_name):
		self.tty_name = tty_name

	def write(self, msg):
		print('{} << {}'.format(self.tty_name, msg.hex(' ')))

	def close(self):
		pass

class MidiCV:
	def __init__(self, tty_name):
		self.tty_name = tty_name
		self.channels = [
			{ 'note_pin': 12, 'gate_pin': 13, 'notes': [] },
			{ 'note_pin': 14, 'gate_pin': 15, 'notes': [] },
		]
		self.poly_mode = 'last'

	def __enter__(self):
		self.midi_in = mido.open_input()
		self.serial_out = open_serial_with_fallback(self.tty_name)
		return self

	def __exit__(self, *args, **kwargs):
		self.midi_in.close()
		self.serial_out.close()

	def listen(self):
		for msg in self.midi_in:
			print(msg)

			if msg.type == 'note_on':
				try:
					channel = self.channels[msg.channel]
				except IndexError:
					continue

				channel['notes'].append(msg.note)
				commands = [
					{ 'type': 'analog', 'pin': channel['note_pin'], 'value': midi_note_to_cv(msg.note) },
					{ 'type': 'digital', 'pin': channel['gate_pin'], 'value': True },
				]
				self.serial_out.write(serialize(commands))

			elif msg.type == 'note_off':
				try:
					channel = self.channels[msg.channel]
				except IndexError:
					continue

				try:
					channel['notes'].remove(msg.note)
				except ValueError:
					continue

				if len(channel['notes']) == 0:
					commands = [
						{ 'type': 'digital', 'pin': channel['gate_pin'], 'value': False },
					]
					self.serial_out.write(serialize(commands))
				else:
					note = channel['notes'][-1]
					commands = [
						{ 'type': 'analog', 'pin': channel['note_pin'], 'value': midi_note_to_cv(note) },
						{ 'type': 'digital', 'pin': channel['gate_pin'], 'value': True },
					]
					self.serial_out.write(serialize(commands))

with MidiCV('/dev/ttyACM0') as midicv:
	midicv.listen()

