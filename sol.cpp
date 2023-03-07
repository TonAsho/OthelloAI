#include <iostream>
#include <bitset>
using namespace std;
#define int_64 uint64_t
#define rep(i, n) for (int i = 0; i < n; ++i)

const int H = 8, W = 8;


struct Board {
    int_64 PlayerBoard;
    int_64 OpponentBoard;
    int_64 top_bottom_Mask;
    int_64 right_left_Mask;
    int_64 all_side_Mask;
    Board() {
        PlayerBoard   =   0x0000000810000000;
        OpponentBoard =   0x0000001008000000;
        top_bottom_Mask = 0x00FFFFFFFFFFFF00;
        right_left_Mask = 0x7e7e7e7e7e7e7e7e;
        all_side_Mask =   0x007e7e7e7e7e7e00;
    }

    //　can_put(player, opponent)置ける場所をビットで返す. m は着手箇所、1ビットのみが1で他はすべて0
    int_64 can_put(int_64 playerboard, int_64 opponentboard) {
        int_64 tmp;
        int_64 x = x = opponentboard & right_left_Mask;
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
        legal_board = (tmp >> 1) & blank_board;

        // 上方向
        // 下方向
        // 右斜め上方向
        // 右斜め下方向
        // 左斜め上方向
        // 左斜め下方向


        return legal_board;
    }

    //　put(int_64, index) = int_64のindexに駒を置く
    void put(int_64 &x, int index) {
        x = x | (1ll << (63 - index));
    }

    // print(black, white) = ８✖️８のマスに変換して表示する
    void print(int_64 black, int_64 white) {
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

    // xのint_64における立っているbitの個数
    int pop_count(int_64 &x) {
        x = x - ((x >> 1) & 0x5555555555555555);
	    x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);
	    x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0F;
	    x = (x * 0x0101010101010101) >> 56;
        return x;
    }
};

int main() {
    Board x;
    return 0;
}