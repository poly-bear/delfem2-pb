/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */


#ifndef DFM2_CAD2_DTRI_H
#define DFM2_CAD2_DTRI_H

#include "delfem2/dfm2_inline.h"
#include "delfem2/dtri2_v2dtri.h"
#include "delfem2/vec2.h"
#include "delfem2/cadtopo.h"
#include "delfem2/srchbv2aabb.h"

namespace delfem2 {

class CCad2D_VtxGeo {
 public:
  CCad2D_VtxGeo(const CVec2d &p) : pos(p) {}
 public:
  CVec2d pos;
};

/**
 * @details this class should be independent from any other classes except for "CVector2" and "CBoundingBox2D"
 * std::vector<CCad2D_EdgeGeo> will stands for a loop of curves
 */
class CCad2D_EdgeGeo {
 public:
  enum EDGE_TYPE {
    LINE = 0,
    BEZIER_CUBIC = 1,
    BEZIER_QUADRATIC = 2,
  };
  CCad2D_EdgeGeo() {
    type_edge = LINE;
  }
  CCad2D_EdgeGeo(const CCad2D_EdgeGeo &e) {
    this->p0 = e.p0;
    this->p1 = e.p1;
    this->type_edge = e.type_edge;
    this->param = e.param;
  }

  void SetLine(){
    this->type_edge = EDGE_TYPE::LINE;
    this->param.clear();
  }

  void SetQuadraticBezierCurve(const CVec2d& pos0){
    this->type_edge = EDGE_TYPE::BEZIER_QUADRATIC;
    this->param = {
        pos0.x - p0.x,
        pos0.y - p0.y };
  }

  void SetCubicBezierCurve(const CVec2d& pos0, const CVec2d& pos1){
    this->type_edge = EDGE_TYPE::BEZIER_CUBIC;
    this->param = {
        pos0.x - p0.x,
        pos0.y - p0.y,
        pos1.x - p1.x,
        pos1.y - p1.y };
  }

  std::vector<double> GenMesh(unsigned int ndiv) const;

  double Distance(double x, double y) const;

  double LengthMesh() const;

  double LengthNDiv(unsigned int ndiv) const;

  CBoundingBox2<double> BB() const {
    CBoundingBox2<double> bb;
    bb.Add(p0.x, p0.y);
    bb.Add(p1.x, p1.y);
    // for (unsigned int ip = 0; ip < aP.size(); ++ip) { bb.Add(aP[ip].x, aP[ip].y); }
    return bb;
  }

  void Transform(double A[4]){
    this->MatVec2(p0.x,p0.y, A, p0.x, p0.y);
    this->MatVec2(p1.x,p1.y, A, p1.x, p1.y);
    if( type_edge == CCad2D_EdgeGeo::BEZIER_CUBIC ){
      assert( param.size() == 4);
      this->MatVec2(param[0],param[1],A,param[0],param[1]);
      this->MatVec2(param[2],param[3],A, param[2],param[3]);
    }
    if( type_edge == CCad2D_EdgeGeo::BEZIER_QUADRATIC ){
      assert( param.size() == 2);
      this->MatVec2(param[0],param[1],A,param[0],param[1]);
    }
  }

 public:
  CVec2d p0, p1;
  EDGE_TYPE type_edge; // 0: line, 1:Cubic Bezier 2: Quadratic Bezier
  std::vector<double> param;
 private:
  void MatVec2(double& x1, double& y1,
               const double A[4],
               double x0, double y0){
    x1 = A[0]*x0 + A[1]*y0;
    y1 = A[2]*x0 + A[3]*y0;
  }
};

double AreaLoop(
    const std::vector<CCad2D_EdgeGeo> &aEdge);

std::vector<CCad2D_EdgeGeo> InvertLoop(
    const std::vector<CCad2D_EdgeGeo> &aEdge);

std::vector<CCad2D_EdgeGeo> RemoveEdgeWithZeroLength(
    const std::vector<CCad2D_EdgeGeo> &aEdge);

CBoundingBox2<double> BB_LoopEdgeCad2D(
    const std::vector<CCad2D_EdgeGeo> &aEdge);

// ---------------------------------------------------------------------------------

/**
 * @brief class to define 2D shapes bounded by parametric curves
 */
class CCad2D {
 public:
  CCad2D() {
    ivtx_picked = -1;
    iedge_picked = -1;
    iface_picked = -1;
    is_draw_face = true;
  }

  void Clear() {
    aVtx.clear();
    aEdge.clear();
    topo.Clear();
  }

  void Pick(
      double x0, double y0,
      double view_height);

  void DragPicked(double p1x, double p1y, double p0x, double p0y);
  // --------------------
  // const method here
  std::vector<double> MinMaxXYZ() const;

  CBoundingBox2<double> BB() const;

  bool Check() const;

  int GetEdgeType(int iedge) const {
    assert(iedge >= 0 && iedge < (int) aEdge.size());
    return aEdge[iedge].type_edge;
  }

  // size_t nFace() const { return aFace.size(); }

  size_t nVtx() const { return aVtx.size(); }

  size_t nEdge() const { return aEdge.size(); }

  /**
   * @brief return std::vector of XY that bounds the face with index iface
   */
  std::vector<double> XY_VtxCtrl_Face(int iface) const;

  std::vector<double> XY_Vtx(int ivtx) const;

  std::vector<std::pair<int, bool> > Ind_Edge_Face(int iface) const;

  std::vector<unsigned int> Ind_Vtx_Face(int iface) const;

  std::vector<int> Ind_Vtx_Edge(int iedge) const;

  /**
   * @brief add index to aIdP if a point in aXY is on the edge
   */
  void GetPointsEdge(
      std::vector<int> &aIdP,
      const double *pXY, int np,
      const std::vector<int> &aIE,
      double tolerance) const;

  // ----------------------------------
  // geometric operations from here

  void AddPolygon(
      const std::vector<double> &aXY);

  void AddFace(
      const std::vector<CCad2D_EdgeGeo> &aEdge);

  void AddVtxFace(
      double x0, double y0, unsigned int ifc_add);

  void AddVtxEdge(
      double x, double y, unsigned int ie_add);

 public:
  CadTopo topo;
  //
  std::vector<CCad2D_VtxGeo> aVtx;
  std::vector<CCad2D_EdgeGeo> aEdge;

  unsigned int ivtx_picked;
  unsigned int iedge_picked;
  int iface_picked;
  int ipicked_elem;

  bool is_draw_face;
 private:
  void CopyVertexPositionsToEdges(){
    for(size_t ie=0;ie<topo.edges.size();++ie) {
      const int iv0 = topo.edges[ie].iv0;
      const int iv1 = topo.edges[ie].iv1;
      aEdge[ie].p0 = aVtx[iv0].pos;
      aEdge[ie].p1 = aVtx[iv1].pos;
    }
  }
};

/**
 * @brief mesher for 2 dimensional CAD
 */
class CMesher_Cad2D {
 public:
  CMesher_Cad2D() {
    edge_length = 0.1;
    nvtx = 0;
    nedge = 0;
    nface = 0;
  }
  void Meshing(
      CMeshDynTri2D &dmesh,
      const CCad2D &cad2d);

  std::vector<unsigned int> IndPoint_IndEdgeArray(
      const std::vector<int> &aIndEd,
      const CCad2D &cad2d);

  std::vector<int> IndPoint_IndFaceArray(
      const std::vector<int> &aIndFc,
      const CCad2D &cad2d);

  std::vector<unsigned int> IndPoint_IndEdge(
      const unsigned int ie,
      bool is_end_point,
      const CCad2D &cad2d);
 public:
  // inputs for meshing
  double edge_length;
  /**
   * @brief specifiation of how many divisions in the cad edge.
   * @details this specification has more priority than the this->edge_length
   */
  std::map<unsigned int, unsigned int> mapIdEd_NDiv;

  // --------------
  // output for meshing

  size_t nvtx;
  size_t nedge;
  size_t nface;

  /**
   * map point to the index to vertex, edge, and face
   */
  std::vector<unsigned int> aFlgPnt;

  /**
   * @brief map triangle index to cad face index
   * @details after calling "this->Meshing()", the size of "this->aFlgTri" should be equal to the number of all the triangles
   */
  std::vector<unsigned int> aFlgTri;

  std::vector< std::vector<std::pair<unsigned int, double> > > edge_point;
};

} // namespace delfem2

#ifndef DFM2_STATIC_LIBRARY
#  include "delfem2/cad2_dtri2.cpp"
#endif

#endif /* DFM2_CAD2_DTRI_H */
