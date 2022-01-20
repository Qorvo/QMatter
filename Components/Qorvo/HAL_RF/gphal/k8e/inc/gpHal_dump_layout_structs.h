
struct rangelist {
    gpHal_Address_t startAddress;
    gpHal_Address_t endAddress;
};


struct rangelist plme[] =
{
        {0x0000, 0x0007},
        {0x0009, 0x000e},
        {0x0010, 0x0012},
        {0x0014, 0x0014},
        {0x0016, 0x001c},
        {0x001e, 0x002c},
        {0x002e, 0x0031}
};

struct rangelist rx[] =
{
        {0x0080, 0x008b},
        {0x008d, 0x008e},
        {0x0090, 0x00a6},
        {0x00a8, 0x00aa},
        {0x00ac, 0x00ae},
        {0x00b0, 0x00b2},
        {0x00b4, 0x00be},
        {0x00c0, 0x00c2},
        {0x00c4, 0x00c4}
};

struct rangelist tx[] =
{
        {0x0100, 0x010b}
};

struct rangelist trx[] =
{
        {0x0180, 0x01b8},
        {0x01ba, 0x01be},
        {0x01c0, 0x01c4},
        {0x01c6, 0x01ce},
        {0x01d0, 0x01d4},
        {0x01d6, 0x01d9}
};

struct rangelist radioitf[] =
{
        {0x0200, 0x0208},
        {0x020a, 0x0216},
        {0x0218, 0x022a},
        {0x022c, 0x0233}
};

struct rangelist rib[] =
{
        {0x0400, 0x0409},
        {0x040c, 0x0417},
        {0x041a, 0x0422},
        {0x0426, 0x0433},
        {0x0435, 0x0436},
        {0x0438, 0x0448},
        {0x044a, 0x044e}
};

struct rangelist qta[] =
{
        {0x0480, 0x048e},
        {0x0491, 0x0493}
};

struct rangelist radio_arb[] =
{
        {0x04c0, 0x04c1}
};

struct rangelist parfcs[] =
{
        {0x04d0, 0x04d1},
        {0x04d3, 0x04d3},
        {0x04d5, 0x04d5},
        {0x04d7, 0x04dc}
};

struct rangelist parble[] =
{
        {0x04e0, 0x04e0},
        {0x04e2, 0x04e9},
        {0x04eb, 0x04f9},
        {0x04fb, 0x04fe}
};

struct rangelist rci[] =
{
        {0x0500, 0x0500},
        {0x050b, 0x054d}
};

struct rangelist prg[] =
{
        {0x0580, 0x0580},
        {0x0582, 0x0584}
};

struct rangelist pmud[] =
{
        {0x0600, 0x0606},
        {0x0608, 0x0626}
};

struct rangelist iob[] =
{
        {0x0700, 0x0712},
        {0x0714, 0x0718},
        {0x071a, 0x071c},
        {0x071e, 0x0722}
};

struct rangelist standby[] =
{
        {0x0800, 0x0803},
        {0x0806, 0x080a},
        {0x080c, 0x0810},
        {0x0812, 0x081b},
        {0x081d, 0x0829},
        {0x082b, 0x082c},
        {0x0832, 0x0852},
        {0x0854, 0x0856},
        {0x0858, 0x085a},
        {0x085c, 0x0860}
};

struct rangelist mm[] =
{
        {0x0900, 0x094c},
        {0x094e, 0x0957},
        {0x0959, 0x096c},
        {0x096e, 0x096e}
};

struct rangelist pbm_adm[] =
{
        {0x0980, 0x0987},
        {0x0989, 0x098d}
};

struct rangelist msi[] =
{
        {0x0a00, 0x0a04}
};

struct rangelist int_ctrl[] =
{
        {0x0a80, 0x0a80},
        {0x0a82, 0x0a86},
        {0x0a88, 0x0abc},
        {0x0abe, 0x0ae0}
};

struct rangelist es[] =
{
        {0x0b00, 0x0b00},
        {0x0b02, 0x0b03},
        {0x0b05, 0x0b06},
        {0x0b08, 0x0b16},
        {0x0b18, 0x0b32},
        {0x0b34, 0x0b35},
        {0x0b3a, 0x0b40},
        {0x0b42, 0x0b43},
        {0x0b45, 0x0b48},
        {0x0b4a, 0x0b53},
        {0x0b56, 0x0b57},
        {0x0b5a, 0x0b5a},
        {0x0b5c, 0x0b63}
};

struct rangelist gpio[] =
{
        {0x0c00, 0x0c12},
        {0x0c14, 0x0c19}
};

struct rangelist i2c_m[] =
{
        {0x0c20, 0x0c25},
        {0x0c27, 0x0c27}
};

struct rangelist adcif[] =
{
        {0x0c40, 0x0c4d},
        {0x0c52, 0x0c5c},
        {0x0c5e, 0x0c64}
};

struct rangelist ssp[] =
{
        {0x0c80, 0x0c8a},
        {0x0c8c, 0x0c92},
        {0x0c94, 0x0c97},
        {0x0c99, 0x0c99}
};

struct rangelist uart_0[] =
{
        {0x0ca4, 0x0ca9}
};

struct rangelist uart_1[] =
{
        {0x0cb4, 0x0cb9}
};

struct rangelist uart_2[] =
{
        {0x0cc4, 0x0cc9}
};

struct rangelist spi_m[] =
{
        {0x0cd4, 0x0cd9}
};

struct rangelist keypad[] =
{
        {0x0ce0, 0x0ce0},
        {0x0ce2, 0x0ced},
        {0x0cef, 0x0cf9},
        {0x0cfb, 0x0cfb}
};

struct rangelist ir[] =
{
        {0x0d00, 0x0d08},
        {0x0d0a, 0x0d0d},
        {0x0d10, 0x0d13}
};

struct rangelist watchdog[] =
{
        {0x0d20, 0x0d20},
        {0x0d22, 0x0d23},
        {0x0d25, 0x0d25},
        {0x0d27, 0x0d29}
};

struct rangelist led[] =
{
        {0x0d30, 0x0d3b}
};

struct rangelist pwm[] =
{
        {0x0d40, 0x0d53},
        {0x0d56, 0x0d5a},
        {0x0d5c, 0x0d6a},
        {0x0d6f, 0x0d6f},
        {0x0d73, 0x0d73},
        {0x0d77, 0x0d77},
        {0x0d7b, 0x0d7b}
};

struct rangelist ipc[] =
{
        {0x0d90, 0x0d96},
        {0x0d9b, 0x0d9c},
        {0x0da1, 0x0da2}
};

struct rangelist spi_sl[] =
{
        {0x0dc2, 0x0dc5}
};

struct rangelist i2c_sl[] =
{
        {0x0de2, 0x0de4},
        {0x0de6, 0x0de7}
};

struct rangelist dma_0[] =
{
        {0x0e00, 0x0e02},
        {0x0e04, 0x0e06},
        {0x0e08, 0x0e13},
        {0x0e16, 0x0e1a}
};

struct rangelist dma_1[] =
{
        {0x0e20, 0x0e22},
        {0x0e24, 0x0e26},
        {0x0e28, 0x0e33},
        {0x0e36, 0x0e3a}
};

struct rangelist i2s_m[] =
{
        {0x0e42, 0x0e48}
};

struct rangelist coex[] =
{
        {0x0e60, 0x0e67}
};

struct rangelist asp[] =
{
        {0x0e70, 0x0e74},
        {0x0e7a, 0x0e7a},
        {0x0e7c, 0x0e7c}
};

struct rangelist gpmicro[] =
{
        {0x1000, 0x1006},
        {0x1008, 0x100c},
        {0x100e, 0x100f}
};

struct rangelist cortexm4[] =
{
        {0x1100, 0x1102},
        {0x1104, 0x1106},
        {0x1108, 0x110a},
        {0x110c, 0x110c}
};

struct rangelist macfilt[] =
{
        {0x42800, 0x4283c},
        {0x4283e, 0x4284c}
};

struct rangelist ble_mgr[] =
{
        {0x42880, 0x428aa},
        {0x428ac, 0x428be},
        {0x428c0, 0x428ea},
        {0x428ec, 0x428f8},
        {0x428fa, 0x428ff}
};

struct rangelist fll_table[] =
{
        {0x42900, 0x4292f}
};

struct rangelist nvr_1[] =
{
        {0x1c0000, 0x1c0003},
        {0x1c0008, 0x1c0065},
        {0x1c006c, 0x1c0109},
        {0x1c010b, 0x1c0124},
        {0x1c0130, 0x1c0143},
        {0x1c0150, 0x1c0173},
        {0x1c0178, 0x1c01ff}
};

struct rangelist nvr_2[] =
{
        {0x1c0200, 0x1c0205},
        {0x1c03e0, 0x1c03ff}
};

struct rangelist event[] =
{
        {0x0000, 0x000f}
};

struct rangelist pbm_format_t[] =
{
        {0x0000, 0x0014},
        {0x0016, 0x0019}
};

struct rangedescription {
    struct rangelist* rlp;
    UInt16 rangesize;
};

struct rangedescription layoutlist[] = {
        { plme, 7 },
        { rx, 9 },
        { tx, 1 },
        { trx, 6 },
        { radioitf, 4 },
        { rib, 7 },
        { qta, 2 },
        { radio_arb, 1 },
        { parfcs, 4 },
        { parble, 4 },
        { rci, 2 },
        { prg, 2 },
        { pmud, 2 },
        { iob, 4 },
        { standby, 10 },
        { mm, 4 },
        { pbm_adm, 2 },
        { msi, 1 },
        { int_ctrl, 4 },
        { es, 13 },
        { gpio, 2 },
        { i2c_m, 2 },
        { adcif, 3 },
        { ssp, 4 },
        { uart_0, 1 },
        { uart_1, 1 },
        { uart_2, 1 },
        { spi_m, 1 },
        { keypad, 4 },
        { ir, 3 },
        { watchdog, 4 },
        { led, 1 },
        { pwm, 7 },
        { ipc, 3 },
        { spi_sl, 1 },
        { i2c_sl, 2 },
        { dma_0, 4 },
        { dma_1, 4 },
        { i2s_m, 1 },
        { coex, 1 },
        { asp, 3 },
        { gpmicro, 3 },
        { cortexm4, 4 },
        { macfilt, 2 },
        { ble_mgr, 5 },
        { fll_table, 1 },
        { nvr_1, 7 },
        { nvr_2, 2 },
        { event, 1 },
        { pbm_format_t, 2 }
};

