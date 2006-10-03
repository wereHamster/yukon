
import os

env = Environment(
	CC = 'gcc',
	CPPPATH = ['#include'],
	CCFLAGS = ['-std=c99', '-pipe', '-O3'],
)

objPatcher = env.Program('patcher', 'src/patcher.c')

env = env.Copy()
env.Append(LIBS = ['dl', 'pthread', 'FG', 'X13', 'seom'])
env.Append(LIBPATH = os.getenv('LIBPATH'))
objLibrary = env.SharedLibrary('yukon', 'src/yukon.c')

env.Default([ objPatcher ])
