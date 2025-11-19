// mouse_sim.c (updated with step-following loop and recompute-on-block)
// Build: gcc -std=c11 -O2 mouse_sim.c -o mouse_sim && ./mouse_sim
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define MAZE_N 16

// 壁ビット（3:S, 2:E, 1:W, 0:N）
#define WALL_N (1u<<0)
#define WALL_W (1u<<1)
#define WALL_E (1u<<2)
#define WALL_S (1u<<3)

// 壁情報の変数
uint8_t truth[MAZE_N][MAZE_N] = { { 14,4,6,6,6,6,6,6,6,6,6,6,6,6,6,5 },{ 12,3,12,4,6,6,6,6,6,6,4,6,6,5,12,3 },{ 9,12,3,9,12,6,6,6,6,5,10,4,5,9,10,5 },{ 9,9,12,1,9,12,6,6,6,3,12,3,9,10,5,9 },{ 9,9,9,10,1,10,6,6,6,5,10,5,9,12,3,9 },{ 9,9,8,7,10,5,14,4,5,9,12,3,9,10,6,1 },{ 9,9,10,5,12,3,12,3,10,3,10,5,9,12,5,11 },{ 9,10,5,9,10,5,9,12,5,12,4,3,10,3,10,5 },{ 8,7,9,9,12,3,9,10,1,11,10,5,14,5,12,3 },{ 9,12,1,9,10,5,9,14,0,7,12,2,4,2,2,5 },{ 9,9,9,9,12,3,9,14,0,7,10,5,8,6,6,3 },{ 9,8,1,9,10,5,9,14,0,7,12,3,10,6,6,5 },{ 9,9,9,9,12,3,9,14,0,7,10,4,6,6,5,9 },{ 9,9,10,3,10,5,9,12,0,5,12,2,7,13,9,9 },{ 9,10,5,13,12,3,8,1,11,9,10,6,6,1,9,11 },{ 10,6,2,3,10,6,3,10,6,2,6,6,6,3,10,7 } };

// --- ユーティリティ ---
static int in_range(int x, int y) {return (0 <= x && x < MAZE_N && 0 <= y && y < MAZE_N);}

void printMazeBits(const uint8_t maze[MAZE_N][MAZE_N], const char *title) {
    printf("=== %s (bit: S E W N) ===\n", title);
    for (int8_t y = MAZE_N - 1; y >= 0; --y) {
        for (int8_t x = 0; x < MAZE_N; ++x) {
            uint8_t c = maze[y][x];
            char s = (c & WALL_S) ? '1' : '0';
            char e = (c & WALL_E) ? '1' : '0';
            char w = (c & WALL_W) ? '1' : '0';
            char n = (c & WALL_N) ? '1' : '0';
            printf("[%c%c%c%c]", s, e, w, n);
        }
        printf("  y=%d\n", y);
    }
    printf("  x=0                                                        x=15\n\n");
}

// --- セル配列 -> 垂直/水平壁配列（別用途の1次元配列）へ変換 ---
// vWalls[0..16] : 縦の壁線（xの壁線）。各要素のbit yが「その壁線のy位置に壁がある」
// hWalls[0..16] : 横の壁線（yの壁線）。各要素のbit xが「その壁線のx位置に壁がある」
void cellsToVH(const uint8_t maze[MAZE_N][MAZE_N], uint16_t vWalls[MAZE_N+1], uint16_t hWalls[MAZE_N+1]) {
    for (uint8_t i = 0; i <= MAZE_N; ++i) { vWalls[i] = 0; hWalls[i] = 0; }
    for (uint8_t y = 0; y < MAZE_N; ++y) {
        for (uint8_t x = 0; x < MAZE_N; ++x) {
            uint8_t c = maze[y][x];
            if (c & WALL_W) vWalls[x]      |= (1u << y); // 西壁は壁線x
            if (c & WALL_E) vWalls[x+1]    |= (1u << y); // 東壁は壁線x+1
            if (c & WALL_S) hWalls[y]      |= (1u << x); // 南壁は壁線y
            if (c & WALL_N) hWalls[y+1]    |= (1u << x); // 北壁は壁線y+1
        }
    }
}

void drawMazeASCII(const uint16_t vWalls[MAZE_N+1], const uint16_t hWalls[MAZE_N+1],
                   int mx, int my, const int dist[MAZE_N][MAZE_N]) {
    for (int8_t y = MAZE_N - 1; y >= 0; --y) {
        // 上側の横壁線（hWalls[y+1]）
        printf("+");
        for (int8_t x = 0; x < MAZE_N; ++x) {
            int has = (hWalls[y+1] & (1u << x)) ? 1 : 0;
            printf("%s+", has ? "---" : "   ");
        }
        printf("\n");

        // 左側の縦壁線とセル内表示
        for (int8_t x = 0; x < MAZE_N; ++x) {
            int hasL = (vWalls[x] & (1u << y)) ? 1 : 0;
            printf("%c", hasL ? '|' : ' ');
            if (x == mx && y == my) {
                printf(" M ");
            } else if (dist) {
                if (dist[y][x] >= 0 && dist[y][x] <= 99) {
                    printf("%2d ", dist[y][x]);
                } else {
                    printf(" .. ");
                }
            } else {
                printf("   ");
            }
        }
        int hasR = (vWalls[MAZE_N] & (1u << y)) ? 1 : 0;
        printf("%c\n", hasR ? '|' : ' ');
    }
    // 最下段
    printf("+");
    for (int8_t x = 0; x < MAZE_N; ++x) {        int has = (hWalls[0] & (1u << x)) ? 1 : 0;
        printf("%s+", has ? "---" : "   ");
    }
    printf("\n");
}

// --- センサ観測：到着区画の壁を既知へ反映（相互整合も確保） ---
void senseAndStoreWalls(uint8_t known[MAZE_N][MAZE_N], int x, int y) {
    // 例外処理
    if (!in_range(x, y)) return;
    
    uint8_t c = truth[y][x];
    known[y][x] = c; // 到着区画の全方位が観測できたと仮定
    printf("(%d,%d) is 0x%2x\n",y,x,truth[y][x]);

    if ((c & WALL_N) && in_range(x, y+1)) known[y+1][x] |= WALL_S;
    if ((c & WALL_S) && in_range(x, y-1)) known[y-1][x] |= WALL_N;
    if ((c & WALL_E) && in_range(x+1, y)) known[y][x+1] |= WALL_W;    
    if ((c & WALL_W) && in_range(x-1, y)) known[y][x-1] |= WALL_E;
}

// --- 歩数マップ（既知壁のみを障害物扱い） ---
void computeStepMap(const uint8_t known[MAZE_N][MAZE_N], int gx, int gy, int dist[MAZE_N][MAZE_N]) {
    for (int y = 0; y < MAZE_N; ++y)
        for (int x = 0; x < MAZE_N; ++x)
            dist[y][x] = -1;

    if (!in_range(gx, gy)) return;

    int qx[MAZE_N*MAZE_N], qy[MAZE_N*MAZE_N];
    int qs = 0, qe = 0;

    dist[gy][gx] = 0;
    qx[qe] = gx; qy[qe] = gy; qe++;

    while (qs != qe) {
        int x = qx[qs], y = qy[qs]; qs++;
        int d = dist[y][x];

        if (!(known[y][x] & WALL_N) && in_range(x, y+1) && dist[y+1][x] < 0) {
            dist[y+1][x] = d + 1; qx[qe] = x;   qy[qe] = y+1; qe++;
        }
        if (!(known[y][x] & WALL_S) && in_range(x, y-1) && dist[y-1][x] < 0) {
            dist[y-1][x] = d + 1; qx[qe] = x;   qy[qe] = y-1; qe++;
        }
        if (!(known[y][x] & WALL_E) && in_range(x+1, y) && dist[y][x+1] < 0) {
            dist[y][x+1] = d + 1; qx[qe] = x+1; qy[qe] = y;   qe++;
        }
        if (!(known[y][x] & WALL_W) && in_range(x-1, y) && dist[y][x-1] < 0) {
            dist[y][x-1] = d + 1; qx[qe] = x-1; qy[qe] = y;   qe++;
        }
    }
}

void printStepMap(const int dist[MAZE_N][MAZE_N], const char *title) {
    printf("=== %s (goal=0) ===\n", title);
    for (uint8_t y = MAZE_N - 1; y >= 0; --y) {
        for (uint8_t x = 0; x < MAZE_N; ++x) {
            if (dist[y][x] < 0) printf("  .");
            else                printf("%3d", dist[y][x]);
        }
        printf("  y=%d\n", y);
    }
    printf("  x=0                                  ...                 x=15\n\n");
}

// --- デモ用：内部壁の追加（隣接セル間に壁を立てる） ---
void addWallBetween(int x, int y, char dir) {
    if (!in_range(x, y)) return;
    if (dir == 'N' && in_range(x, y+1)) {
        truth[y][x]   |= WALL_N;
        truth[y+1][x] |= WALL_S;
    } else if (dir == 'S' && in_range(x, y-1)) {
        truth[y][x]   |= WALL_S;
        truth[y-1][x] |= WALL_N;
    } else if (dir == 'E' && in_range(x+1, y)) {
        truth[y][x]   |= WALL_E;
        truth[y][x+1] |= WALL_W;
    } else if (dir == 'W' && in_range(x-1, y)) {
        truth[y][x]   |= WALL_W;
        truth[y][x-1] |= WALL_E;
    }
}

void initKnownWithBorders(uint8_t known[MAZE_N][MAZE_N]) {
    memset(known, 0, sizeof(uint8_t)*MAZE_N*MAZE_N);
    // 左右両端の壁を配置したい
    for (int y = 0; y < MAZE_N; ++y) {
        known[y][0]        |= (0x02 & WALL_W);
        known[y][MAZE_N-1] |= (0x04 & WALL_E);
    }
    // 上下両端の壁を配置したい
    for (int x = 0; x < MAZE_N; ++x) {
        known[0][x]        |= (0x08 & WALL_S);
        known[MAZE_N-1][x] |= (0x01 & WALL_N);
    }
}

// --- 追加：歩数マップに従って1マスずつ移動するシミュレーション ---
// 仕様：
// ・各ループで「既知壁から距離マップ計算 → 1マス移動 → 到着区画を観測」を行う。
// ・距離に従って選んだ進行方向が、真値の壁で実際には塞がれていた場合：
//    - その場で現在区画を観測して既知壁を更新
//    - 再度、既知壁のみで距離マップを計算し直し、移動をやり直す
// ・ゴールに到達したら終了。安全のためステップ数に上限あり。
typedef enum {DIR_NONE=-1, DIR_N=0, DIR_E=1, DIR_S=2, DIR_W=3} Dir;

static const char* dirName(Dir d) {
    switch(d){
        case DIR_N: return "N";
        case DIR_E: return "E";
        case DIR_S: return "S";
        case DIR_W: return "W";
        default: return "-";
    }
}

static int dirDx(Dir d) {return (d==DIR_E) - (d==DIR_W);}
static int dirDy(Dir d) {return (d==DIR_N) - (d==DIR_S);}

static uint8_t dirBit(Dir d) {
    switch(d){
        case DIR_N: return WALL_N;
        case DIR_E: return WALL_E;
        case DIR_S: return WALL_S;
        case DIR_W: return WALL_W;
        default: return 0;
    }
}

// 距離マップに従い、現在セルから距離が1小さい隣接セルへ向かう（優先順 N,E,S,W）
static Dir chooseNextDir(const int dist[MAZE_N][MAZE_N], int x, int y) {
    int d = dist[y][x];
    if (d <= 0) return DIR_NONE; // 0ならゴール、<0は未到達
    struct { Dir d; int dx; int dy; } cand[4] = {
        {DIR_N, 0, +1},
        {DIR_E, +1, 0},
        {DIR_S, 0, -1},
        {DIR_W, -1, 0},
    };
    for (int i=0;i<4;i++){
        int nx = x + cand[i].dx;
        int ny = y + cand[i].dy;
        if (in_range(nx, ny) && dist[ny][nx] == d-1) return cand[i].d;
    }
    return DIR_NONE;
}

void simulateToGoal( uint8_t known[MAZE_N][MAZE_N],int sx, int sy, int gx, int gy,
                    int max_steps, int verbose_every) {
    int mx = sx, my = sy;
    uint16_t vWalls[MAZE_N+1], hWalls[MAZE_N+1];
    int dist[MAZE_N][MAZE_N];

    // 初手でスタート区画を観測しておく（好みで切替可）
//    senseAndStoreWalls(known, mx, my);

    for (int step = 0; step < max_steps; /* 手動インクリメント */ ) {
        computeStepMap(known, gx, gy, dist);

        if (dist[my][mx] < 0) {
            printf("[STOP] Need more search\n");
            cellsToVH(known, vWalls, hWalls);
            drawMazeASCII(vWalls, hWalls, mx, my, dist);
            break;
        }
        if (dist[my][mx] == 0) {
            printf("[GOAL] get (%d,%d)\n", gx, gy);
            cellsToVH(known, vWalls, hWalls);
            drawMazeASCII(vWalls, hWalls, mx, my, dist);
            break;
        }

        Dir dir = chooseNextDir(dist, mx, my);
        if (dir == DIR_NONE) {
            // 何らかの理由で次手が選べない（局所的な未整合など）
            printf("[RECOMP] ERROR\n");
            senseAndStoreWalls(known, mx, my);
            continue; // 再計算
        }

        // 進行方向に真値の壁があるなら、ブロックされる → 現在地を観測して再計算
        if (truth[my][mx] & dirBit(dir)) {
            printf("[BLOCK] Direction %s has wall, Sense(%d,%d) & Reculculate\n", dirName(dir), mx, my);
            senseAndStoreWalls(known, mx, my);
            continue;
        }

        // 実際に1マス移動
        int nx = mx + dirDx(dir);
        int ny = my + dirDy(dir);
        printf("[STEP %d] %s move: (%d,%d) -> (%d,%d)\n", step+1, dirName(dir), mx, my, nx, ny);
        mx = nx; my = ny;
        step++;

        // 到着区画を観測して既知を更新
        senseAndStoreWalls(known, mx, my);

        // 表示（頻度を抑えるためのverbose_every）
        if (verbose_every > 0 && (step % verbose_every == 0 || dist[my][mx] == 0)) {
            cellsToVH(known, vWalls, hWalls);
            drawMazeASCII(vWalls, hWalls, mx, my, dist);
        }
    }
}

int main(void) {
    uint8_t known[MAZE_N][MAZE_N];
    uint16_t vWalls[MAZE_N+1], hWalls[MAZE_N+1];
    int dist[MAZE_N][MAZE_N];

    // 迷路（真値）と既知（外周のみ）を用意
    initKnownWithBorders(known);

    // 1) 二次元壁情報の表示（真値・既知）
    printMazeBits(truth, "Ground Truth (uint8_t walls)");
    printMazeBits(known, "Known Walls (initial, borders only)");
    sleep(3);

    // 2) マウス配置＆全体表示（既知壁のみ）
    int sx = 0, sy = 0;
    cellsToVH(known, vWalls, hWalls);
    printf("=== Maze + Mouse (Known walls only) ===\n");
    drawMazeASCII(vWalls, hWalls, sx, sy, NULL);
    sleep(3);

    // 3) スタート区画を観測（初回）
    senseAndStoreWalls(known, sx, sy);
    cellsToVH(known, vWalls, hWalls);
    printf("\n=== After first sensing at start (%d,%d) ===\n", sx, sy);
    drawMazeASCII(vWalls, hWalls, sx, sy, NULL);
    sleep(3);

    // 4) 距離マップの展開（例：ゴール単一点 7,7）
    int gx = 7, gy = 7;
    computeStepMap(known, gx, gy, dist);
    printStepMap(dist, "Distance Map from Goal (Known walls)");
    printf("=== Maze + Mouse + Distance (Known walls) ===\n");
    drawMazeASCII(vWalls, hWalls, sx, sy, dist);
    sleep(3);

    // 5) 追記仕様：距離マップに従って1マスずつ移動し、ブロック時は再計算
    printf("\n=== Simulation: follow distance map step-by-step (recompute on block) ===\n");
    sleep(3);
    simulateToGoal(known, sx, sy, gx, gy, /*max_steps=*/200, /*verbose_every=*/4);

    // 最終状態の距離マップを表示
/*
    computeStepMap(known, gx, gy, dist);
    printStepMap(dist, "Final Distance Map (Known walls)");
    uint8_tsToVH(known, vWalls, hWalls);
    printf("=== Final Maze + Mouse + Distance (Known walls) ===\n");
    drawMazeASCII(vWalls, hWalls, sx, sy, dist);
*/
    return 0;
}
