/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cmath>
#include "delfem2/vec2.h"
#include "delfem2/vec3.h"
#include "delfem2/mat3.h"
#include "delfem2/mat4.h"
#include "delfem2/quat.h"
//
#include "delfem2/geo3_v23m34q.h"

// ----------------------------------------

DFM2_INLINE delfem2::CVec2d delfem2::screenXYProjection
(const CVec3d& v,
 const float* mMV,
 const float* mPj)
{
  CVec3d sp0 = screenProjection(v,mMV,mPj);
  return delfem2::CVec2d(sp0.x(),sp0.y());
}

DFM2_INLINE delfem2::CVec3d delfem2::GetCartesianRotationVector
 (const CMat3d& m)
{
  const double* mat = m.mat;
  CVec3d a;
  a.p[0] = mat[7]-mat[5];
  a.p[1] = mat[2]-mat[6];
  a.p[2] = mat[3]-mat[1];
  double act = (m.Trace()-1)*0.5;
  if( act > +1 ){ act = +1; }
  if( act < -1 ){ act = -1; }
  double theta = acos(act);
  if( myIsNAN_Matrix3(theta) ){ return a; }
  if( fabs(theta) < 1.0e-5 ){ return a*0.5; }
  double mag = 0.5*theta/sin(theta);
  a *= mag;
  return a;
}

DFM2_INLINE delfem2::CVec3d delfem2::GetSpinVector(const CMat3d& m)
{
  const double* mat = m.mat;
  CVec3d r;
  r.p[0] = (mat[7]-mat[5])*0.5;
  r.p[1] = (mat[2]-mat[6])*0.5;
  r.p[2] = (mat[3]-mat[1])*0.5;
  return r;
}

DFM2_INLINE delfem2::CVec3d delfem2::MatVec(const CMat3d& m, const CVec3d& vec0)
{
  CVec3d vec1;
  delfem2::MatVec3(vec1.p, m.mat,vec0.p);
  return vec1;
}

DFM2_INLINE delfem2::CVec3d delfem2::MatVecTrans
 (const CMat3d& m, const CVec3d& vec0)
{
  CVec3d vec1;
  MatTVec3(vec1.p, m.mat,vec0.p);
  return vec1;
}

// ---------------------------------------------------------------------

DFM2_INLINE void delfem2::SetDiag
 (CMat3d& m,
  const CVec3d& d)
{
  double* mat = m.mat;
  mat[0*3+0] = d.x();
  mat[1*3+1] = d.y();
  mat[2*3+2] = d.z();
}

DFM2_INLINE void delfem2::SetRotMatrix_Cartesian
 (CMat3d& m,
  const CVec3d& v)
{
  m.SetRotMatrix_Cartesian(v.p);
}

DFM2_INLINE void delfem2::SetSpinTensor(CMat3d& m, const CVec3d& vec0)
{
  Mat3_Spin(m.mat, vec0.p);
}

DFM2_INLINE void delfem2::SetOuterProduct
 (CMat3d& m,
  const CVec3d& vec0,
  const CVec3d& vec1 )
{
  double* mat = m.mat;
  mat[0] = vec0.x()*vec1.x(); mat[1] = vec0.x()*vec1.y(); mat[2] = vec0.x()*vec1.z();
  mat[3] = vec0.y()*vec1.x(); mat[4] = vec0.y()*vec1.y(); mat[5] = vec0.y()*vec1.z();
  mat[6] = vec0.z()*vec1.x(); mat[7] = vec0.z()*vec1.y(); mat[8] = vec0.z()*vec1.z();
}

DFM2_INLINE void delfem2::SetProjection(CMat3d& m, const CVec3d& vec0)
{
  double* mat = m.mat;
  const CVec3d& u = vec0.Normalize();
  mat[0] = 1-u.x()*u.x(); mat[1] = 0-u.x()*u.y(); mat[2] = 0-u.x()*u.z();
  mat[3] = 0-u.y()*u.x(); mat[4] = 1-u.y()*u.y(); mat[5] = 0-u.y()*u.z();
  mat[6] = 0-u.z()*u.x(); mat[7] = 0-u.z()*u.y(); mat[8] = 1-u.z()*u.z();
}

// ----------------------------

DFM2_INLINE delfem2::CMat3d delfem2::Mirror(const CVec3d& n)
{
  CVec3d N = n;
  N.SetNormalizedVector();
  return CMat3d::Identity() - 2*delfem2::Mat3_OuterProduct(N,N);
}

DFM2_INLINE delfem2::CMat3d delfem2::Mat3_CrossCross(const CVec3d& v)
{
  return Mat3(v)*Mat3(v);
}

DFM2_INLINE delfem2::CMat3d delfem2::RotMatrix_Cartesian(const CVec3d& v){
 CMat3d m;
 SetRotMatrix_Cartesian(m,v);
 return m;
}

DFM2_INLINE delfem2::CMat3d delfem2::Mat3(const CVec3d& vec0){
  CMat3d m;
  SetSpinTensor(m,vec0);
  return m;
}

DFM2_INLINE delfem2::CMat3d delfem2::Mat3(const CVec3d& vec0, const CVec3d& vec1){
  CMat3d m;
  SetOuterProduct(m,vec0, vec1);
  return m;
}

DFM2_INLINE delfem2::CMat3d delfem2::Mat3(const CVec3d& vec0, const CVec3d& vec1, const CVec3d& vec2)
{
  CMat3d m;
  double* mat = m.mat;
  mat[0*3+0]=vec0.x(); mat[0*3+1]=vec1.x(); mat[0*3+2]=vec2.x();
  mat[1*3+0]=vec0.y(); mat[1*3+1]=vec1.y(); mat[1*3+2]=vec2.y();
  mat[2*3+0]=vec0.z(); mat[2*3+1]=vec1.z(); mat[2*3+2]=vec2.z();
  return m;
}

DFM2_INLINE delfem2::CMat3d delfem2::Mat3_Spin(const CVec3d& vec0){
  CMat3d m;
  ::delfem2::Mat3_Spin(m.mat,vec0.p);
  return m;
}

DFM2_INLINE delfem2::CMat3d delfem2::Mat3_OuterProduct(const CVec3d& vec0, const CVec3d& vec1 )
{
  CMat3d m;
  SetOuterProduct(m,vec0,vec1);
  return m;
}

DFM2_INLINE delfem2::CMat3d delfem2::Mat3_RotCartesian(const CVec3d& vec0)
{
  CMat3d m;
  m.SetRotMatrix_Cartesian(vec0.x(), vec0.y(), vec0.z());
  return m;
}

// ------------------

namespace delfem2 {
  
DFM2_INLINE CVec3d operator* (const CVec3d& v, const CMat3d& m){
  return MatVecTrans(m,v);
}
  
DFM2_INLINE CVec3d operator* (const CMat3d& m, const CVec3d& v)
{
  return MatVec(m,v);
}
  
}

// ------------------------------

template <typename REAL>
delfem2::CMat3<REAL> delfem2::Mat3_MinimumRotation
(const CVec3<REAL>& V,
 const CVec3<REAL>& v)
{
  CVec3<REAL> ep = V.Normalize();
  CVec3<REAL> eq = v.Normalize();
  CVec3<REAL> n = ep^eq;
  const double st2 = n*n;
  CMat3<REAL> m;
  if( st2 < 1.0e-4f ){
    m.mat[0] = 1.f      +0.5f*(n.x()*n.x()-st2);
    m.mat[1] =    -n.z()+0.5f*(n.x()*n.y());
    m.mat[2] =    +n.y()+0.5f*(n.x()*n.z());
    m.mat[3] =    +n.z()+0.5f*(n.y()*n.x());
    m.mat[4] = 1.f      +0.5f*(n.y()*n.y()-st2);
    m.mat[5] =    -n.x()+0.5f*(n.y()*n.z());
    m.mat[6] =    -n.y()+0.5f*(n.z()*n.x());
    m.mat[7] =    +n.x()+0.5f*(n.z()*n.y());
    m.mat[8] = 1.f      +0.5f*(n.z()*n.z()-st2);
    return m;
  }
  const double st = sqrt(st2);
  const double ct = ep*eq;
  n.SetNormalizedVector();
  m.mat[0] = ct         +(1.f-ct)*n.x()*n.x();
  m.mat[1] =   -n.z()*st+(1.f-ct)*n.x()*n.y();
  m.mat[2] =   +n.y()*st+(1.f-ct)*n.x()*n.z();
  m.mat[3] =   +n.z()*st+(1.f-ct)*n.y()*n.x();
  m.mat[4] = ct         +(1.f-ct)*n.y()*n.y();
  m.mat[5] =   -n.x()*st+(1.f-ct)*n.y()*n.z();
  m.mat[6] =   -n.y()*st+(1.f-ct)*n.z()*n.x();
  m.mat[7] =   +n.x()*st+(1.f-ct)*n.z()*n.y();
  m.mat[8] = ct         +(1.f-ct)*n.z()*n.z();
  return m;
}
#if !defined(DFM2_HEADER_ONLY)
template delfem2::CMat3d delfem2::Mat3_MinimumRotation(const CVec3d& V, const CVec3d& v);
#endif


// --------------------------

DFM2_INLINE delfem2::CMat3d delfem2::Mat3_ParallelTransport
(const CVec3d& p0,
 const CVec3d& p1,
 const CVec3d& q0,
 const CVec3d& q1)
{
  return Mat3_MinimumRotation(p1-p0, q1-q0);
}

// -----------------------------------------------------
// below: rotational inertia

// moment of inertia around origin triangle vtx (origin,d0,d1,d2) the area_density=1
// see http://www.dcs.warwick.ac.uk/~rahil/files/RigidBodySimulation.pdf
DFM2_INLINE delfem2::CMat3d delfem2::Mat3_IrotTri
(const CVec3d& d0,
 const CVec3d& d1,
 const CVec3d& d2)
{
  
  CVec3d dv = d0+d1+d2;
  CMat3d I0 = Mat3_OuterProduct(d0,d0) + Mat3_OuterProduct(d1,d1) + Mat3_OuterProduct(d2,d2) + Mat3_OuterProduct(dv,dv);
  double tr0 = I0.Trace();
  CMat3d I = tr0*CMat3d::Identity()-I0;
  
  double darea = ((d1-d0)^(d2-d0)).Length();
  I *= darea/24.0;
  return I;
}

// moment of inertia triangle pyramid with vtx (origin,d0,d1,d2) volume_density = 1
// see http://www.dcs.warwick.ac.uk/~rahil/files/RigidBodySimulation.pdf
DFM2_INLINE delfem2::CMat3d delfem2::Mat3_IrotTriSolid
(const CVec3d& d0,
 const CVec3d& d1,
 const CVec3d& d2)
{
  CVec3d dv = d0+d1+d2;
  CMat3d I0 = Mat3_OuterProduct(d0,d0) + Mat3_OuterProduct(d1,d1) + Mat3_OuterProduct(d2,d2) + Mat3_OuterProduct(dv,dv);
  double tr0 = I0.Trace();
  CMat3d I = tr0*CMat3d::Identity()-I0;
  
  double darea = (d0*(d1^d2));
  I *= darea/120.0;
  return I;
}

DFM2_INLINE delfem2::CMat3d delfem2::Mat3_IrotLineSeg
(const CVec3d& d0,
 const CVec3d& d1)
{
  CVec3d dv = d1-d0;
  double l = dv.Length();
  CMat3d I;
  {
    I = dv.DLength()*CMat3d::Identity()-Mat3_OuterProduct(dv,dv);
    I *= l/12.0;
  }
  CVec3d p = (d0+d1)*0.5;
  I += l*(p.DLength()*CMat3d::Identity()-Mat3_OuterProduct(p,p));
  return I;
}

DFM2_INLINE delfem2::CMat3d delfem2::Mat3_IrotPoint
(const CVec3d& d0)
{
  return (d0.DLength()*CMat3d::Identity()-Mat3_OuterProduct(d0,d0));
}


// above: rotational inertia
// ---------------------------------------------------------------------


DFM2_INLINE void delfem2::Mat4_MatTransl(double m[16], const CMat3d& mat, const CVec3d& trans)
{
  mat.AffineMatrixTrans(m);
  m[3*4+0] = trans.x();
  m[3*4+1] = trans.y();
  m[3*4+2] = trans.z();
}


DFM2_INLINE void delfem2::Mat4_ScaleMatTransl
 (double m[16],
  double scale,
  const CMat3d& mat,
  const CVec3d& trans)
{
  mat.AffineMatrixTrans(m);
  for(int i=0;i<3;++i){
  for(int j=0;j<3;++j){
    m[i*4+j] *= scale;
  }
  }
  m[3*4+0] = trans.x();
  m[3*4+1] = trans.y();
  m[3*4+2] = trans.z();
}

// ---------------------------------------------------------------------------------------


// ----------------------------------------------------
// quaternion

namespace delfem2 {

template <typename REAL>
DFM2_INLINE CVec3<REAL> operator*
 (const CQuat<REAL>& q,
  const CVec3<REAL>& v)
{
  CVec3<REAL> p;
  QuatVec(p.p,
          q.q, v.p);
  return p;
}
#ifndef DFM2_HEADER_ONLY
template CVec3f operator* (const CQuatf& q, const CVec3f& v);
template CVec3d operator* (const CQuatd& q, const CVec3d& v);
#endif

}

// ------------

DFM2_INLINE delfem2::CQuatd delfem2::Quat_CartesianAngle(const CVec3d& p)
{
  CQuatd q;
  Quat_CartesianAngle(q.q, p.p);
  return q;
}


DFM2_INLINE void delfem2::UpdateRotationsByMatchingCluster
 (std::vector<double>& aQuat1,
  const std::vector<double>& aXYZ0,
  const std::vector<double>& aXYZ1,
  const std::vector<unsigned int>& psup_ind,
  const std::vector<unsigned int>& psup)
{
  const unsigned int np = aXYZ0.size()/3;
  for(unsigned int ip=0;ip<np;++ip){
    const CVec3d Pi(aXYZ0.data()+ip*3);
    const CVec3d pi(aXYZ1.data()+ip*3);
    const CQuatd Qi(aQuat1.data()+ip*4);
    CMat3d Mat; Mat.SetZero();
    CVec3d rhs; rhs.SetZero();
    for(unsigned int ipsup=psup_ind[ip];ipsup<psup_ind[ip+1];++ipsup){
      const unsigned int jp = psup[ipsup];
      const CVec3d v0 = Qi*(CVec3d(aXYZ0.data()+jp*3)-Pi);
      const CVec3d v1 = CVec3d(aXYZ1.data()+jp*3)-pi-v0;
      Mat += Mat3_CrossCross(v0);
      rhs += v1^v0;
    }
    CVec3d sol = Mat.Inverse()*rhs;
    CQuatd q0 = Quat_CartesianAngle(sol);
    CQuatd q1 = q0*CQuatd(aQuat1.data()+ip*4);
    q1.CopyTo(aQuat1.data()+ip*4);
  }
}
