#!/usr/bin/env python3
"""Check the production simulator held-time function on press/release frames."""
from pathlib import Path
import re
import subprocess
import tempfile
source = (Path(__file__).resolve().parents[1] / 'src/HalGPIO.cpp').read_text()
method = re.search(r'unsigned long HalGPIO::getHeldTime\(\) const \{.*?\n}', source, re.S).group()
program = r'''
#include <cassert>
#include <cstdint>
#include <cstddef>
constexpr int NUM_BUTTONS=2;
uint8_t state[2]{}; bool syntheticButtonDown[2]{},releasedThisFrame[2]{};
int buttonScancode[2]{0,1}; unsigned long buttonPressTime[2]{100,0};
unsigned long now=100; unsigned long SDL_GetTicks(){return now;}
const uint8_t* SDL_GetKeyboardState(void*){return state;}
struct HalGPIO {unsigned long getHeldTime()const;};
@METHOD@
int main(){
 HalGPIO gpio;
 for(bool synthetic:{false,true}) {
  buttonPressTime[0]=100;now=500;state[0]=!synthetic;syntheticButtonDown[0]=synthetic;
  assert(gpio.getHeldTime()==400);
  now=1300;state[0]=0;syntheticButtonDown[0]=false;releasedThisFrame[0]=true;
  assert(gpio.getHeldTime()==1200);
  releasedThisFrame[0]=false;assert(gpio.getHeldTime()==0);
  buttonPressTime[0]=1400;now=1480;releasedThisFrame[0]=true;
  assert(gpio.getHeldTime()==80);releasedThisFrame[0]=false;
 }
}
'''.replace('@METHOD@', method).replace('#include <cassert>','#include <cassert>\n#include <initializer_list>')
with tempfile.TemporaryDirectory(prefix='sim-release-') as tmp:
    cpp=Path(tmp)/'check.cpp';binary=Path(tmp)/'check';cpp.write_text(program)
    subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
print('Physical/synthetic button release duration and next-contact reset passed')
