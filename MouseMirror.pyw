import ctypes, time;
from ctypes import wintypes;

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

user = ctypes.windll.user32
ctypes.windll.shcore.SetProcessDpiAwareness(2)

class RECT(ctypes.Structure):
    _fields_ = [
        ("x", wintypes.LONG),
        ("y", wintypes.LONG),
        ("w", wintypes.LONG),
        ("h", wintypes.LONG),
    ]

monitors: list[RECT] = []

MONITORENUMPROC = ctypes.WINFUNCTYPE(
    wintypes.BOOL,
    wintypes.HMONITOR,
    wintypes.HDC,
    ctypes.POINTER(RECT),
    wintypes.LPARAM
)

@MONITORENUMPROC
def monitor_enum_proc(hMonitor, hdcMonitor, lprcMonitor, dwData):
    rect = lprcMonitor.contents
    monitors.append(RECT(rect.x, rect.y, rect.w, rect.h))
    return True

user.EnumDisplayMonitors(0, 0, monitor_enum_proc, 0)

mouse = pynput.mouse.Controller();
global pressCount;
pressCount = [0];

def appendPresses(x, y, button, isPressed):
    if isPressed:
        pressCount[0] += 1;
    else:
        pressCount[0] -= 1;

def sayCoords(x, v):
    print(x, y)

mouseEar = pynput.mouse.Listener(on_click=appendPresses);
mouseEar.start();

def getScreenMerge(monitors):
    mergeInt = []

    for i, mon in enumerate(monitors):
        if i < len(monitors) - 1:
            next_mon = monitors[i + 1]
            if mon.w == next_mon.x:
                y_overlap_start = max(mon.y, next_mon.y)
                y_overlap_end = min(mon.h, next_mon.h)
                if y_overlap_start < y_overlap_end:
                    mergeInt.append(((mon.w - 1, y_overlap_start), (mon.w - 1, y_overlap_end)))
                    mergeInt.append(((next_mon.x, y_overlap_start), (next_mon.x, y_overlap_end)))
    return mergeInt

def rayCast(monitors, x, y, dir) -> tuple[int, int]:
    fx = None
    fy = None
    if dir == 'l':
        fy = y
        for mon in monitors:
            if not (mon.y <= fy < mon.h):
                continue
            if fx is None or mon.w > fx:
                fx = mon.w - 2
    elif dir == 'r':
        fy = y
        for mon in monitors:
            if not (mon.y <= fy < mon.h):
                continue
            if fx is None or mon.x < fx:
                fx = mon.x + 1
    elif dir == 'u':
        fx = x
        for mon in monitors:
            if not (mon.x <= fx < mon.w):
                continue
            if fy is None or mon.h > fy:
                fy = mon.h - 2
    elif dir == 'd':
        fx = x
        for mon in monitors:
            if not (mon.x <= fx < mon.w):
                continue
            if fy is None or mon.y < fy:
                fy = mon.y + 1
    return (fx, fy)

def outerScreen(monitors, x, y) -> RECT:
    for mon in monitors:
        if mon.x <= x < mon.w and mon.y <= y < mon.h:
            return mon
    
def getCollSide(mon, x, y) -> str:
    dir = ''
    if x == mon.x:
        dir = 'l'
    elif x == mon.w - 1:
        dir = 'r'
    elif y == mon.y:
        dir = 'u'
    elif y == mon.h - 1:
        dir = 'd'
    return dir

def atMerge(merges, x, y):
    for merge in merges:
        if merge[0][0] <= x <= merge[1][0] and merge[0][1] <= y <= merge[1][1]:
            return True
    return False

merges = getScreenMerge(monitors)
print("Found screen merges:", merges)

while True:
    if pressCount[0] == 0:
        x: int = mouse.position[0];
        y: int = mouse.position[1];
        if not atMerge(merges, x, y):
            dir = getCollSide(outerScreen(monitors, x, y), x, y)
            if dir:
                res = rayCast(monitors, x, y, dir);
                mouse.move(res[0] - x, res[1] - y);
    time.sleep(0.01);
