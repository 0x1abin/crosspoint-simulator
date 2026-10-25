#!/usr/bin/env python3
"""Compare the native display metadata directly with compiled SDK profiles."""
import json,os,pathlib,subprocess,sys,tempfile
ROOT=pathlib.Path(__file__).resolve().parents[1]
SDK=pathlib.Path(sys.argv[1]).resolve()
PROFILES=[('X4',None),('X3','X3'),('X4PRO','X4_PRO'),('X4CLASSIC','X4_CLASSIC'),('STICKY','STICKY'),('PAPERMONO','PAPERMONO'),('EEGO_A4','EEGO_A4'),('MURPHY_M4','MURPHY_M4'),('METALIO_EINK4','METALIO_EINK4'),('READPICO','READPICO'),('PICCO','PICCO')]
BODY=r'''
#include <cstdio>
#include <BoardConfig.h>
int main(){const auto& p=BoardConfig::ACTIVE;
 printf("%u %u %u %u %u %u %u %u %d %u %d %u %u %u %d %u %u\n",
 unsigned(p.displayWidth),unsigned(p.displayHeight),unsigned(p.displayController),unsigned(p.displayControllerVariant),
 unsigned(p.viewableInsets.top),unsigned(p.viewableInsets.right),unsigned(p.viewableInsets.bottom),unsigned(p.viewableInsets.left),
 int(p.headerOffsetY),unsigned(p.cornerRadius),int(p.tabBarOffsetY),unsigned(p.tabBarHeightPct),unsigned(p.tabBarWidthPct),
 unsigned(p.wifiMaxTxPower),int(p.statusBarOffsetY),unsigned(p.bottomCornerRadius),unsigned(p.grayscaleLevels));}
'''
with tempfile.TemporaryDirectory(prefix='sdk-simulator-profile-') as directory:
 folder=pathlib.Path(directory)
 sim=folder/'sim.cpp';sim.write_text(BODY)
 sdk=folder/'sdk.cpp';sdk.write_text('#include <Arduino.h>\nusing HardwareSerial = SerialStub;\ninline HardwareSerial Serial0;\n'+BODY)
 result={}
 for sdk_flag,sim_flag in PROFILES:
  values=[]
  for kind,src,includes,flags in [('sdk',sdk,[SDK/'libs/hardware/InputManager/test/host/metalio_stubs',SDK/'libs/hardware/BoardConfig/include'],['-DFREEINK_DEVICE_'+sdk_flag+'=1']),('sim',sim,[ROOT/'src'],['-DSIMULATOR_DEVICE_'+sim_flag] if sim_flag else [])]:
   binary=folder/kind
   subprocess.run([os.environ.get('CXX','c++'),'-std=c++17',*[f'-I{x}' for x in includes],*flags,str(src),'-o',str(binary)],check=True)
   values.append(subprocess.check_output([str(binary)],text=True).strip())
  result[sdk_flag]={'sdk':values[0],'simulator':values[1],'equal':values[0]==values[1]}
 print(json.dumps(result,indent=2))
 if not all(v['equal'] for v in result.values()):raise SystemExit(1)
print('All11 native display profiles match the actual reviewed SDK fields')
