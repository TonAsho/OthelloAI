#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
using namespace std;
using int_64 = uint64_t;
#define rep(i, n) for (int i = 0; i < n; ++i)


// CodingGame用.....................................
int id, board_size, action_count;
vector<string> field, put;
// .................................................

const int H = 8, W = 8;
const int Evaluation[64] = {30, -12, 0, -1, -1, 0, -12, 30,-12, -15, -3, -3, -3, -3, -15, -12,0, -3, 0, -1, -1, 0, -3, 0, -1, -3, -1, -1, -1, -1, -3, -1,-1, -3, -1, -1, -1, -1, -3, -1,0, -3, 0, -1, -1, 0, -3, 0, -12, -15, -3, -3, -3, -3, -15, -12,30, -12, 0, -1, -1, 0, -12, 30};


struct Board {
    int_64 playerboard;
    int_64 opponentboard;
    int_64 m;
    int counter;
    int_64 policy; // 盤面に至る直前に打った手
    int value;  // 盤面の仮の評価値(move orderingに使う)
    bool active; // 試合中かどうか.
    Board() {
        playerboard   =   0x0000000810000000;
        opponentboard =   0x0000001008000000;
        counter = 0;
        active = true;
    }

    // 評価関数
    int evaluation() {
        int x = 0, y = 0;
        rep(i, 64) {
            if(playerboard & (1ll << i)) x += Evaluation[63 - i];
            else if(opponentboard & (1ll << i)) y += Evaluation[63 - i];
        }
        return x - y;
    }

    // isPass() = 手番がパスだったらtrue
    bool isPass() {
        return (get_legal_board() == 0);
    }

    // turn_koma()
    void turn_koma(int_64 m) {
        if((get_legal_board() & m) == 0) {
            cerr << "そこには置けません." << endl;
            return ;
        }
        policy = m;
        int_64 res = 0;
        rep(i, 8) {
            int_64 reverse_board = 0;
            int_64 tmp = transfer(m,i);
            while(tmp != 0 && (tmp & opponentboard) != 0) {
                reverse_board |= tmp;
                tmp = transfer(tmp,i);
            }
            if((tmp & playerboard) != 0) {
                res |= reverse_board;
            } 
        }
        playerboard ^= m | res;
        opponentboard ^= res;
        swap(playerboard, opponentboard);
        counter++;
        // if(isPass()) {
        //     //cerr << "パスします." << endl;
        //     swap(playerboard, opponentboard);
        //     if(isPass()) {
        //         //cerr << "試合終了です." << endl;
        //         active = false;
        //     }
        // }
        return ;
    }

    // ようわからん
    int_64 transfer(int_64 put, int i) {
        if(i == 0) return (put << 8) & 0xffffffffffffff00ll;
        if(i == 1) return (put << 7) & 0x7f7f7f7f7f7f7f00ll;
        if(i == 2) return (put >> 1) & 0x7f7f7f7f7f7f7f7fll;
        if(i == 3) return (put >> 9) & 0x007f7f7f7f7f7f7fll;
        if(i == 4) return (put >> 8) & 0x00ffffffffffffffll;
        if(i == 5) return (put >> 7) & 0x00fefefefefefefell;
        if(i == 6) return (put << 1) & 0xfefefefefefefefell;
        if(i == 7) return (put << 9) & 0xfefefefefefefe00ll;
        return 0;
    }

    //　can_put(player, opponent)置ける場所をビットで返す. m は着手箇所、1ビットのみが1で他はすべて0
    int_64 get_legal_board() {
        int_64 tmp;
        int_64 x = opponentboard & 0x7e7e7e7e7e7e7e7e;
        int_64 y = opponentboard & 0x00FFFFFFFFFFFF00;
        int_64 z = opponentboard & 0x007e7e7e7e7e7e00;
        int_64 legal_board;
        int_64 blank_board = ~(playerboard | opponentboard);

        // 左方向
        tmp = (playerboard << 1) & x;
        tmp |= (tmp << 1) & x;
        tmp |= (tmp << 1) & x;
        tmp |= (tmp << 1) & x;
        tmp |= (tmp << 1) & x;
        tmp |= (tmp << 1) & x;
        legal_board = (tmp << 1) & blank_board;

        // 右方向
        tmp = (playerboard >> 1) & x;
        tmp |= (tmp >> 1) & x;
        tmp |= (tmp >> 1) & x;
        tmp |= (tmp >> 1) & x;
        tmp |= (tmp >> 1) & x;
        tmp |= (tmp >> 1) & x;
        legal_board |= (tmp >> 1) & blank_board;

        // 上方向
        tmp = (playerboard << 8) & y;
        tmp |= (tmp << 8) & y;
        tmp |= (tmp << 8) & y;
        tmp |= (tmp << 8) & y;
        tmp |= (tmp << 8) & y;
        tmp |= (tmp << 8) & y;
        legal_board |= (tmp << 8) & blank_board;

        // 下方向
        tmp = (playerboard >> 8) & y;
        tmp |= (tmp >> 8) & y;
        tmp |= (tmp >> 8) & y;
        tmp |= (tmp >> 8) & y;
        tmp |= (tmp >> 8) & y;
        tmp |= (tmp >> 8) & y;
        legal_board |= (tmp >> 8) & blank_board;

        // 右斜め上方向
        tmp = (playerboard << 7) & z;
        tmp |= (tmp << 7) & z;
        tmp |= (tmp << 7) & z;
        tmp |= (tmp << 7) & z;
        tmp |= (tmp << 7) & z;
        tmp |= (tmp << 7) & z;
        legal_board |= (tmp << 7) & blank_board;

        // 左斜め上方向
        tmp = (playerboard << 9) & z;
        tmp |= (tmp << 9) & z;
        tmp |= (tmp << 9) & z;
        tmp |= (tmp << 9) & z;
        tmp |= (tmp << 9) & z;
        tmp |= (tmp << 9) & z;
        legal_board |= (tmp << 9) & blank_board;

        // 右斜め下方向
        tmp = (playerboard >> 9) & z;
        tmp |= (tmp >> 9) & z;
        tmp |= (tmp >> 9) & z;
        tmp |= (tmp >> 9) & z;
        tmp |= (tmp >> 9) & z;
        tmp |= (tmp >> 9) & z;
        legal_board |= (tmp >> 9) & blank_board;

        // 左斜め下方向
        tmp = (playerboard >>7) & z;
        tmp |= (tmp >> 7) & z;
        tmp |= (tmp >> 7) & z;
        tmp |= (tmp >> 7) & z;
        tmp |= (tmp >> 7) & z;
        tmp |= (tmp >> 7) & z;
        legal_board |= (tmp >>7) & blank_board;

        return legal_board;
    }

    // print(black, white) = ８✖️８のマスに変換して表示する
    void print() {
        int_64 black, white;
        if(counter % 2 == 0) {
            black = playerboard;
            white = opponentboard;
        } else {
            black = opponentboard;
            white = playerboard;
        }
        rep(i, H) {
            rep(j, W) {
                int_64 temp = 1ll << 63-(i*8+j);
                if(black & temp) {
                    cerr << "o";
                } else if(white & temp) {
                    cerr << "x";
                } else {
                    cerr << ".";
                }
            }
            cerr << endl;
        }
    }
    // print_board(bit_board) = 8 * 8に交換
    void print_board(int_64 x) {
        rep(i, H) {
            rep(j, W) {
                int_64 temp = 1ll << 63-(i*8+j);
                if(x & temp) {
                    cerr << "o";
                } else {
                    cerr << ".";
                }
            }
            cerr << endl;
        }
    }

    // xのint_64における立っているbitの個数
    int pop_count(int_64 &x) {
        x = x - ((x >> 1) & 0x5555555555555555);
	    x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);
	    x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0F;
	    x = (x * 0x0101010101010101) >> 56;
        return x;
    }
    // sortする用
    bool operator<(const Board& another) const {
        return value > another.value;
    }
    // ==
    bool operator==(const Board& another) const {
        return (playerboard==another.playerboard && opponentboard==another.opponentboard);
    }
    // !=
    bool operator!=(const Board& another) const {
        return !(operator==(another));
    }
    // unordered_map用
    struct Hash {
        size_t operator()(Board item) const {
            std::size_t h1 = hash<int_64>()(item.playerboard);
            std::size_t h2 = hash<int_64>()(item.opponentboard);
            return h1 ^ h2;
        }
    };
    
};


unordered_map<Board, int, Board::Hash> seen_board;
unordered_map<Board, int, Board::Hash> pre_seen_board;

// move ordering用
int calc_value(Board x) {
    int res;
    if(pre_seen_board.find(x) != pre_seen_board.end()) {
        res = 1000 - pre_seen_board[x];
    } else {
        res = -x.evaluation();
    }
    return res;
}

int_64 nodes = 0;
// alpha_beta法
int alpha_beta(Board b, int depth, bool passed, int alpha, int beta) {
    nodes++;
    if(depth == 0) {
        return b.evaluation();
    }
    // パスの場合
    if(b.isPass()) {
        if(passed) {
            return b.evaluation();
        }
        swap(b.playerboard, b.opponentboard);
        return -alpha_beta(b, depth, true, -beta, -alpha);
    }

    // 探索済みの局面があった場合
    if(seen_board.find(b) != seen_board.end()) 
        return seen_board[b];

    int_64 m = b.get_legal_board();
    vector<Board> child_nodes;
    rep(i, 64) if(m & (1ll << i)) {
        Board x = b;
        x.turn_koma(1ll << i);
        x.value = calc_value(x);
        child_nodes.push_back(x);
    }

    sort(child_nodes.begin(), child_nodes.end());
    
    int mx = -1e9;
    for(Board x : child_nodes) {
        int val = -alpha_beta(x, depth - 1, false, -beta, -alpha);
        if(val >= beta) {
            return val;
        }
        alpha = max(alpha, val);
        mx = max(mx, val);
    }
    return seen_board[b] = mx;
}


// それっぽくなってきたAI
int search(Board board, int depth) {

    int_64 m = board.get_legal_board();
    int res = 0, mx = -1e9, alpha = -1e9, beta = 1e9;
    
    vector<Board> child_nodes;
    rep(i, 64) if(m & (1ll << i)) {
        Board x = board;
        x.turn_koma(1ll << i);
        x.policy = 63 - i;
        child_nodes.push_back(x);
    }

    for(int i = max(1, depth - 3); i <= depth; ++i) {
        nodes = 0;
        alpha = -1e9, beta = 1e9;
        for(Board &x : child_nodes) {
            x.value = calc_value(x);
        }
        sort(child_nodes.begin(), child_nodes.end());

        for(Board x : child_nodes) {
            int now = -alpha_beta(x, i - 1, false, -beta, -alpha);
            if(alpha < now) {
                alpha = now;
                res = x.policy;
            }
        }
        pre_seen_board.swap(seen_board);
        seen_board.clear();
    }
    return res;
}



#include <random>
std::random_device rd;
std::mt19937 gen(rd());
int random(int low, int high)
{
    std::uniform_int_distribution<> dist(low, high);
    return dist(gen);
}
#include <chrono>
#include <bitset>
int main() {
    Board board;
    cin >> id; cin.ignore();
    cin >> board_size; cin.ignore();
    field = vector<string>(board_size);
    string s;
    while(true) {
        s = "";
        rep(i, 8) {
            string x; cin >> x;
            s += x;
        }
        cin >> action_count; cin.ignore();
        put = vector<string>(action_count);
        for (int i = 0; i < action_count; i++) {
            cin >> put[i]; cin.ignore();
        }
        int_64 black = 0, white = 0;
        rep(i, 64) {
            if(s[i] == '0') black |= (1ll << (63 - i));
            else if(s[i] == '1') white |= (1ll << (63 - i));
        }
        if(id == 0) board.playerboard = black, board.opponentboard = white;
        else if(id == 1) board.playerboard = white, board.opponentboard = black;

        board.counter++;
        int_64 m = board.get_legal_board();
        
        // 1手読みAI
        int p = search(board, 7);
        cout << (char)((char)(p % 8) + 'a') << p / 8 + 1 << endl;

        board.counter++;
    }
    return 0;
}