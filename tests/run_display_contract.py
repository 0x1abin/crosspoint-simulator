#!/usr/bin/env python3
"""Exercise the real host display's capability, fallback and cancellation chain."""
from pathlib import Path
import shlex,subprocess,tempfile,sys
R=Path(__file__).resolve().parents[1]
PROJECT=Path(sys.argv[1])
with tempfile.TemporaryDirectory(prefix='sim-display-contract-') as temporary:
    root=Path(temporary);source=root/'test.cpp';binary=root/'test'
    source.write_text('''#include <HalDisplay.h>
#include <GfxRenderer.h>
#include <cassert>
GfxRenderer renderer;ESPMock ESP;
int main(){
 if(BoardConfig::isReadPico()) {
  assert(display.grayscaleCapabilities().supported());
  assert(!display.grayscaleCapabilities(HalDisplay::GrayscaleMode::Absolute).supported());
  assert(!display.grayscaleCapabilities(HalDisplay::GrayscaleMode::Direct).supported());
 }else if(BoardConfig::isEegoA4()){
  assert(display.grayscaleCapabilities(HalDisplay::GrayscaleMode::Absolute).base == HalDisplay::GrayscaleBase::Combined);
  assert(display.grayscaleCapabilities(HalDisplay::GrayscaleMode::Direct).base == HalDisplay::GrayscaleBase::Combined);
 }else{
  BoardConfig::ACTIVE.displayController=HalDisplay::Controller::UC8279;
  BoardConfig::ACTIVE.displayControllerVariant=0x67;
  assert(!display.grayscaleCapabilities().supported());
  assert(!display.displayGrayscaleBase(HalDisplay::GrayscaleMode::Overlay));
  display.displayGrayscaleBase();assert(display.displayCommitted());
  display.cancelGrayscale();
  display.copyGrayscaleLsbBuffers(display.getFrameBuffer());display.displayGrayBuffer();
  BoardConfig::ACTIVE.displayControllerVariant=0x68;
 }
 display.displayGrayscaleBase();
 assert(display.displayCommitted()==!display.combinesGrayscaleBase());
 display.cancelGrayscale();display.cleanupGrayscaleBuffers(display.getFrameBuffer());
 if(display.combinesGrayscaleBase())assert(!display.displayCommitted());
 display.displayGrayscaleBase();display.cleanupGrayscaleBuffers(display.getFrameBuffer());
 assert(display.displayCommitted());
 display.displayGrayscaleBase();display.displayGrayBuffer();assert(display.displayCommitted());
}
''')
    for profile in ['DEFAULT','SIMULATOR_DEVICE_EEGO_A4','SIMULATOR_DEVICE_READPICO']:
        command=['c++','-std=c++20','-DSIMULATOR','-DCROSSPOINT_EMULATED=1','-I'+str(R/'tests/display_stubs'),'-I'+str(R/'src'),'-I'+str(PROJECT/'lib/Memory'),str(source),str(R/'src/HalDisplay.cpp'),str(R/'src/HalGPIO.cpp'),str(R/'src/SimulatorLifecycle.cpp'),*shlex.split(subprocess.check_output(['sdl2-config','--cflags','--libs'],text=True)),'-pthread','-o',str(binary)]
        if profile!='DEFAULT':command.append('-D'+profile)
        subprocess.run(command,check=True);subprocess.run([str(binary)],check=True)
print('Real display capabilities, deferred base, commit, cancellation, B/W fallback and unsupported UC8279 passed')

# Include the real implementation in the EEGO fixture to inspect its composed pixels without a new public HAL API.
with tempfile.TemporaryDirectory(prefix='sim-eego-display-') as temporary:
    binary=Path(temporary)/'eego'
    command=['c++','-std=c++20','-DSIMULATOR','-DCROSSPOINT_EMULATED=1','-DSIMULATOR_DEVICE_EEGO_A4',
             '-I'+str(R/'tests/display_stubs'),'-I'+str(R/'src'),'-I'+str(PROJECT/'lib/Memory'),
             str(R/'tests/eego_display_self_test.cpp'),str(R/'src/HalGPIO.cpp'),str(R/'src/SimulatorLifecycle.cpp'),
             *shlex.split(subprocess.check_output(['sdl2-config','--cflags','--libs'],text=True)),'-pthread','-o',str(binary)]
    subprocess.run(command,check=True);subprocess.run([str(binary)],check=True)
