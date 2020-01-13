import os

env=Environment(ENV=os.environ, tools=['default', 'protoc', 'grpc'])

print(env['ENV']['HOME'])
print(env['ENV']['PATH'])

if 'CC' in os.environ:
    env['CC']=os.environ['CC']
    
if 'CXX' in os.environ:
    env['CXX']=os.environ['CXX']

root_dir = Dir('#').srcnode().abspath

config = {}

config['cryto_lib_dir'] = root_dir + "/third_party/crypto/library"
config['cryto_include'] = config['cryto_lib_dir']  + "/include"
config['cryto_lib'] = config['cryto_lib_dir']  + "/lib"

config['db-parser_lib_dir'] = root_dir + "/third_party/db-parser/library"
config['db-parser_include'] = config['db-parser_lib_dir']  + "/include"
config['db-parser_lib'] = config['db-parser_lib_dir']  + "/lib"


if 'config_file' in ARGUMENTS:
    SConscript(ARGUMENTS['config_file'], exports=['env','config'])



env.Append(CCFLAGS = ['-fPIC','-Wall', '-march=native'])
env.Append(CXXFLAGS = ['-std=c++14'])
env.Append(CPPPATH = ['/usr/local/include', config['cryto_include'], config['db-parser_include'],'/usr/local/include/cryptopp'])
env.Append(LIBPATH = ['/usr/local/lib', config['cryto_lib'], config['db-parser_lib']])
env.Append(RPATH = [config['cryto_lib'], config['db-parser_lib']])

env.Append(LIBS = ['crypto', 'sse_crypto', 'grpc++_unsecure', 'grpc', 'protobuf', 'pthread', 'dl', 'sse_dbparser', 'rocksdb', 'snappy', 'z', 'bz2',  'lz4','cryptopp'])
  
env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME']=1

env.Append(CCFLAGS = ['-g','-O0'])

static_relic = ARGUMENTS.get('static_relic', 0)

env.Append(CPPDEFINES = ['BENCHMARK'])

def run_test(target, source, env):
    app = str(source[0].abspath)
    if os.spawnl(os.P_WAIT, app, app)==0:
        return 0
    else:
        return 1

bld = Builder(action = run_test)
env.Append(BUILDERS = {'Test' :  bld})


crypto_lib_target = env.Command(config['cryto_lib_dir'], "", "cd third_party/crypto && scons lib static_relic={0}".format(static_relic))
db_parser_target = env.Command(config['db-parser_lib_dir'], "", "cd third_party/db-parser && scons lib")
env.Alias('deps', [crypto_lib_target, db_parser_target])

objects = SConscript('src/build.scons', exports='env', variant_dir='build')

env.Depends(objects["mitra"],[crypto_lib_target , db_parser_target])
env.Depends(objects["static"],[crypto_lib_target , db_parser_target])
env.Depends(objects["horus"],[crypto_lib_target , db_parser_target])
env.Depends(objects["qos"],[crypto_lib_target , db_parser_target])


Clean(objects["horus"]+objects["static"]+objects["mitra"]+objects["qos"] , 'build')

outter_env = env.Clone()
outter_env.Append(CPPPATH = ['build'])

qos_debug_prog   = outter_env.Program('qos_debug',    ['test_qos.cpp']     + objects["qos"])

mitra_debug_prog   = outter_env.Program('mitra_debug',    ['test_mitra.cpp']     + objects["mitra"])

static_amortized_debug_prog   = outter_env.Program('static_amortized_debug',    ['test_static_amortized.cpp']     + objects["static"])
static_deamortized_debug_prog   = outter_env.Program('static_deamortized_debug',    ['test_static_deamortized.cpp']     + objects["static"])
horus_debug_prog   = outter_env.Program('horus_debug',    ['test_horus.cpp']     + objects["horus"])

env.Alias('mitra', [mitra_debug_prog])
env.Alias('static', [static_amortized_debug_prog])
env.Alias('static', [static_deamortized_debug_prog])
env.Alias('qos', [qos_debug_prog])
env.Alias('horus', [horus_debug_prog])
env.Default(['horus','static','mitra','qos'])
