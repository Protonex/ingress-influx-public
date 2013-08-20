#ifndef S2_H
#define S2_H

#include "../pch.h"
#include "mathutil.h"

namespace S2
{

static int const kFaceBits = 3;
static int const kNumFaces = 6;
static int const kMaxCellLevel = 30;
static int const kMaxLevel = kMaxCellLevel;
static int const kMaxSize = 1 << kMaxLevel;
static int const kPosBits = 2 * kMaxLevel + 1;

static int const kSwapMask = 0x01;
static int const kInvertMask = 0x02;
static int const kLookupBits = 4;

static quint16 lookup_pos[1 << (2 * kLookupBits + 2)];
static quint16 lookup_ij[1 << (2 * kLookupBits + 2)];

int const kPosToIJ[4][4] = {
  // 0  1  2  3
  {  0, 1, 3, 2 },    // canonical order:    (0,0), (0,1), (1,1), (1,0)
  {  0, 2, 3, 1 },    // axes swapped:       (0,0), (1,0), (1,1), (0,1)
  {  3, 2, 0, 1 },    // bits inverted:      (1,1), (1,0), (0,0), (0,1)
  {  3, 1, 0, 2 },    // swapped & inverted: (1,1), (0,1), (0,0), (1,0)
};
int const kPosToOrientation[4] = {
  kSwapMask,
  0,
  0,
  kInvertMask + kSwapMask,
};

quint64 FromFaceIJ(int face, int i, int j);
quint64 PosToCellID(QPointF p_xPos);
QPointF CellIDToPos(quint64 id);
QString PosToE6(QPointF p_xPos);
bool CellIDIsLeaf(quint64 id);
int CellIDLevel(quint64 id);
quint64 CellIDParent(quint64 id,int level);
float PosDistance(QPointF pa, QPointF pb);

} //namespace S2

#endif // S2_H
