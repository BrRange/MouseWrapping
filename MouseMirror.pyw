import ctypes, time;

goodToGo = True;
async def importCheck():
    import pip;
    await pip.main(['install', 'pynput']);
    goodToGo = True;
try:
    import pynput;
except ModuleNotFoundError:
    goodToGo = False;
    importCheck();
    while not goodToGo:
        time.sleep(5);
    import pynput;

user = ctypes.windll.user32;
screen = user.GetSystemMetrics(0), user.GetSystemMetrics(1);
mouse = pynput.mouse.Controller();
global pressCount;
pressCount = [0];

def appendPresses(x, y, button, isPressed):
    if isPressed:
        pressCount[0] += 1;
    else:
        pressCount[0] -= 1;

mouseEar = pynput.mouse.Listener(on_click=appendPresses);
mouseEar.start();

while True:
    x: int = mouse.position[0];
    y: int = mouse.position[1];
    if x == 0 and pressCount[0] == 0: mouse.position = (screen[0] - 2, y);
    elif x == screen[0] - 1 and pressCount[0] == 0: mouse.position = (1, y);
    if y == 0 and pressCount[0] == 0: mouse.position = (x, screen[1] - 2);
    elif y == screen[1] - 1 and pressCount[0] == 0: mouse.position = (x, 1);
    time.sleep(0.01);
