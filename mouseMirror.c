#include <windows.h>
#include <time.h>
#include <stdio.h>

typedef struct {
  LONG x, y, w, h;
} Rect;

typedef struct {
  POINT start, end;
} Merge;

Rect *monitors = 0;
Merge *merges = 0;
int monitorCount = 0, mergeCount = 0;

/* Leave for now
HICON iconHolder;
WNDCLASS wc = {};
HWND ghostWindow;
NOTIFYICONDATA nData = {};

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
    case WM_APP + 1:
    if (lParam == WM_RBUTTONUP) {
      POINT pt;
      GetCursorPos(&pt);
      HMENU menu = CreatePopupMenu();
      AppendMenu(menu, MF_STRING, 101, "Stop");
      SetForegroundWindow(hwnd);
      TrackPopupMenu(menu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
      DestroyMenu(menu);
    }
    break;
    case WM_COMMAND:
    if (LOWORD(wParam) == 1001) {
      Shell_NotifyIcon(NIM_DELETE, &nData);
      PostQuitMessage(0);
    }
    break;
    case WM_DESTROY:
    PostQuitMessage(0);
    break;
    default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}

void getTrayIcon(){
  HINSTANCE Instance = GetModuleHandle(0);
  iconHolder = LoadIcon(Instance, MAKEINTRESOURCE(101));
  wc.lpfnWndProc = WndProc;
  wc.hInstance = Instance;
  wc.lpszClassName = "MouseMirrorWindowClass";
  wc.style = CS_OWNDC;
  RegisterClass(&wc);
  
  ghostWindow = CreateWindow(wc.lpszClassName, "", WS_OVERLAPPED,
  0, 0, 0, 0,
  0, 0, Instance, 0);
  nData.cbSize = sizeof(NOTIFYICONDATA);
  nData.hWnd = ghostWindow;
  nData.uID = 1;
  nData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  nData.uCallbackMessage = WM_APP + 1;
  nData.hIcon = iconHolder;
  strcpy(nData.szTip, "Mouse Mirror");
  
  Shell_NotifyIcon(NIM_ADD, &nData);
}
*/

BOOL CALLBACK MonitorEnumProc(HMONITOR hMon, HDC hdc, LPRECT lprc, LPARAM data) {
  monitors = (Rect *)realloc(monitors, sizeof(Rect) * (monitorCount + 1));
  monitors[monitorCount].x = lprc->left;
  monitors[monitorCount].y = lprc->top;
  monitors[monitorCount].w = lprc->right;
  monitors[monitorCount].h = lprc->bottom;
  monitorCount++;
  return TRUE;
}

void getScreenMerge(Rect *mons) {
  for (int i = 0; i < monitorCount - 1; i++)
    for (int j = i + 1; j < monitorCount; j++) {
      Rect mon = mons[i];
      Rect next_mon = mons[j];
      if (mon.w == next_mon.x || mon.x == next_mon.w) {
        long y_overlap_start = max(mon.y, next_mon.y);
        long y_overlap_end = min(mon.h, next_mon.h);
        if (y_overlap_start <= y_overlap_end) {
          merges = realloc(merges, (mergeCount + 2) * sizeof(Merge));
          if (mon.w == next_mon.x){
            merges[mergeCount] = (Merge) {mon.w - 1, y_overlap_start, mon.w - 1, y_overlap_end};
            merges[mergeCount + 1] = (Merge) {next_mon.x, y_overlap_start, next_mon.x, y_overlap_end};
          } else {
            merges[mergeCount] = (Merge) {mon.x, y_overlap_start, mon.x, y_overlap_end};
            merges[mergeCount + 1] = (Merge) {next_mon.w - 1, y_overlap_start, next_mon.w - 1, y_overlap_end};
          }
          mergeCount += 2;
        }
      }
      else if (mon.h == next_mon.y || mon.y == next_mon.h) {
        long x_overlap_start = min(mon.x, next_mon.x);
        long x_overlap_end = max(mon.w, next_mon.w);
        if (x_overlap_start <= x_overlap_end) {
          merges = realloc(merges, (mergeCount + 2) * sizeof(Merge));
          if (mon.h == next_mon.y){
            merges[mergeCount] = (Merge) {x_overlap_start, mon.h - 1, x_overlap_end, mon.h - 1};
            merges[mergeCount + 1] = (Merge) {x_overlap_start, next_mon.y, x_overlap_end, next_mon.y};
          } else {
            merges[mergeCount] = (Merge) {x_overlap_start, mon.y, x_overlap_end, mon.y};
            merges[mergeCount + 1] = (Merge) {x_overlap_start, next_mon.h - 1, x_overlap_end, next_mon.h - 1};
          }
          mergeCount += 2;
        }
      }
    }
}

void rayCast(Rect *mons, int x, int y, char dir) {
  BOOL isSet;
  int fx = 0, fy = 0;
  Rect mon;
  switch (dir) {
  case 'l':
    fy = y;
    for (int i = 0; i < monitorCount; i++) {
      mon = mons[i];
      if (!(mon.y <= fy && fy < mon.h))
        continue;
      if (!isSet || mon.w > fx) {
        fx = mon.w - 2;
        isSet = 1;
      }
    }
    break;
  case 'r':
    fy = y;
    for (int i = 0; i < monitorCount; i++) {
      mon = mons[i];
      if (!(mon.y <= fy && fy < mon.h))
        continue;
      if (!isSet || mon.x < fx) {
        fx = mon.x + 1;
        isSet = 1;
      }
    }
    break;
  case 'u':
    fx = x;
    for (int i = 0; i < monitorCount; i++) {
      mon = mons[i];
      if (!(mon.x <= fx && fx < mon.w))
        continue;
      if (!isSet || mon.h > fy) {
        fy = mon.h - 2;
        isSet = 1;
      }
    }
    break;
  case 'd':
    fx = x;
    for (int i = 0; i < monitorCount; i++) {
      mon = mons[i];
      if (!(mon.x <= fx && fx < mon.w))
        continue;
      if (!isSet || mon.y < fy) {
        fy = mon.y + 1;
        isSet = 1;
      }
    }
    break;
  }
  SetCursorPos(fx, fy);
}

Rect outerScreen(Rect *mons, int x, int y) {
  Rect mon;
  for (int i = 0; i < monitorCount; i++) {
    mon = mons[i];
    if (mon.x <= x && x < mon.w && mon.y <= y && y < mon.h)
      return mon;
  }
  return mon;
}

char getCollSide(Rect mon, int x, int y) {
  if (x <= mon.x)
    return 'l';
  if (x >= mon.w - 1)
    return 'r';
  if (y <= mon.y)
    return 'u';
  if (y >= mon.h - 1)
    return 'd';
  return '\0';
}

BOOL atMerge(Merge *mrgs, int x, int y) {
  for (int i = 0; i < mergeCount; i++) {
    Merge mrg = mrgs[i];
    if (mrg.start.x <= x && x <= mrg.end.x && mrg.start.y <= y && y <= mrg.end.y)
      return 1;
  }
  return 0;
}

POINT getMousePos() {
  POINT pt;
  GetCursorPos(&pt);
  return pt;
}

int isMousePressed() {
  return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) || (GetAsyncKeyState(VK_RBUTTON) & 0x8000);
}

int escapeSequence() {
  return GetAsyncKeyState(VK_ESCAPE) & 0x8000;
}

int main() {
  int i;
  SetProcessDPIAware();
  EnumDisplayMonitors(0, 0, MonitorEnumProc, 0);

  printf("Found %d monitor%c\n", monitorCount, monitorCount == 1 ? '\0' : 's');
  for (i = 0; i < monitorCount; i++)
    printf("Monitor %i: (%li %li %li %li)\n", i, monitors[i].x, monitors[i].y, monitors[i].w, monitors[i].h);
  printf("\n");
  getScreenMerge(monitors);

  printf("Found %d merge%c\n", mergeCount, mergeCount == 1 ? '\0' : 's');
  for (i = 0; i < mergeCount; i++)
    printf("Merge %i: (%li, %li) -> (%li, %li)\n", i, merges[i].start.x, merges[i].start.y, merges[i].end.x, merges[i].end.y);
  printf("\n");
  char dir;
  clock_t elapse = clock();
  POINT pt;

  while (1) {
    if (escapeSequence()) {
      i++;
      if (clock() - elapse >= 3 * CLOCKS_PER_SEC) break;
    } else {
      elapse = clock();
    }
    if (!isMousePressed()) {
      pt = getMousePos();
      if (!atMerge(merges, pt.x, pt.y)) {
        dir = getCollSide(outerScreen(monitors, pt.x, pt.y), pt.x, pt.y);
        if (dir) rayCast(monitors, pt.x, pt.y, dir);
      }
    }
    Sleep(10);
  }

  free(monitors);
  free(merges);
}