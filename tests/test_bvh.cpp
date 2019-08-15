/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <iostream>

#include "gtest/gtest.h"

#include "delfem2/vec3.h"
#include "delfem2/bv.h"
#include "delfem2/bvh.h"
#include "delfem2/sdf.h"
#include "delfem2/primitive.h"
#include "delfem2/msh.h"

#include "delfem2/srchuni_v3.h"
#include "delfem2/objfunc_v23.h"
#include "delfem2/srch_v3bvhmshtopo.h"

#ifndef M_PI
#define M_PI 3.14159265359
#endif

TEST(bvh,inclusion)
{
  std::vector<double> aXYZ;
  std::vector<unsigned int> aTri;
  { // make a unit sphere
    MeshTri3D_Sphere(aXYZ, aTri, 1.0, 64, 32);
    Rotate(aXYZ, 0.2, 0.3, 0.4);
  }
  //  std::cout << "ntri: " << aTri.size()/3 << std::endl;
  CBVH_MeshTri3D<CBV3D_Sphere> bvh;
  bvh.Init(aXYZ, aTri, 0.03);
  for(int itr=0;itr<10000;++itr){
    CVector3 p0;
    {
      p0.x = 2.0*(rand()/(RAND_MAX+1.0)-0.5);
      p0.y = 2.0*(rand()/(RAND_MAX+1.0)-0.5);
      p0.z = 2.0*(rand()/(RAND_MAX+1.0)-0.5);
    }
    for(int ibvh=0;ibvh<bvh.aNodeBVH.size();++ibvh){
      const CBV3D_Sphere& bv = bvh.aBB_BVH[ibvh];
      const CNodeBVH& node = bvh.aNodeBVH[ibvh];
      bool is_intersect = bv.isInclude_Point(p0.x, p0.y, p0.z);
      if( !is_intersect && node.ichild[1] != -1 ){ // branch
        const int ichild0 = node.ichild[0];
        const int ichild1 = node.ichild[1];
        EXPECT_FALSE( bvh.aBB_BVH[ichild0].isInclude_Point(p0.x, p0.y, p0.z) );
        EXPECT_FALSE( bvh.aBB_BVH[ichild1].isInclude_Point(p0.x, p0.y, p0.z) );
      }
    }
  }
}

TEST(bvh,nearest_local)
{
  std::vector<double> aXYZ;
  std::vector<unsigned int> aTri;
  { // make a unit sphere
    MeshTri3D_Sphere(aXYZ, aTri, 1.0, 64, 32);
    Rotate(aXYZ, 0.2, 0.3, 0.4);
  }
//  std::cout << "ntri: " << aTri.size()/3 << std::endl;
  std::vector<double> aNorm(aXYZ.size());
  Normal_MeshTri3D(aNorm.data(),
                   aXYZ.data(), aXYZ.size()/3, aTri.data(), aTri.size()/3);
  CBVH_MeshTri3D<CBV3D_Sphere> bvh;
  bvh.Init(aXYZ, aTri, 0.03);
  for(int itr=0;itr<1000;++itr){
    CVector3 p0;
    {
      p0.x = 10.0*(rand()/(RAND_MAX+1.0)-0.5);
      p0.y = 10.0*(rand()/(RAND_MAX+1.0)-0.5);
      p0.z = 10.0*(rand()/(RAND_MAX+1.0)-0.5);
      p0.SetNormalizedVector();
      if( itr % 2 == 0 ){ p0 *= 1.02; } // outside included in bvh
      else{               p0 *= 0.98; } // inside in included in bvh
    }
    CPointElemSurf pes1 = bvh.Nearest_Point_IncludedInBVH(p0,aXYZ,aTri);
    EXPECT_TRUE( pes1.Check(aXYZ, aTri,1.0e-10) );
    CVector3 q1 = pes1.Pos_Tri(aXYZ, aTri);
    {
      CPointElemSurf pes0 = Nearest_Point_MeshTri3D(p0, aXYZ, aTri);
      CVector3 q0 = pes0.Pos_Tri(aXYZ, aTri);
      EXPECT_LT(Distance(q0,q1),1.0e-10);
    }
    CVector3 n0 = pes1.UNorm_Tri(aXYZ, aTri, aNorm);
    EXPECT_EQ( n0*(p0-q1)>0, itr%2==0);
    //////
    {
      CPointElemSurf pes2;
      double dist_min = -1;
      BVH_NearestPoint_IncludedInBVH_MeshTri3D(dist_min, pes2,
                                               p0.x, p0.y, p0.z,
                                               aXYZ, aTri,
                                               bvh.iroot_bvh, bvh.aNodeBVH, bvh.aBB_BVH);
      CVector3 q2 = pes2.Pos_Tri(aXYZ, aTri);
      EXPECT_LT(Distance(q2,q1),1.0e-10);
    }
  }
}


TEST(bvh,nearest_range) // find global nearest from range
{
  std::vector<double> aXYZ;
  std::vector<unsigned int> aTri;
  { // make a unit sphere
    MeshTri3D_Sphere(aXYZ, aTri, 1.0, 64, 32);
    Rotate(aXYZ, 0.2, 0.3, 0.4);
  }
  CBVH_MeshTri3D<CBV3D_Sphere> bvh;
  bvh.Init(aXYZ, aTri, 0.0);
  for(int itr=0;itr<1000;++itr){
    CVector3 p0;
    {
      p0.x = 10.0*(rand()/(RAND_MAX+1.0)-0.5);
      p0.y = 10.0*(rand()/(RAND_MAX+1.0)-0.5);
      p0.z = 10.0*(rand()/(RAND_MAX+1.0)-0.5);
    }
    {
      double dist_min=-1, dist_max = -1;
      ::BVH_Range_DistToNearestPoint(dist_min, dist_max,
                                     p0.x, p0.y, p0.z,
                                     bvh.iroot_bvh, bvh.aNodeBVH, bvh.aBB_BVH);
      bool is_max = false;
      for(int it=0;it<aTri.size()/3;++it){
        CBV3D_Sphere bb;
        bb.is_active = false;
        for(int inoel=0;inoel<3;++inoel){
          const int ino0 = aTri[it*3+inoel];
          bb.AddPoint(aXYZ[ino0*3+0], aXYZ[ino0*3+1], aXYZ[ino0*3+2], 0.0);
        }
        double min0, max0;
        bb.Range_DistToPoint(min0, max0, p0.x, p0.y, p0.z);
        EXPECT_GE( max0, dist_max );
        EXPECT_GE( min0, dist_min );
        if( max0 < dist_max+1.0e-10 ){ is_max = true; }
      }
      EXPECT_TRUE( is_max );
      std::vector<int> aIndElem;
      BVH_GetIndElem_InsideRange(aIndElem,
                                 dist_min,dist_max,
                                 p0.x, p0.y, p0.z,
                                 bvh.iroot_bvh, bvh.aNodeBVH, bvh.aBB_BVH);
      EXPECT_GT(aIndElem.size(), 0);
      std::vector<int> aFlg(aTri.size()/3,0);
      for(int iit=0;iit<aIndElem.size();++iit){
        int itri0 = aIndElem[iit];
        aFlg[itri0] = 1;
      }
      for(int itri=0;itri<aTri.size()/3;++itri){
        CBV3D_Sphere bb;
        bb.is_active = false;
        for(int inoel=0;inoel<3;++inoel){
          const int ino0 = aTri[itri*3+inoel];
          bb.AddPoint(aXYZ[ino0*3+0], aXYZ[ino0*3+1], aXYZ[ino0*3+2], 0.0);
        }
        double min0, max0;
        bb.Range_DistToPoint(min0, max0, p0.x, p0.y, p0.z);
        if( aFlg[itri] == 1 ){ // inside range
          EXPECT_LE(min0,dist_max);
          EXPECT_GE(max0,dist_min);
        }
        else{ // outside range
          EXPECT_TRUE((min0>dist_max)||(max0<dist_min));
        }
      }
    }
  }
}

TEST(bvh,nearest_point) // find global nearest directry
{
  std::vector<double> aXYZ;
  std::vector<unsigned int> aTri;
  { // make a unit sphere
    MeshTri3D_Sphere(aXYZ, aTri, 1.0, 64, 32);
    Rotate(aXYZ, 0.2, 0.3, 0.4);
  }
  CBVH_MeshTri3D<CBV3D_Sphere> bvh;
  bvh.Init(aXYZ, aTri, 0.0);
  for(int itr=0;itr<1000;++itr){
    CVector3 p0;
    {
      p0.x = 10.0*(rand()/(RAND_MAX+1.0)-0.5);
      p0.y = 10.0*(rand()/(RAND_MAX+1.0)-0.5);
      p0.z = 10.0*(rand()/(RAND_MAX+1.0)-0.5);
    }
    CPointElemSurf pes1 = bvh.NearestPoint_Global(p0,aXYZ,aTri);
    EXPECT_TRUE( pes1.Check(aXYZ, aTri,1.0e-10) );
    CVector3 q1 = pes1.Pos_Tri(aXYZ, aTri);
    {
      CPointElemSurf pes0 = Nearest_Point_MeshTri3D(p0, aXYZ, aTri);
      CVector3 q0 = pes0.Pos_Tri(aXYZ, aTri);
      EXPECT_LT(Distance(q0,q1),1.0e-10);
    }
  }
}


TEST(bvh,sdf) // find global nearest directry
{
  std::vector<double> aXYZ;
  std::vector<unsigned int> aTri;
  { // make a unit sphere
    MeshTri3D_Sphere(aXYZ, aTri, 1.0, 64, 32);
    Rotate(aXYZ, 0.2, 0.3, 0.4);
  }
  std::vector<double> aNorm(aXYZ.size());
  Normal_MeshTri3D(aNorm.data(),
                   aXYZ.data(), aXYZ.size()/3, aTri.data(), aTri.size()/3);
  CBVH_MeshTri3D<CBV3D_Sphere> bvh;
  bvh.Init(aXYZ, aTri, 0.0);
  for(int itr=0;itr<1000;++itr){
    CVector3 p0;
    {
      p0.x = 3.0*(rand()/(RAND_MAX+1.0)-0.5);
      p0.y = 3.0*(rand()/(RAND_MAX+1.0)-0.5);
      p0.z = 3.0*(rand()/(RAND_MAX+1.0)-0.5);
    }
    if( (p0.Length()-1.0)<1.0e-3 ){ continue; }
    CVector3 n0;
    double sdf = bvh.SignedDistanceFunction(n0,
                                            p0, aXYZ, aTri, aNorm);
    EXPECT_NEAR(1-p0.Length(), sdf, 1.0e-2);
    EXPECT_NEAR(n0*p0.Normalize(), 1.0, 1.0e-2 );
  }
}


TEST(bvh,lineintersection)
{
  std::vector<double> aXYZ;
  std::vector<unsigned int> aTri;
  { // make a unit sphere
    MeshTri3D_Sphere(aXYZ, aTri, 1.0, 64, 32);
    Rotate(aXYZ, 0.2, 0.3, 0.4);
  }
  std::vector<double> aNorm(aXYZ.size());
  Normal_MeshTri3D(aNorm.data(),
                   aXYZ.data(), aXYZ.size()/3, aTri.data(), aTri.size()/3);
  CBVH_MeshTri3D<CBV3D_Sphere> bvh;
  bvh.Init(aXYZ, aTri, 1.0e-5);
  for(int itr=0;itr<100;++itr){
    CVector3 s0, d0;
    {
      s0.x = 3.0*(rand()/(RAND_MAX+1.0)-0.5);
      s0.y = 3.0*(rand()/(RAND_MAX+1.0)-0.5);
      s0.z = 3.0*(rand()/(RAND_MAX+1.0)-0.5);
      d0.x = 3.0*(rand()/(RAND_MAX+1.0)-0.5);
      d0.y = 3.0*(rand()/(RAND_MAX+1.0)-0.5);
      d0.z = 3.0*(rand()/(RAND_MAX+1.0)-0.5);
      d0.SetNormalizedVector();
    }
    double ps0[3]; s0.CopyValueTo(ps0);
    double pd0[3]; d0.CopyValueTo(pd0);
    for(int ibvh=0;ibvh<bvh.aNodeBVH.size();++ibvh){
      const CBV3D_Sphere& bv = bvh.aBB_BVH[ibvh];
      const CNodeBVH& node = bvh.aNodeBVH[ibvh];
      bool is_intersect = bv.IsIntersectLine(ps0,pd0);
      if( !is_intersect && node.ichild[1] != -1 ){ // branch
        const int ichild0 = node.ichild[0];
        const int ichild1 = node.ichild[1];
        EXPECT_FALSE( bvh.aBB_BVH[ichild0].IsIntersectLine(ps0,pd0) );
        EXPECT_FALSE( bvh.aBB_BVH[ichild1].IsIntersectLine(ps0,pd0) );
      }
    }
    std::vector<int> aIndElem;
    BVH_GetIndElem_IntersectLine(aIndElem, ps0, pd0,
                                 bvh.iroot_bvh, bvh.aNodeBVH, bvh.aBB_BVH);
    std::vector<int> aFlg(aTri.size()/3,0);
    for(unsigned int iit=0;iit<aIndElem.size();++iit){
      int itri0 = aIndElem[iit];
      aFlg[itri0] = 1;
    }
    for(unsigned int itri=0;itri<aTri.size()/3;++itri){
      CBV3D_Sphere bb;
      bb.is_active = false;
      for(int inoel=0;inoel<3;++inoel){
        const int ino0 = aTri[itri*3+inoel];
        bb.AddPoint(aXYZ[ino0*3+0], aXYZ[ino0*3+1], aXYZ[ino0*3+2], 1.0e-5);
      }
      bool res = bb.IsIntersectLine(ps0, pd0);
      EXPECT_EQ( res , aFlg[itri] == 1 );
    }
  }
}


TEST(bvh,rayintersection)
{
  std::vector<double> aXYZ;
  std::vector<unsigned int> aTri;
  { // make a unit sphere
    MeshTri3D_Sphere(aXYZ, aTri, 1.0, 64, 32);
    Rotate(aXYZ, 0.2, 0.3, 0.4);
  }
  std::vector<double> aNorm(aXYZ.size());
  Normal_MeshTri3D(aNorm.data(),
                   aXYZ.data(), aXYZ.size()/3, aTri.data(), aTri.size()/3);
  CBVH_MeshTri3D<CBV3D_Sphere> bvh;
  bvh.Init(aXYZ, aTri, 1.0e-5);
  for(int itr=0;itr<100;++itr){
    CVector3 s0, d0;
    {
      s0.x = 3.0*(rand()/(RAND_MAX+1.0)-0.5);
      s0.y = 3.0*(rand()/(RAND_MAX+1.0)-0.5);
      s0.z = 3.0*(rand()/(RAND_MAX+1.0)-0.5);
      d0.x = 3.0*(rand()/(RAND_MAX+1.0)-0.5);
      d0.y = 3.0*(rand()/(RAND_MAX+1.0)-0.5);
      d0.z = 3.0*(rand()/(RAND_MAX+1.0)-0.5);
      d0.SetNormalizedVector();
    }
    double ps0[3]; s0.CopyValueTo(ps0);
    double pd0[3]; d0.CopyValueTo(pd0);
    for(int ibvh=0;ibvh<bvh.aNodeBVH.size();++ibvh){
      const CBV3D_Sphere& bv = bvh.aBB_BVH[ibvh];
      const CNodeBVH& node = bvh.aNodeBVH[ibvh];
      bool is_intersect = bv.IsIntersectRay(ps0,pd0);
      if( !is_intersect && node.ichild[1] != -1 ){ // branch
        const int ichild0 = node.ichild[0];
        const int ichild1 = node.ichild[1];
        EXPECT_FALSE( bvh.aBB_BVH[ichild0].IsIntersectRay(ps0,pd0) );
        EXPECT_FALSE( bvh.aBB_BVH[ichild1].IsIntersectRay(ps0,pd0) );
      }
    }
    std::vector<int> aIndElem;
    BVH_GetIndElem_IntersectRay(aIndElem, ps0, pd0,
                                bvh.iroot_bvh, bvh.aNodeBVH, bvh.aBB_BVH);
    std::vector<int> aFlg(aTri.size()/3,0);
    for(unsigned int iit=0;iit<aIndElem.size();++iit){
      int itri0 = aIndElem[iit];
      aFlg[itri0] = 1;
    }
    for(unsigned int itri=0;itri<aTri.size()/3;++itri){
      CBV3D_Sphere bb;
      bb.is_active = false;
      for(int inoel=0;inoel<3;++inoel){
        const int ino0 = aTri[itri*3+inoel];
        bb.AddPoint(aXYZ[ino0*3+0], aXYZ[ino0*3+1], aXYZ[ino0*3+2], 1.0e-5);
      }
      bool res = bb.IsIntersectRay(ps0, pd0);
      EXPECT_EQ( res , aFlg[itri] == 1 );
    }
    {
      std::map<double,CPointElemSurf> mapDepthPES0;
      IntersectionRay_MeshTri3D(mapDepthPES0,
                                s0, d0, aTri, aXYZ);
      std::map<double,CPointElemSurf> mapDepthPES1;
      IntersectionRay_MeshTri3DPart(mapDepthPES1,
                                    s0, d0,
                                    aTri, aXYZ, aIndElem);
      EXPECT_EQ(mapDepthPES0.size(),mapDepthPES1.size());
      int N = mapDepthPES0.size();
      std::map<double,CPointElemSurf>::iterator itr0 = mapDepthPES0.begin();
      std::map<double,CPointElemSurf>::iterator itr1 = mapDepthPES0.begin();
      for(int i=0;i<N;++i){
        EXPECT_FLOAT_EQ(itr0->first,itr1->first);
        CPointElemSurf pes0 = itr0->second;
        CPointElemSurf pes1 = itr1->second;
        CVector3 q0 = pes0.Pos_Tri(aXYZ, aTri);
        CVector3 q1 = pes1.Pos_Tri(aXYZ, aTri);
        EXPECT_NEAR(Distance(q0,q1), 0.0, 1.0e-10);
      }
    }
  }
}

