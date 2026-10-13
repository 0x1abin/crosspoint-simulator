#!/usr/bin/env python3
"""Check the production simulator held-time function on press/release frames."""
from pathlib import Path
import re
import subprocess
import tempfile
source = (Path(__file__).resolve().parents[1] / 'src/HalGPIO.cpp').read_text()
power_method = re.search(r'unsigned long HalGPIO::getPowerButtonHeldTime\(\) const \{.*?\n}', source, re.S).group()
method = re.search(r'unsigned long HalGPIO::getHeldTime\(\) const \{.*?\n}', source, re.S).group()
program = r'''
#include <cassert>
#include <cstdint>
#include <cstddef>
constexpr int NUM_BUTTONS=2;
uint8_t state[2]{}; bool syntheticButtonDown[2]{},coverButtonDown[2]{},releasedThisFrame[2]{};
bool available[2]{true,true}; bool buttonAvailable(int button){return available[button];}
int buttonScancode[2]{0,1}; unsigned long buttonPressTime[2]{100,0},buttonReleaseTime[2]{};
unsigned long now=100; unsigned long SDL_GetTicks(){return now;}
const uint8_t* SDL_GetKeyboardState(void*){return state;}
struct HalGPIO {static constexpr int BTN_POWER=0;unsigned long getHeldTime()const;unsigned long getPowerButtonHeldTime()const;};
@POWER@
@METHOD@
int main(){
 HalGPIO gpio;
 buttonPressTime[0]=100;buttonReleaseTime[0]=1300;now=2000;
 assert(gpio.getPowerButtonHeldTime()==1200);
 buttonPressTime[0]=2100;buttonReleaseTime[0]=0;now=2150;syntheticButtonDown[0]=true;
 assert(gpio.getPowerButtonHeldTime()==50);
 syntheticButtonDown[0]=false;buttonReleaseTime[0]=2180;assert(gpio.getPowerButtonHeldTime()==80);
 for(int source:{0,1,2}) {
  buttonPressTime[0]=100;now=500;state[0]=source==0;syntheticButtonDown[0]=source==1;coverButtonDown[0]=source==2;
  assert(gpio.getHeldTime()==400);
  now=1300;state[0]=0;syntheticButtonDown[0]=false;coverButtonDown[0]=false;releasedThisFrame[0]=true;
  assert(gpio.getHeldTime()==1200);
  releasedThisFrame[0]=false;assert(gpio.getHeldTime()==0);
  buttonPressTime[0]=1400;now=1480;releasedThisFrame[0]=true;
  assert(gpio.getHeldTime()==80);releasedThisFrame[0]=false;
 }
 available[0]=false;state[0]=1;now=1600;assert(gpio.getHeldTime()==0);
}
'''.replace('@METHOD@', method).replace('@POWER@', power_method).replace('#include <cassert>','#include <cassert>\n#include <initializer_list>')
with tempfile.TemporaryDirectory(prefix='sim-release-') as tmp:
    cpp=Path(tmp)/'check.cpp';binary=Path(tmp)/'check';cpp.write_text(program)
    subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
print('Physical/synthetic/cover button release duration, disabled inputs and next-contact reset passed')
