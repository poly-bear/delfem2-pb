/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef DYNTRI_V2_H
#define DYNTRI_V2_H

#include <map>
#include <algorithm>
#include <stack>

#include "delfem2/vec2.h"
#include "delfem2/dyntri.h"

////////////////////////////////////////////////////////////////////////////////////////////////
// OpenGL dependency! these functions should be removed and put in another file

void DrawMeshDynTri_Edge(const std::vector<ETri>& aSTri,
                         const std::vector<CVector2>& aVec2);

void DrawMeshDynTri_FaceNorm(const std::vector<ETri>& aSTri,
                             const std::vector<CVector2>& aVec2);

//////////////////////////////////////////////////////////////////////////////////////////////////

void FixLoopOrientation(std::vector<int>& loopIP,
                        const std::vector<int>& loopIP_ind,
                        const std::vector<CVector2>& aXY);

void ResamplingLoop(std::vector<int>& loopIP1_ind,
                    std::vector<int>& loopIP1,
                    std::vector<CVector2>& aXY,
                    double max_edge_length);

void JArray_FromVecVec_XY(std::vector<int>& aIndXYs,
                          std::vector<int>& loopIP0,
                          std::vector<CVector2>& aXY,
                          const std::vector< std::vector<double> >& aaXY);

//////////////////////////////////////////////////////////////////////////////////////////////////

bool CheckTri(const std::vector<CEPo2>& aPo3D,
              const std::vector<ETri>& aSTri,
              const std::vector<CVector2>& aXYZ);

bool DelaunayAroundPoint(int ipo0,
                         std::vector<CEPo2>& aPo,
                         std::vector<ETri>& aTri,
                         const std::vector<CVector2>& aVec2);

void MeshTri2D_Export(std::vector<double>& aXY_out,
                      std::vector<int>& aTri_out,
                      const std::vector<CVector2>& aVec2,
                      const std::vector<ETri>& aTri_in);

bool CheckInputBoundaryForTriangulation(const std::vector<int>& loop_ind,
                                        const std::vector<CVector2>& aXY);

void Meshing_Initialize(std::vector<CEPo2>& aPo2D,
                        std::vector<ETri>& aTri,
                        std::vector<CVector2>& aVec2);

void FlagConnected(std::vector<int>& inout_flg,
                   const std::vector<ETri>& aTri_in,
                   int itri0_ker,
                   int iflag);

void DeleteTriFlag(std::vector<ETri>& aTri_in,
                    const std::vector<int>& inout_flg,
                    int flag);

void EnforceEdge(const std::vector<CVector2>& aVec2,
                 std::vector<CEPo2>& aPo2D,
                 std::vector<ETri>& aTri,
                 const std::vector<int>& aPtrVtxInd,
                 const std::vector<int>& aVtxInd);

void Meshing_SingleConnectedShape2D(std::vector<CEPo2>& aPo2D,
                                    std::vector<CVector2>& aVec2,
                                    std::vector<ETri>& aETri,
                                    const std::vector<int>& loopIP_ind,
                                    const std::vector<int>& loopIP);

void DeleteUnrefPoints(std::vector<CVector2>& aVec2,
                  std::vector<CEPo2>& aPo2D,
                  std::vector<ETri>& aTri_in,
                  const std::vector<int>& aPoDel);

void MakeInvMassLumped_Tri(std::vector<double>& aInvMassLumped,
                           double rho,
                           const std::vector<CVector2>& aVec2,
                           const std::vector<ETri>& aETri);
void MinMaxTriArea(double& min_area,
                   double& max_area,
                   const std::vector<CVector2>& aVec2,
                   const std::vector<ETri>& aETri);
void MakeMassMatrixTri(double M[9],
                       double rho,
                       const int aIP[3],
                       const std::vector<CVector2>& aVec2);

void CMeshTri2D(std::vector<double>& aXY,
                std::vector<int>& aTri,
                std::vector<CVector2>& aVec2,
                std::vector<ETri>& aETri);

class CInputTriangulation
{
public:
  virtual double edgeLengthRatio(double px, double py) const = 0;
};

class CInputTriangulation_Uniform : public CInputTriangulation {
public:
  CInputTriangulation_Uniform(double elen): elen(elen){}
  virtual double edgeLengthRatio(double px, double py) const {
    return 1.0;
  }
public:
  double elen;
};

void MeshingInside(std::vector<CEPo2>& aPo2D,
                    std::vector<ETri>& aTri,
                    std::vector<CVector2>& aVec2,
                    const std::vector<int>& aVtxInd,
                    const double len,
                    const CInputTriangulation& mesh_density);


class CCmdRefineMesh
{
public:
  class CCmdEdge
  {
  public:
    CCmdEdge(int i0, int i1, double s0){
      if( i0 < i1 ){ ipo0 = i0; ipo1 = i1; r0 = s0;   }
      else{          ipo0 = i1; ipo1 = i0; r0 = 1-s0; }
    }
    bool operator < (const CCmdEdge& rhs) const {
      if( ipo0 != rhs.ipo0 ){ return ipo0 < rhs.ipo0; }
      return ipo1 < rhs.ipo1;
    }
  public:
    int ipo_new, ipo0, ipo1;
    double r0;
  };
public:
  void Interpolate(double* pVal, int np, int ndim) const {
    for(unsigned int icmd=0;icmd<aCmdEdge.size();++icmd){
      const int i0 = aCmdEdge[icmd].ipo0; assert( i0>=0 && i0<np );
      const int i1 = aCmdEdge[icmd].ipo1; assert( i1>=0 && i1<np );
      const int i2 = aCmdEdge[icmd].ipo_new;
      if( i2 >= np || i2 < 0 ){ continue; }
      double r0 = aCmdEdge[icmd].r0;
      for(int idim=0;idim<ndim;++idim){
        pVal[i2*ndim+idim] = r0*pVal[i0*ndim+idim] + (1-r0)*pVal[i1*ndim+idim];
      }
    }
  }
public:
  std::vector<CCmdEdge> aCmdEdge;
};

/*
class CMapper
{
public:
  void Print(){
    const int np = iv_ind.size()-1;
    for(int ip=0;ip<np;++ip){
      std::cout << ip << " --> ";
      for(int iip=iv_ind[ip];iip<iv_ind[ip+1];++iip){
        std::cout << iv[iip] << " " << w[iip] << "    ";
      }
      std::cout << std::endl;
    }
  }
public:
  int nv_in;
  std::vector<int> iv_ind;
  std::vector<int> iv;
  std::vector<double> w;
};
 */

void RefinementPlan_EdgeLongerThan_InsideCircle(CCmdRefineMesh& aCmd,
                                                double elen,
                                                double px, double py, double rad,
                                                const std::vector<CEPo2>& aPo2D,
                                                const std::vector<CVector2>& aVec2,
                                                const std::vector<ETri>& aETri);

void RefineMesh(std::vector<CEPo2>& aPo3D,
                std::vector<ETri>& aSTri,
                std::vector<CVector2>& aVec2,
                CCmdRefineMesh& aCmd);

class CMeshDynTri2D{
public:
  void Initialize(const double* aXY, int nPo,
                  const int* aTri, int nTri)
  {
    aVec2.resize(nPo);
    for(int ipo=0;ipo<nPo;ipo++){
      aVec2[ipo].x = aXY[ipo*2+0];
      aVec2[ipo].y = aXY[ipo*2+1];
    }
    InitializeMesh(aEPo, aETri,
                   aTri, nTri, nPo);
  }
  void Check()
  {
    CheckTri(aETri);
    CheckTri(aEPo, aETri);
    CheckTri(aEPo, aETri, aVec2);
  }
  std::vector<double> MinMax_XYZ() const {
    double x_min,x_max, y_min,y_max;
    x_min=x_max=aVec2[0].x;
    y_min=y_max=aVec2[0].y;
    for(unsigned int ipo=0;ipo<aEPo.size();ipo++){
      const double x = aVec2[ipo].x;
      const double y = aVec2[ipo].y;
      x_min = (x_min < x) ? x_min : x;
      x_max = (x_max > x) ? x_max : x;
      y_min = (y_min < y) ? y_min : y;
      y_max = (y_max > y) ? y_max : y;
    }
    std::vector<double> bb;
    bb.push_back(x_min);
    bb.push_back(x_max);
    bb.push_back(y_min);
    bb.push_back(y_max);
    bb.push_back(0.0);
    bb.push_back(0.0);
    return bb;
  }
  int insertPointElem(int itri0, double r0, double r1){
    CVector2 v2;
    {
      int i0 = aETri[itri0].v[0];
      int i1 = aETri[itri0].v[1];
      int i2 = aETri[itri0].v[2];
      v2 = r0*aVec2[i0]+r1*aVec2[i1]+(1-r0-r1)*aVec2[i2];
    }
    const int ipo0 = aEPo.size();
    aVec2.push_back(v2);
    aEPo.push_back(CEPo2());
    InsertPoint_Elem(ipo0, itri0, aEPo, aETri);
    return ipo0;
  }
  void DelaunayAroundPoint(int ipo){
    ::DelaunayAroundPoint(ipo, aEPo, aETri, aVec2);
  }
  void meshing_loops(const std::vector< std::vector<double> >& aaXY,
                     double edge_length)
  {
    std::vector<int> loopIP_ind, loopIP;
    {
      JArray_FromVecVec_XY(loopIP_ind,loopIP, aVec2,
                           aaXY);
      if( !CheckInputBoundaryForTriangulation(loopIP_ind,aVec2) ){
        return;
      }
      FixLoopOrientation(loopIP,
                         loopIP_ind,aVec2);
      if( edge_length > 10e-10 ){
        ResamplingLoop(loopIP_ind,loopIP,aVec2,
                       edge_length );
      }
    }
    ////
    Meshing_SingleConnectedShape2D(aEPo, aVec2, aETri,
                                   loopIP_ind,loopIP);
    if( edge_length > 1.0e-10 ){
      CInputTriangulation_Uniform param(1.0);
      MeshingInside(aEPo,aETri,aVec2, loopIP,
                    edge_length, param);
    }
  }
  void RefinementPlan_EdgeLongerThan_InsideCircle(CCmdRefineMesh& aCmd,
                                                  double elen,
                                                  double px, double py, double rad){
//    CCmdRefineMesh aCmd;
    ::RefinementPlan_EdgeLongerThan_InsideCircle(aCmd,
                                                 elen,px,py, rad,
                                                 aEPo,aVec2,aETri);
    const int np0 = aVec2.size();
    RefineMesh(aEPo, aETri, aVec2, aCmd);
    assert( aEPo.size() == aVec2.size() );
    /*
    /////
    const int np = aVec2.size();
    mpr.nv_in = np0;
    mpr.iv_ind.resize(np+1);
    mpr.iv_ind[0] = 0;
    for(unsigned int icmd=0;icmd<aCmd.size();++icmd){
      const int ip = aCmd[icmd].ipo_new;
      mpr.iv_ind[ip+1] = 2;
    }
    for(int ip=0;ip<np;++ip){
      if( mpr.iv_ind[ip+1] == 0 ){ mpr.iv_ind[ip+1] = 1; }
    }
    for(int ip=0;ip<np;++ip){
      mpr.iv_ind[ip+1] += mpr.iv_ind[ip];
    }
    const int nmap = mpr.iv_ind[np];
    mpr.iv.resize(nmap);
    mpr.w.resize(nmap);
    for(int ip=0;ip<np;++ip){
      if( mpr.iv_ind[ip+1] - mpr.iv_ind[ip] != 1 ){ continue; }
      int i0 = mpr.iv_ind[ip];
      mpr.iv[i0] = ip;
      mpr.w[i0] = 1.0;
      mpr.iv_ind[ip] += 1;
    }
    for(unsigned int icmd=0;icmd<aCmd.size();++icmd){
      const int ip = aCmd[icmd].ipo_new;
      const int i0 = mpr.iv_ind[ip];
      mpr.iv[i0+0] = aCmd[icmd].ipo0;
      mpr.iv[i0+1] = aCmd[icmd].ipo1;
      mpr.w[i0+0] = aCmd[icmd].r0;
      mpr.w[i0+1] = aCmd[icmd].r1;
      mpr.iv_ind[ip] += 2;
    }
    for(int ip=np-1;ip>=0;--ip){
      mpr.iv_ind[ip+1] = mpr.iv_ind[ip];
    }
    mpr.iv_ind[0] = 0;
     */
  }
  void Draw_FaceNorm()const { DrawMeshDynTri_FaceNorm(aETri,aVec2); }
  void Draw_Edge() const { DrawMeshDynTri_Edge(aETri,aVec2); }
  void draw() const { this->Draw_Edge(); }
  int nTri() const { return aETri.size(); }
  int nPoint() const { return aEPo.size(); }
  void DeleteTriEdge(int itri, int iedge){ Collapse_ElemEdge(itri, iedge, aEPo, aETri); }
public:
  std::vector<CEPo2> aEPo;
  std::vector<ETri> aETri;
  std::vector<CVector2> aVec2;
};

#endif // #endif SURFACE_MESH_H
