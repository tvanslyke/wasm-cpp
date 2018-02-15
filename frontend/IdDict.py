from collections import OrderedDict


class IdDict(OrderedDict):
	
	def __init__(self, *args, **kwargs):
		super(IdDict, self).__init__(*args, **kwargs)

	def __getitem__(self, key):
		try:
			return OrderedDict.__getitem__(self, key)
		except KeyError:
			return self.__missing__(key)
			
	def __missing__(self, key):
		sz = len(self)
		self[key] = sz
		return sz

	def copy(self):
		return self.__copy__()

	def __setitem__(self, key, value):
		return NotImplemented
	
	def fromkeys(self, *args, **kwargs):
		return NotImplemented
	
	def update(self, keys):
		for k in keys:
			self[k]
	
	
