#include <stdio.h>
#include <windows.h>

typedef struct {
  LONG x, y, w, h;
} Rect;

typedef struct {
  POINT start, end;
} Merge;

Rect *monitors = 0;
Merge *merges = 0;
int monitorCount = 0, mergeCount = 0;

BOOL CALLBACK MonitorEnumProc(HMONITOR hMon, HDC hdc, LPRECT lprc,
                              LPARAM data) {
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
      if (mon.w == next_mon.x) {
        long y_overlap_start = max(mon.y, next_mon.y);
        long y_overlap_end = min(mon.h, next_mon.h);
        if (y_overlap_start < y_overlap_end) {
          merges = realloc(merges, (mergeCount + 2) * sizeof(Merge));
          merges[mergeCount] =
              (Merge){mon.w - 1, y_overlap_start, mon.w - 1, y_overlap_end};
          merges[mergeCount] =
              (Merge){next_mon.x, y_overlap_start, next_mon.x, y_overlap_end};
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
  if (x == mon.x)
    return 'l';
  if (x == mon.w - 1)
    return 'r';
  if (y == mon.y)
    return 'u';
  if (y == mon.h - 1)
    return 'd';
  return '\0';
}

BOOL atMerge(Merge *mrgs, int x, int y) {
  for (int i = 0; i < mergeCount; i++) {
    Merge mrg = mrgs[i];
    if (mrg.start.x <= x && x <= mrg.end.x && mrg.start.y <= y &&
        y <= mrg.end.y)
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
  return GetAsyncKeyState(VK_ESCAPE) & 0x8001;
}

int main() {
  int i;
  SetProcessDPIAware();
  EnumDisplayMonitors(0, 0, MonitorEnumProc, 0);

  printf("Found %d monitor%c\n", monitorCount, monitorCount == 1 ? '\0' : 's');
  for (i = 0; i < monitorCount; i++)
    printf("Monitor %i: (%lu %lu %lu %lu)\n", i, monitors[i].x, monitors[i].y, monitors[i].w, monitors[i].h);

  printf("Found %d merge%c\n", mergeCount, mergeCount == 1 ? '\0' : 's');
  for (i = 0; i < mergeCount; i++)
    printf("Merge %i: (%li, %li) -> (%li, %li)", i, merges[i].start.x, merges[i].start.y, merges[i].end.x, merges[i].end.y);

  i = 0;
  char dir;

  while (1) {
    if (escapeSequence()) {
      i++;
      if (i >= 200)
        break;
    } else {
      i = 0;
    }
    if (!isMousePressed()) {
      POINT pt = getMousePos();
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
