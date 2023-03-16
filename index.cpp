#include <iostream>
using namespace std;

#define black 0
#define white 1
#define vacant 2
#define n_board_idx 38  // インデックスの個数 縦横各8x2、斜め11x2
#define n_line 6561     // ボードの1つのインデックスが取りうる値の種類。3^8
#define hw 8 // そのまま
#define hw2 64 // そのまま

// インデックスごとのマスの移動数
const int move_offset[n_board_idx] = {1, 1, 1, 1, 1, 1, 1, 1, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};

// インデックス化するときのマス
const int global_place[n_board_idx][hw] = {
    {0, 1, 2, 3, 4, 5, 6, 7},{8, 9, 10, 11, 12, 13, 14, 15},{16, 17, 18, 19, 20, 21, 22, 23},{24, 25, 26, 27, 28, 29, 30, 31},{32, 33, 34, 35, 36, 37, 38, 39},{40, 41, 42, 43, 44, 45, 46, 47},{48, 49, 50, 51, 52, 53, 54, 55},{56, 57, 58, 59, 60, 61, 62, 63},
    {0, 8, 16, 24, 32, 40, 48, 56},{1, 9, 17, 25, 33, 41, 49, 57},{2, 10, 18, 26, 34, 42, 50, 58},{3, 11, 19, 27, 35, 43, 51, 59},{4, 12, 20, 28, 36, 44, 52, 60},{5, 13, 21, 29, 37, 45, 53, 61},{6, 14, 22, 30, 38, 46, 54, 62},{7, 15, 23, 31, 39, 47, 55, 63},
    {5, 14, 23, -1, -1, -1, -1, -1},{4, 13, 22, 31, -1, -1, -1, -1},{3, 12, 21, 30, 39, -1, -1, -1},{2, 11, 20, 29, 38, 47, -1, -1},{1, 10, 19, 28, 37, 46, 55, -1},{0, 9, 18, 27, 36, 45, 54, 63},{8, 17, 26, 35, 44, 53, 62, -1},{16, 25, 34, 43, 52, 61, -1, -1},{24, 33, 42, 51, 60, -1, -1, -1},{32, 41, 50, 59, -1, -1, -1, -1},{40, 49, 58, -1, -1, -1, -1, -1},
    {2, 9, 16, -1, -1, -1, -1, -1},{3, 10, 17, 24, -1, -1, -1, -1},{4, 11, 18, 25, 32, -1, -1, -1},{5, 12, 19, 26, 33, 40, -1, -1},{6, 13, 20, 27, 34, 41, 48, -1},{7, 14, 21, 28, 35, 42, 49, 56},{15, 22, 29, 36, 43, 50, 57, -1},{23, 30, 37, 44, 51, 58, -1, -1},{31, 38, 45, 52, 59, -1, -1, -1},{39, 46, 53, 60, -1, -1, -1, -1},{47, 54, 61, -1, -1, -1, -1, -1}
};

int turn_count_arr[2][n_line][hw][2]; //turn_count[プレイヤー][ボードのインデックス][マスの位置][0: 左 1: 右] = 何マスひっくり返せるか
bool legal_arr[2][n_line][hw];      // legal_arr[プレイヤー][ボードのインデックス][マスの位置] = trueなら合法、falseなら非合法
int flip_arr[2][n_line][hw], only_put_arr[2][n_line][hw];        // flip_arr[プレイヤー][ボードのインデックス][マスの位置] = ボードのインデックスのマスの位置をひっくり返した後のインデックス
int place_included[hw2][4];         // place_included[マスの位置] = そのマスが関わるインデックス番号の配列(3つのインデックスにしか関わらない場合は最後の要素に-1が入る)
int pow3[11];
int isi[n_line][hw]; // isi[インデックス][マスの位置] = そこにある石の色(black:0, white:1, vacant:2)
int local_place[n_board_idx][hw2];

void print_idx(int now) {
    for(int j = 0; j < hw; ++j) {
        if(now % 3 == 0) cerr << "O";
        else if(now % 3 == 1) cerr << "X";
        else cerr << ".";
        now /= 3;
    }
    cerr << endl;
}

// 黒白それぞれのindexにおける配置をbitboardで生成
int create_bit_idx(int idx, int col) {
    int res = 0;
    for(int place = 0; place < hw; ++place) {
        if(idx % 3 == col) {
            res |= 1 << place;
        }
        idx /= 3;
    }
    return res;
}

// ようわからん
int transfer(int m, int k) {
    if(k == 0) {
        return m << 1;
    } else {
        return m >> 1;
    }
}

// pのplaceに置いたときにoを何枚ｋ方向にひっくり返せるか？
int get_turn_count(int p, int o, int place, int k) {
    int res = 0;
    int pt = 1 << (hw - 1 - place);
    if (pt & p || pt & o)
        return res;
    int mask = transfer(pt, k);
    while(mask && (mask & o)) {
        ++res;
        mask = transfer(mask, k);
        if(mask & p) {
            return res;
        }
    }
    return 0;
}   
 
// indexボードの前計算をする
void board_init() {
    int idx, b, w, place, l_place, k;
    pow3[0] = 1;
    for(idx = 1; idx < 11; ++idx)
        pow3[idx] = pow3[idx - 1] * 3;
    for(idx = 0; idx < n_line; ++idx) {
        int now = idx;
        for(k = 0; k < hw; ++k) {
            isi[idx][k] = now % 3;
            now /= 3;
        }
    }
    for(idx = 0; idx < n_line; ++idx) {
        b = create_bit_idx(idx, black);
        w = create_bit_idx(idx, white);
        for(place = 0; place < hw; ++place) {
            turn_count_arr[black][idx][hw - place - 1][0] = get_turn_count(b, w, place, 1);
            turn_count_arr[black][idx][hw - place - 1][1] = get_turn_count(b, w, place, 0);
            if(turn_count_arr[black][idx][hw - place - 1][0] || turn_count_arr[black][idx][hw - place - 1][1]) legal_arr[black][idx][hw - place - 1] = true;
            else legal_arr[black][idx][hw - place - 1] = false;

            turn_count_arr[white][idx][hw - place - 1][0] = get_turn_count(w, b, place, 1);
            turn_count_arr[white][idx][hw - place - 1][1] = get_turn_count(w, b, place, 0);
            if(turn_count_arr[white][idx][hw - place - 1][0] || turn_count_arr[white][idx][hw - place - 1][1]) legal_arr[white][idx][hw - place - 1] = true;
            else legal_arr[white][idx][hw - place - 1] = false;
        }
        for(place = 0; place < hw; ++place) {
            flip_arr[black][idx][place] = idx;
            flip_arr[white][idx][place] = idx;
            only_put_arr[black][idx][place] = idx;
            only_put_arr[white][idx][place] = idx;
            if(isi[idx][place] == 0) {
                only_put_arr[white][idx][place] += pow3[place];
            } else if(isi[idx][place] == 1) {
                only_put_arr[black][idx][place] -= pow3[place];
            } else {
                only_put_arr[black][idx][place] -= 2 * pow3[place];
                only_put_arr[white][idx][place] -= pow3[place];
            }
            if(legal_arr[black][idx][place]) {
                for(k = place - turn_count_arr[black][idx][place][0]; k <= place + turn_count_arr[black][idx][place][1]; ++k) {
                    if(isi[idx][k] == 1) flip_arr[black][idx][place] -= pow3[k];
                    else if(isi[idx][k] == 2) flip_arr[black][idx][place] -= 2 * pow3[k];
                }
            }
            if(legal_arr[white][idx][place]) {
                for(k = place - turn_count_arr[white][idx][place][0]; k <= place + turn_count_arr[white][idx][place][1]; ++k) {
                    if(isi[idx][k] == 0) flip_arr[white][idx][place] += pow3[k];
                    else if(isi[idx][k] == 2) flip_arr[white][idx][place] -= pow3[k];
                }
            }
        }
    }
    for(place = 0; place < hw2; ++place) {
        int now = 0;
        for(idx = 0; idx < n_board_idx; ++idx) {
            for(l_place = 0; l_place < hw; ++l_place) {
                if(global_place[idx][l_place] == place) {
                    place_included[place][now] = idx;
                    now++;
                }
            }
        }
        if (now == 3)
            place_included[place][now] = -1;
    }
    
    for (idx = 0; idx < n_board_idx; ++idx){
        for (place = 0; place < hw2; ++place){
            local_place[idx][place] = -1;
            for (l_place = 0; l_place < hw; ++l_place){
                if (global_place[idx][l_place] == place)
                    local_place[idx][place] = l_place;
            }
        }
    }

    cerr << "A index board generated." << endl;
}

struct Board {
    int board_idx[n_board_idx]; // インデックス
    int player;                 // 盤面から打つ手番
    int policy;                 // 盤面に至る直前に打った手
    int value;                  // 盤面の仮の評価値(move orderingに使う)
    int n_stones;               // 石数
    
    // 合法手かどうか
    bool legal(int p) {
        if(legal_arr[player][board_idx[place_included[p][0]]][local_place[place_included[p][0]][p]]) return true;
        if(legal_arr[player][board_idx[place_included[p][1]]][local_place[place_included[p][1]][p]]) return true;
        if(legal_arr[player][board_idx[place_included[p][2]]][local_place[place_included[p][2]][p]]) return true;
        if(place_included[p][3] != 0 && legal_arr[player][board_idx[place_included[p][3]]][local_place[place_included[p][3]][p]]) return true;
        return false;
    }

    void flip(Board &res, int g_place) {
        for (int i = 0; i < 3; ++i)
            res.board_idx[place_included[g_place][i]] = only_put_arr[player][res.board_idx[place_included[g_place][i]]][local_place[place_included[g_place][i]][g_place]];
        if (place_included[g_place][3] != -1)
            res.board_idx[place_included[g_place][3]] = only_put_arr[player][res.board_idx[place_included[g_place][3]]][local_place[place_included[g_place][3]][g_place]];
    }
    void move_p(Board &res, int place, int i) {
        int j;
        int now_idx = res.board_idx[place_included[place][i]];
        int l_place = local_place[place_included[place][i]][place];
        res.board_idx[place_included[place][i]] = only_put_arr[player][res.board_idx[place_included[place][i]]][local_place[place_included[place][i]][place]];
        for (j = 1; j <= turn_count_arr[player][now_idx][l_place][0]; ++j) flip(res, place - move_offset[place_included[place][i]] * j);
        for (j = 1; j <= turn_count_arr[player][now_idx][l_place][1]; ++j) flip(res, place + move_offset[place_included[place][i]] * j);
    }

    // 着手する.着手後のBoardクラスを返す
    Board move(const int place) {
        Board res;
        for(int i = 0; i < n_board_idx; ++i) res.board_idx[i] = board_idx[i];
        res.player = 1 - player;
        res.policy = place;
        res.n_stones = n_stones + 1;

        // ひっくり返した場所１つ１つにかかるインデックス４つもかわるのでめんどい
        // 左方向を調べる
        move_p(res, place, 0);
        move_p(res, place, 1);
        move_p(res, place, 2);
        if (place_included[place][3] != -1) move_p(res, place, 3);
        return res;
    }

    // ボードを出力
    void print() {
        for(int i = 0; i < hw; ++i) {
            int now = board_idx[i];
            for(int j = 0; j < hw; ++j) {
                if(now % 3 == 0) cerr << "O";
                else if(now % 3 == 1) cerr << "X";
                else cerr << ".";
                now /= 3;
            }
            cerr << endl;
        }
    }

    // vacant:2, black:0, white:1配列からインデックス形式に変換
    void translate_from_arr(const int arr[], int ai_player) {
        int i, j;
        n_stones = hw2;
        // とりまこうする
        for(i = 0; i < n_board_idx; ++i) {
            board_idx[i] = n_line - 1;
        }

        for(i = 0; i < hw2; ++i) {
            for(j = 0; j < 4; ++j) {
                int now_index = place_included[i][j];
                if(now_index == -1)
                    continue;
                if(arr[i] == black) {
                    board_idx[now_index] -= 2 * pow3[local_place[now_index][i]]; 
                } else if(arr[i] == white) {
                    board_idx[now_index] -= pow3[local_place[now_index][i]]; 
                } else {
                    // 空白なので、石の数を１減らす 
                    n_stones--;
                }
            }
        }

        // ついでに構造体の初期設定もする
        player = ai_player;
    }
};










const int cell_weight[hw2] = {
    130, -12, 0, -1, -1, 0, -12, 130,
    -12, -25, -3, -3, -3, -3, -25, -12,
    0, -3, 0, -1, -1, 0, -3, 0,
    -1, -3, -1, -1, -1, -1, -3, -1,
    -1, -3, -1, -1, -1, -1, -3, -1,
    0, -3, 0, -1, -1, 0, -3, 0,
    -12, -25, -3, -3, -3, -3, -25, -12,
    130, -12, 0, -1, -1, 0, -12, 130
};

int cell_score[hw / 2][n_line]; 
void evaluate_init() {
    int idx, i, place, b, w;
    for (idx = 0; idx < n_line; ++idx) {
        b = create_bit_idx(idx, 0);
        w = create_bit_idx(idx, 1);
        for (i = 0; i < hw / 2; ++i)
            cell_score[i][idx] = 0;
        for (place = 0; place < hw; ++place) {
            for (i = 0; i < hw / 2; ++i) {
                cell_score[i][idx] += (1 & (b >> place)) * cell_weight[i * hw + place];
                cell_score[i][idx] -= (1 & (w >> place)) * cell_weight[i * hw + place];
            }
        }
    }
}

int evaluate(Board b) {
    int res = 0, i;
    for (i = 0; i < hw / 2; ++i)
        res += cell_score[i][b.board_idx[i]];
    for (i = 0; i < hw / 2; ++i)
        res += cell_score[hw / 2 - 1 - i][b.board_idx[hw / 2 + i]];
    if (b.player == white)
        res = -res;
    return res;
}


int search(Board b, int depth) {
    int i, mx = -1e9, res = 0;
    for(i = 0; i < hw2; ++i) if(b.legal(i)) {
        if(mx < -evaluate(b.move(i))) {
            res = i;
            mx = -evaluate(b.move(i));
        }
    }
    return res;
}

int main() {
    board_init();
    evaluate_init();
    int id, policy; cin >> id;
    string s;
    int arr[64];
    Board b;
    while(true) {
        cin >> s;
        for(int i = 0; i < hw2; ++i) {
            char x = s[i];
            if(x == '1') arr[i] = 1;
            else if(x == '0') arr[i] = 0;
            else arr[i] = -1;
        }
        b.translate_from_arr(arr, id);
        policy = search(b, 4);
        cout << policy / 8 << " " << policy % 8 << endl;
    }
    
}