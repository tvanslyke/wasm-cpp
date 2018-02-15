import leb128
from tqdm import tqdm
def leb128_uint_encode(num):
	enc = []
	if num == 0:
		enc = [0]
	while 1:
		bt = (num & 0b01111111)
		num >>= 7
		if num != 0:
			bt |= 0b10000000
		enc.append(bt)
		if num == 0:
			break
	return bytes(bytearray(enc))

def twoscomp(num):
	bitstr = bin(num)[3:] if num < 0 else bin(num)[2:]
	bitstr = bitstr[::-1]
	bitstr = ['0' if char == '1' else '1' for char in bitstr]
	idx = bitstr.find('0')
	if idx > 0:
		bitstr = '0' * idx + '1' + bitstr[idx + 1:]
		return bitstr[::-1]
	else:
		return '1' + '0' * len(bitstr)

def leb128_sint_encode(num):
	enc = []
	is_negative = num < 0
	size = num.bit_length()
	more = True
	while more:
		byte = num & 0b0111_1111
		num >>= 7
		# if is_negative:
		# 	num |= (0x0111_1111) << (size - 7)
		done = (num == 0) and (not bool(byte & 0b0100_0000)) 
		done = done or ((num == -1) and (bool(byte & 0b0100_0000)))
		if done:
			more = False
		else:
			byte |= 0b1000_0000
		enc.append(byte)
	return bytes(bytearray(enc))
	



for i in tqdm(range(-100_000_000, 0)):
	enc = leb128_sint_encode(i)
	dec = leb128.read_signed(enc, 32)[0]
	if i != dec:
		print(i, dec)
		assert False


