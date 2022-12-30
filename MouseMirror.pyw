import ctypes, time, pynput

user = ctypes.windll.user32
screen = user.GetSystemMetrics(0), user.GetSystemMetrics(1)
mouse = pynput.mouse.Controller()

def mirror():
    while True:
        try:
            if mouse.position[0] == 0:
                mouse.position = (screen[0] - 2, mouse.position[1])
            if mouse.position[0] == screen[0] - 1:
                mouse.position = (1, mouse.position[1])
            if mouse.position[1] == 0:
                mouse.position = (mouse.position[0], screen[1] - 2)
            if mouse.position[1] == screen[1] - 1:
                mouse.position = (mouse.position[0], 1)
            time.sleep(1/100)
        except:
            time.sleep(5)
            mirror()
mirror()