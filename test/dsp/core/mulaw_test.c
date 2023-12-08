/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│ vi: set et ft=c ts=2 sts=2 sw=2 fenc=utf-8                               :vi │
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2021 Justine Alexandra Roberts Tunney                              │
│                                                                              │
│ Permission to use, copy, modify, and/or distribute this software for         │
│ any purpose with or without fee is hereby granted, provided that the         │
│ above copyright notice and this permission notice appear in all copies.      │
│                                                                              │
│ THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL                │
│ WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED                │
│ WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE             │
│ AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL         │
│ DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR        │
│ PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER               │
│ TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR             │
│ PERFORMANCE OF THIS SOFTWARE.                                                │
╚─────────────────────────────────────────────────────────────────────────────*/
#include "dsp/core/core.h"
#include "dsp/core/ituround.h"
#include "libc/assert.h"
#include "libc/macros.internal.h"
#include "libc/math.h"
#include "libc/stdio/rand.h"
#include "libc/stdio/stdio.h"
#include "libc/testlib/ezbench.h"
#include "libc/testlib/testlib.h"

TEST(mulaw, outOfRange) {
  EXPECT_EQ(0, mulaw(-32769));
  EXPECT_EQ(128, mulaw(32768));
  EXPECT_EQ(0, mulaw(-131072));
  EXPECT_EQ(128, mulaw(131072));
}

TEST(unmulaw, outOfRange_modulo256) {
  EXPECT_EQ(8, unmulaw(-2));
  EXPECT_EQ(8, unmulaw(-2 & 255));
  EXPECT_EQ(31100, unmulaw(-127));
  EXPECT_EQ(31100, unmulaw(-127 & 255));
}

TEST(mulaw, regressiontest) {
  int i;
  static const int V[][2] = {
      {13, -18474}, {143, 17241}, {131, 28876}, {10, -21786}, {146, 15194},
      {16, -16141}, {137, 23194}, {10, -22282}, {157, 9513},  {128, 31674},
      {143, 16891}, {13, -19214}, {15, -17028}, {172, 4896},  {11, -21318},
      {131, 28684}, {149, 13594}, {197, 1567},  {132, 27860}, {135, 25141},
      {147, 14699}, {144, 15757}, {140, 19422}, {29, -9374},  {4, -28099},
      {137, 22448}, {176, 3876},  {7, -24928},  {157, 9553},  {167, 6048},
      {133, 26527}, {9, -23278},  {29, -9188},  {24, -12091}, {136, 23694},
      {169, 5722},  {17, -15670}, {149, 13674}, {135, 24796}, {156, 10013},
      {201, 1279},  {144, 15844}, {21, -13686}, {8, -24136},  {161, 7629},
      {18, -15082}, {29, -9484},  {11, -20731}, {131, 28975}, {158, 8866},
      {134, 25514}, {36, -6936},  {29, -9348},  {167, 6082},  {25, -11235},
      {139, 20776}, {45, -4650},  {236, 179},   {141, 18807}, {10, -21498},
      {9, -22455},  {141, 19222}, {1, -31252},  {7, -25035},  {87, -665},
      {134, 25759}, {30, -9037},  {13, -18312}, {13, -18782}, {139, 21031},
      {187, 2466},  {141, 19229}, {131, 29425}, {7, -25428},  {10, -22393},
      {203, 1156},  {13, -19223}, {155, 10566}, {133, 27311}, {45, -4603},
      {134, 25985}, {161, 7595},  {1, -31305},  {130, 30274}, {133, 27410},
      {24, -11919}, {130, 29752}, {40, -5901},  {9, -22472},  {145, 15486},
      {62, -2154},  {172, 4888},  {140, 19967}, {142, 17750}, {7, -25279},
      {56, -2846},  {10, -21643}, {2, -30140},  {201, 1307},  {0, -32480},
  };
  for (i = 0; i < ARRAYLEN(V); ++i) {
    EXPECT_EQ(V[i][0], mulaw(V[i][1]));
  }
}

TEST(unmulaw, test) {
  int i;
  static const int V[][2] = {
      {-32124, 0},  {-31100, 1},  {-30076, 2},  {-29052, 3},  {-28028, 4},
      {-27004, 5},  {-25980, 6},  {-24956, 7},  {-23932, 8},  {-22908, 9},
      {-21884, 10}, {-20860, 11}, {-19836, 12}, {-18812, 13}, {-17788, 14},
      {-16764, 15}, {-15996, 16}, {-15484, 17}, {-14972, 18}, {-14460, 19},
      {-13948, 20}, {-13436, 21}, {-12924, 22}, {-12412, 23}, {-11900, 24},
      {-11388, 25}, {-10876, 26}, {-10364, 27}, {-9852, 28},  {-9340, 29},
      {-8828, 30},  {-8316, 31},  {-7932, 32},  {-7676, 33},  {-7420, 34},
      {-7164, 35},  {-6908, 36},  {-6652, 37},  {-6396, 38},  {-6140, 39},
      {-5884, 40},  {-5628, 41},  {-5372, 42},  {-5116, 43},  {-4860, 44},
      {-4604, 45},  {-4348, 46},  {-4092, 47},  {-3900, 48},  {-3772, 49},
      {-3644, 50},  {-3516, 51},  {-3388, 52},  {-3260, 53},  {-3132, 54},
      {-3004, 55},  {-2876, 56},  {-2748, 57},  {-2620, 58},  {-2492, 59},
      {-2364, 60},  {-2236, 61},  {-2108, 62},  {-1980, 63},  {-1884, 64},
      {-1820, 65},  {-1756, 66},  {-1692, 67},  {-1628, 68},  {-1564, 69},
      {-1500, 70},  {-1436, 71},  {-1372, 72},  {-1308, 73},  {-1244, 74},
      {-1180, 75},  {-1116, 76},  {-1052, 77},  {-988, 78},   {-924, 79},
      {-876, 80},   {-844, 81},   {-812, 82},   {-780, 83},   {-748, 84},
      {-716, 85},   {-684, 86},   {-652, 87},   {-620, 88},   {-588, 89},
      {-556, 90},   {-524, 91},   {-492, 92},   {-460, 93},   {-428, 94},
      {-396, 95},   {-372, 96},   {-356, 97},   {-340, 98},   {-324, 99},
      {-308, 100},  {-292, 101},  {-276, 102},  {-260, 103},  {-244, 104},
      {-228, 105},  {-212, 106},  {-196, 107},  {-180, 108},  {-164, 109},
      {-148, 110},  {-132, 111},  {-120, 112},  {-112, 113},  {-104, 114},
      {-96, 115},   {-88, 116},   {-80, 117},   {-72, 118},   {-64, 119},
      {-56, 120},   {-48, 121},   {-40, 122},   {-32, 123},   {-24, 124},
      {-16, 125},   {-8, 126},    {0, 127},     {32124, 128}, {31100, 129},
      {30076, 130}, {29052, 131}, {28028, 132}, {27004, 133}, {25980, 134},
      {24956, 135}, {23932, 136}, {22908, 137}, {21884, 138}, {20860, 139},
      {19836, 140}, {18812, 141}, {17788, 142}, {16764, 143}, {15996, 144},
      {15484, 145}, {14972, 146}, {14460, 147}, {13948, 148}, {13436, 149},
      {12924, 150}, {12412, 151}, {11900, 152}, {11388, 153}, {10876, 154},
      {10364, 155}, {9852, 156},  {9340, 157},  {8828, 158},  {8316, 159},
      {7932, 160},  {7676, 161},  {7420, 162},  {7164, 163},  {6908, 164},
      {6652, 165},  {6396, 166},  {6140, 167},  {5884, 168},  {5628, 169},
      {5372, 170},  {5116, 171},  {4860, 172},  {4604, 173},  {4348, 174},
      {4092, 175},  {3900, 176},  {3772, 177},  {3644, 178},  {3516, 179},
      {3388, 180},  {3260, 181},  {3132, 182},  {3004, 183},  {2876, 184},
      {2748, 185},  {2620, 186},  {2492, 187},  {2364, 188},  {2236, 189},
      {2108, 190},  {1980, 191},  {1884, 192},  {1820, 193},  {1756, 194},
      {1692, 195},  {1628, 196},  {1564, 197},  {1500, 198},  {1436, 199},
      {1372, 200},  {1308, 201},  {1244, 202},  {1180, 203},  {1116, 204},
      {1052, 205},  {988, 206},   {924, 207},   {876, 208},   {844, 209},
      {812, 210},   {780, 211},   {748, 212},   {716, 213},   {684, 214},
      {652, 215},   {620, 216},   {588, 217},   {556, 218},   {524, 219},
      {492, 220},   {460, 221},   {428, 222},   {396, 223},   {372, 224},
      {356, 225},   {340, 226},   {324, 227},   {308, 228},   {292, 229},
      {276, 230},   {260, 231},   {244, 232},   {228, 233},   {212, 234},
      {196, 235},   {180, 236},   {164, 237},   {148, 238},   {132, 239},
      {120, 240},   {112, 241},   {104, 242},   {96, 243},    {88, 244},
      {80, 245},    {72, 246},    {64, 247},    {56, 248},    {48, 249},
      {40, 250},    {32, 251},    {24, 252},    {16, 253},    {8, 254},
      {0, 255},
  };
  for (i = 0; i < ARRAYLEN(V); ++i) {
    EXPECT_EQ(V[i][0], unmulaw(V[i][1]));
  }
}

BENCH(mulaw, bench) {
  EZBENCH2("mulaw -32k", donothing, mulaw(-32768));
  EZBENCH2("mulaw 32k", donothing, mulaw(32767));
  EZBENCH2("unmulaw 0", donothing, unmulaw(0));
  EZBENCH2("unmulaw 255", donothing, unmulaw(255));
}
