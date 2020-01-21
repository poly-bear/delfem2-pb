/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "delfem2/bem.h"
#include "delfem2/v23m3q.h"

#ifndef M_PI
#define M_PI 3.141592653589793
#endif

#if defined(__APPLE__) && defined(__MACH__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

namespace dfm2 = delfem2;

// ---------------------------
// linear algebra

double squaredNorm(const std::vector<double>& v)
{
  const int n = (int)v.size();
  double s = 0;
  for (int i = 0; i<n; ++i){ s += v[i]*v[i]; }
  return s;
}

// vai*conj(vbi)
double dot(const std::vector<double>& va, const std::vector<double>& vb)
{
  const int n = (int)va.size();
  double s = 0;
  for (int i = 0; i<n; ++i){ s += va[i]*vb[i]; }
  return s;
}

void normalize(std::vector<double>& v)
{
  const int n = (int)v.size();
  double sqlen = squaredNorm(v);
  double leninv = 1.0/sqrt(sqlen);
  for (int i = 0; i<n; ++i){ v[i] *= leninv; }
}

// {y} = [A]{x}
void matVec
(std::vector<double>& y,
const std::vector<double>& A,
const std::vector<double>& x)
{
  const int n = (int)x.size();
  assert(A.size()==n*n);
  y.resize(n);
  for (int i = 0; i<n; ++i){
    double s = 0;
    for (int j = 0; j<n; ++j){ s += A[i*n+j]*x[j]; }
    y[i] = s;
  }
}



// /////////////////////////////////////////////////////////////
// Solve Matrix with BiCGSTAB Methods
// /////////////////////////////////////////////////////////////
bool Solve_BiCGSTAB
(double& conv_ratio, int& iteration,
std::vector<double>& u_vec,
const std::vector<double>& A,
const std::vector<double>& y_vec)
{

  //	std::cout.precision(18);

  const double conv_ratio_tol = conv_ratio;
  const int mx_iter = iteration;

  const int n = (int)y_vec.size();

  u_vec.assign(n, 0);

  assert(A.size()==n*n);

  std::vector<double>  r_vec = y_vec;
  std::vector<double>  s_vec(n);
  std::vector<double> As_vec(n);
  std::vector<double>  p_vec(n);
  std::vector<double> Ap_vec(n);

  std::vector<double>  r0(n);

  double sq_inv_norm_res;
  {
    double dtmp1 = squaredNorm(r_vec);
    //    std::cout << "Initial Residual: " << sqrt(dtmp1) << std::endl;
    if (dtmp1 < 1.0e-30){
      conv_ratio = 0.0;
      iteration = 0;
      return true;
    }
    sq_inv_norm_res = 1.0/dtmp1;
  }

  r0 = r_vec;
  //  for (int i = 0; i<n; ++i){ r0[i] = std::conj(r_vec[i]); }

  // {p} = {r}
  p_vec = r_vec;

  // calc (r,r0*)
  double r_r0conj = dot(r_vec, r0);

  iteration = mx_iter;
  for (int iitr = 1; iitr<mx_iter; iitr++){

    // calc {Ap} = [A]*{p}
    matVec(Ap_vec, A, p_vec);

    // calc alpha
    double alpha;
    {
      const double den = dot(Ap_vec, r0);
      alpha = r_r0conj/den;
    }

    // calc s_vector
    for (int i = 0; i<n; ++i){ s_vec[i] = r_vec[i]-alpha*Ap_vec[i]; }

    // calc {As} = [A]*{s}
    matVec(As_vec, A, s_vec);

    // calc omega
    double omega;
    {
      const double den = squaredNorm(As_vec);
      const double num = dot(As_vec, s_vec);
      omega = num/den;
    }

    // update solution
    for (int i = 0; i<n; ++i){ u_vec[i] += alpha*p_vec[i]+omega*s_vec[i]; }

    // update residual
    for (int i = 0; i<n; ++i){ r_vec[i] = s_vec[i]-omega*As_vec[i]; }

    {
      const double sq_norm_res = squaredNorm(r_vec);
      const double sq_conv_ratio = sq_norm_res * sq_inv_norm_res;
       std::cout << iitr << " " << sqrt(sq_conv_ratio) << " " << sqrt(sq_norm_res) << std::endl;
      if (sq_conv_ratio < conv_ratio_tol * conv_ratio_tol){
        conv_ratio = sqrt(sq_norm_res * sq_inv_norm_res);
        iteration = iitr;
        return true;
      }
    }

    // calc beta
    double beta;
    {
      const double tmp1 = dot(r_vec, r0);
      beta = (tmp1/r_r0conj) * (alpha/omega);
      r_r0conj = tmp1;
    }

    // update p_vector
    for (int i = 0; i<n; ++i){
      p_vec[i] = beta*p_vec[i]+r_vec[i]-(beta*omega)*Ap_vec[i];
    }
  }

  return true;
}

// -----------------------------

dfm2::CVector3 NormalTri(
    int itri,
    const std::vector<unsigned int> &aTri,
    const std::vector<double>& aXYZ)
{
  int i0 = aTri[itri*3+0];
  int i1 = aTri[itri*3+1];
  int i2 = aTri[itri*3+2];
  dfm2::CVector3 v0(aXYZ[i0*3+0], aXYZ[i0*3+1], aXYZ[i0*3+2]);
  dfm2::CVector3 v1(aXYZ[i1*3+0], aXYZ[i1*3+1], aXYZ[i1*3+2]);
  dfm2::CVector3 v2(aXYZ[i2*3+0], aXYZ[i2*3+1], aXYZ[i2*3+2]);
  dfm2::CVector3 n; Normal(n, v0, v1, v2);
  return n;
}


const static unsigned int NIntTriGauss[6] = { 1, 3, 7, 12, 13, 16 };
const static double TriGauss[6][16][3] =
{
  { // 1 point (degree of accuracy = 1)
    { 0.3333333333, 0.3333333333, 1.0 },
    { 0.0, 0.0, 0.0 },
    { 0.0, 0.0, 0.0 },
    { 0.0, 0.0, 0.0 },
    { 0.0, 0.0, 0.0 },
    { 0.0, 0.0, 0.0 },
  },
  { // 3 point (degree of accuracy = 2)
    { 0.1666666667, 0.1666666667, 0.3333333333 },
    { 0.6666666667, 0.1666666667, 0.3333333333 },
    { 0.1666666667, 0.6666666667, 0.3333333333 },
    { 0.0, 0.0, 0.0 },
    { 0.0, 0.0, 0.0 },
    { 0.0, 0.0, 0.0 },
  },
  { // 7 point (degree of accuracy = 5)
    { 0.1012865073, 0.1012865073, 0.1259391805 },
    { 0.7974269854, 0.1012865073, 0.1259391805 },
    { 0.1012865073, 0.7974269854, 0.1259391805 },
    { 0.4701420641, 0.0597158718, 0.1323941527 },
    { 0.4701420641, 0.4701420641, 0.1323941527 },
    { 0.0597158718, 0.4701420641, 0.1323941527 },
    { 0.3333333333, 0.3333333333, 0.2250000000 },
  },
  { // 12 point (degree of accuracy = 6)
    { 0.501426509658179, 0.249286745170910, 0.116786275726379 },
    { 0.249286745170910, 0.501426509658179, 0.116786275726379 },
    { 0.249286745170910, 0.249286745170910, 0.116786275726379 },
    { 0.873821971016996, 0.063089014491502, 0.050844906370207 },
    { 0.063089014491502, 0.873821971016996, 0.050844906370207 },
    { 0.063089014491502, 0.063089014491502, 0.050844906370207 },
    { 0.053145049844817, 0.310352451033784, 0.082851075618374 },
    { 0.053145049844817, 0.636502499121399, 0.082851075618374 },
    { 0.310352451033784, 0.636502499121399, 0.082851075618374 },
    { 0.310352451033784, 0.053145049844817, 0.082851075618374 },
    { 0.636502499121399, 0.053145049844817, 0.082851075618374 },
    { 0.636502499121399, 0.310352451033784, 0.082851075618374 },
  },
  { // 13 point (degree of accuracy =7)
    { 0.333333333333333, 0.333333333333333, -0.149570044467682 },
    { 0.479308067841920, 0.260345966079040, 0.175615257433208 },
    { 0.260345966079040, 0.479308067841920, 0.175615257433208 },
    { 0.260345966079040, 0.260345966079040, 0.175615257433208 },
    { 0.869739794195568, 0.065130102902216, 0.053347235608838 },
    { 0.065130102902216, 0.869739794195568, 0.053347235608838 },
    { 0.065130102902216, 0.065130102902216, 0.053347235608838 },
    { 0.048690315425316, 0.312865496004874, 0.077113760890257 },
    { 0.048690315425316, 0.638444188569810, 0.077113760890257 },
    { 0.312865496004874, 0.638444188569810, 0.077113760890257 },
    { 0.312865496004874, 0.048690315425316, 0.077113760890257 },
    { 0.638444188569810, 0.048690315425316, 0.077113760890257 },
    { 0.638444188569810, 0.312865496004874, 0.077113760890257 },
  },
  { // 16point  (degree of accuracy = 8)
    { 0.333333333333333, 0.333333333333333, 0.144315607677787 },
    { 0.081414823414554, 0.459292588292723, 0.095091634267285 },
    { 0.459292588292723, 0.081414823414554, 0.095091634267285 },
    { 0.459292588292723, 0.459292588292723, 0.095091634267285 },
    { 0.658861384496480, 0.170569307751760, 0.103217370534718 },
    { 0.170569307751760, 0.658861384496480, 0.103217370534718 },
    { 0.170569307751760, 0.170569307751760, 0.103217370534718 },
    { 0.898905543365938, 0.050547228317031, 0.032458497623198 },
    { 0.050547228317031, 0.898905543365938, 0.032458497623198 },
    { 0.050547228317031, 0.050547228317031, 0.032458497623198 },
    { 0.008394777409958, 0.263112829634638, 0.027230314174435 },
    { 0.008394777409958, 0.728492392955404, 0.027230314174435 },
    { 0.263112829634638, 0.728492392955404, 0.027230314174435 },
    { 0.263112829634638, 0.008394777409958, 0.027230314174435 },
    { 0.728492392955404, 0.008394777409958, 0.027230314174435 },
    { 0.728492392955404, 0.263112829634638, 0.027230314174435 },
  }
};

void makeLinearSystem_PotentialFlow_Order1st
(std::vector<double>& A,
std::vector<double>& f,
//
const dfm2::CVector3& velo,
int ngauss,
const std::vector<double>& aXYZ,
const std::vector<int>& aTri,
const std::vector<double>& aSolidAngle)
{
  const int np = (int)aXYZ.size()/3;
  A.assign(np*np, 0.0);
  f.assign(np, 0.0);
  for (int ip = 0; ip<np; ip++){
    dfm2::CVector3 p(aXYZ[ip*3+0], aXYZ[ip*3+1], aXYZ[ip*3+2]);
    for (int jtri = 0; jtri<(int)aTri.size()/3; ++jtri){
      const int jq0 = aTri[jtri*3+0];
      const int jq1 = aTri[jtri*3+1];
      const int jq2 = aTri[jtri*3+2];
      const dfm2::CVector3 q0(aXYZ[jq0*3+0], aXYZ[jq0*3+1], aXYZ[jq0*3+2]);
      const dfm2::CVector3 q1(aXYZ[jq1*3+0], aXYZ[jq1*3+1], aXYZ[jq1*3+2]);
      const dfm2::CVector3 q2(aXYZ[jq2*3+0], aXYZ[jq2*3+1], aXYZ[jq2*3+2]);
      dfm2::CVector3 n = Normal(q0, q1, q2);
      const double area = n.Length()*0.5; // area
      n.SetNormalizedVector(); // unit normal
      n *= -1; // outward pointing vector
      double dC[3] = { 0, 0, 0 };
      double df = 0;
      const int nint = NIntTriGauss[ngauss]; // number of integral points
      for (int iint = 0; iint<nint; iint++){
        double r0 = TriGauss[ngauss][iint][0];
        double r1 = TriGauss[ngauss][iint][1];
        double r2 = 1.0-r0-r1;
        double wb = TriGauss[ngauss][iint][2];
        dfm2::CVector3 yb = r0*q0+r1*q1+r2*q2;
        dfm2::CVector3 v = (p-yb);
        double len = v.Length();
        double G = 1.0/(4*M_PI*len);
        double dGdn = (v*n)/(4*M_PI*len*len*len);
        {
          double wav = wb*area*dGdn;
          dC[0] += r0*wav; 
          dC[1] += r1*wav; 
          dC[2] += r2*wav; 
        }
        {
          double vnyb = -n*velo;
          double val = vnyb*G;
          df += wb*area*val; 
        }
      }
      A[ip*np+jq0] += dC[0];
      A[ip*np+jq1] += dC[1];
      A[ip*np+jq2] += dC[2];
      f[ip] += df;
    }
    A[ip*np+ip] += aSolidAngle[ip]/(4*M_PI);
    //A[ip*np+ip] += aSolidAngle[ip];
  }
}

dfm2::CVector3 evaluateField_PotentialFlow_Order1st
(double& phi_pos,
const dfm2::CVector3& pos,
const dfm2::CVector3& velo_inf,
int ngauss,
const std::vector<double>& aValSrf,
const std::vector<double>& aXYZ,
const std::vector<int>& aTri)
{
  const int np = (int)aXYZ.size()/3;
  if (aValSrf.size()!=np){ return dfm2::CVector3(0, 0, 0); }
  dfm2::CVector3 gradphi_pos = dfm2::CVector3(0, 0, 0);
  phi_pos = 0;
  for (int jtri = 0; jtri<(int)aTri.size()/3; ++jtri){
    const int jq0 = aTri[jtri*3+0];
    const int jq1 = aTri[jtri*3+1];
    const int jq2 = aTri[jtri*3+2];
    const dfm2::CVector3 q0(aXYZ[jq0*3+0], aXYZ[jq0*3+1], aXYZ[jq0*3+2]);
    const dfm2::CVector3 q1(aXYZ[jq1*3+0], aXYZ[jq1*3+1], aXYZ[jq1*3+2]);
    const dfm2::CVector3 q2(aXYZ[jq2*3+0], aXYZ[jq2*3+1], aXYZ[jq2*3+2]);
    assert(ngauss>=0&&ngauss<6);
    dfm2::CVector3 n = Normal(q0, q1, q2);
    const double area = n.Length()*0.5; // area
    n.SetNormalizedVector(); // unit normal
    n *= -1; // outward normal 
    const int nint = NIntTriGauss[ngauss]; // number of integral points
    for (int iint = 0; iint<nint; iint++){
      double r0 = TriGauss[ngauss][iint][0];
      double r1 = TriGauss[ngauss][iint][1];
      double r2 = 1.0-r0-r1;
      double wb = TriGauss[ngauss][iint][2];
      dfm2::CVector3 yb = r0*q0+r1*q1+r2*q2;
      double phiyb = r0*aValSrf[jq0]+r1*aValSrf[jq1]+r2*aValSrf[jq2];
      dfm2::CVector3 v = (pos-yb);
      double len = v.Length();
      double G = 1.0/(4*M_PI*len);
      double dGdn = (v*n)/(4*M_PI*len*len*len);
      dfm2::CVector3 dGdx = -v/(4*M_PI*len*len*len);
      dfm2::CVector3 dGdndx = (1/(4*M_PI*len*len*len))*n-(3*(v*n)/(4*M_PI*len*len*len*len*len))*v;
      double vnyb = -n*velo_inf;
      {
        double phyx = dGdn*phiyb-G*vnyb;
        phi_pos -= wb*area*phyx;
      }
      {
        dfm2::CVector3 gradphyx = dGdndx*phiyb-dGdx*vnyb;
        gradphi_pos -= wb*area*gradphyx;
      }
    }
  }
  gradphi_pos += velo_inf;
  return gradphi_pos;
}


void makeLinearSystem_PotentialFlow_Order0th(
    std::vector<double>& A,
    std::vector<double>& f,
    //
    const dfm2::CVector3& velo_inf,
    int ngauss,
    const std::vector<double>& aXYZ,
    const std::vector<unsigned int> &aTri)
{
  const std::size_t nt = aTri.size()/3;
  A.assign(nt*nt, 0.0);
  f.assign(nt, 0.0);
  for (std::size_t it = 0; it<nt; it++){
    const dfm2::CVector3 pm = MidPoint(it, aTri, aXYZ);
    for (std::size_t jt = 0; jt<nt; ++jt){
      if (it==jt) continue;
      const unsigned int jq0 = aTri[jt*3+0];
      const unsigned int jq1 = aTri[jt*3+1];
      const unsigned int jq2 = aTri[jt*3+2];
      const dfm2::CVector3 q0(aXYZ[jq0*3+0], aXYZ[jq0*3+1], aXYZ[jq0*3+2]);
      const dfm2::CVector3 q1(aXYZ[jq1*3+0], aXYZ[jq1*3+1], aXYZ[jq1*3+2]);
      const dfm2::CVector3 q2(aXYZ[jq2*3+0], aXYZ[jq2*3+1], aXYZ[jq2*3+2]);
      dfm2::CVector3 ny = Normal(q0, q1, q2);
      const double area = ny.Length()*0.5; // area
      ny.SetNormalizedVector(); // unit normal
      ny *= -1; // it is pointing outward to the domain
      double aC = 0;
      double df = 0;
      const int nint = NIntTriGauss[ngauss]; // number of integral points
      for (int iint = 0; iint<nint; iint++){
        double r0 = TriGauss[ngauss][iint][0];
        double r1 = TriGauss[ngauss][iint][1];
        double r2 = 1.0-r0-r1;
        double wb = TriGauss[ngauss][iint][2];
        dfm2::CVector3 yb = r0*q0+r1*q1+r2*q2;
        dfm2::CVector3 r = (pm-yb);
        double len = r.Length();
        double G = 1.0/(4*M_PI*len);
        double dGdn = (r*ny)/(4*M_PI*len*len*len);
        {
          double wav = wb*area*dGdn;
          aC += wav;  // should be plus
        }
        {
          double vnyb = -ny*velo_inf;
          double val = vnyb*G;
          df += wb*area*val;  // should be plus
        }
      }
      A[it*nt+jt] = aC;
      f[it] += df;
    }
    A[it*nt+it] += 0.5; 
  }
  /*
  {
    double sum = 0;
    for (int jt = 0; jt<nt; ++jt){
      double row = 0;
      for (int it = 0; it<nt; ++it){
        row += A[it*nt+jt];
      }
      sum += row;
      std::cout<<"hoge"<<jt<<" "<<row<<std::endl;
    }
    std::cout<<"sum: "<<sum<<std::endl;
  }
  */
}


void evaluateField_PotentialFlow_Order0th(
    double& phi_pos,
    dfm2::CVector3& gradphi_pos,
    //
    const dfm2::CVector3& pos,
    const dfm2::CVector3& velo_inf,
    int ngauss,
    const std::vector<double>& aValTri,
    const std::vector<double>& aXYZ,
    const std::vector<unsigned int> &aTri)
{
  const int nt = (int)aTri.size()/3;
  if (aValTri.size()!=nt){
    gradphi_pos = dfm2::CVector3(0,0,0);
    return;
  }
  gradphi_pos = dfm2::CVector3(0, 0, 0);
  phi_pos = 0;
  for (int jtri = 0; jtri<nt; ++jtri){
    const int jq0 = aTri[jtri*3+0];
    const int jq1 = aTri[jtri*3+1];
    const int jq2 = aTri[jtri*3+2];
    const dfm2::CVector3 q0(aXYZ[jq0*3+0], aXYZ[jq0*3+1], aXYZ[jq0*3+2]);
    const dfm2::CVector3 q1(aXYZ[jq1*3+0], aXYZ[jq1*3+1], aXYZ[jq1*3+2]);
    const dfm2::CVector3 q2(aXYZ[jq2*3+0], aXYZ[jq2*3+1], aXYZ[jq2*3+2]);
    dfm2::CVector3 ny = Normal(q0, q1, q2);
    const double area = ny.Length()*0.5; // area
    const double elen = sqrt(area*2)*0.5;
    ny.SetNormalizedVector(); // unit normal
    ny *= -1; // normal pointing outward
    const double phiy = aValTri[jtri];
    const int nint = NIntTriGauss[ngauss]; // number of integral points
    for (int iint = 0; iint<nint; iint++){
      const double r0 = TriGauss[ngauss][iint][0];
      const double r1 = TriGauss[ngauss][iint][1];
      const double r2 = 1.0-r0-r1;
      const double wb = TriGauss[ngauss][iint][2];
      const dfm2::CVector3 yb = r0*q0+r1*q1+r2*q2;
      const dfm2::CVector3 r = (pos-yb);
      const double len = r.Length();
      double G = 1.0/(4*M_PI*len);
      double dGdny = (r*ny)/(4*M_PI*len*len*len);
      dfm2::CVector3 dGdx = -r/(4*M_PI*len*len*len);
      dfm2::CVector3 dGdnydx = (1/(4*M_PI*len*len*len))*ny-(3*(r*ny)/(4*M_PI*len*len*len*len*len))*r;
      ///
//      const double reg = 1.0-exp(-(len*len*len)/(elen*elen*elen));
//      G *= reg;
//      dGdny *= reg;
//      dGdx *= reg;
//      dGdnydx *= reg;
      ////
      const double vnyb = -ny*velo_inf;
      {
        double phyx = -dGdny*phiy+G*vnyb;
        phi_pos += wb*area*phyx; // should be plus
      }
      {
        dfm2::CVector3 gradphyx = -dGdnydx*phiy+dGdx*vnyb;
        gradphi_pos += wb*area*gradphyx; // should be plus
      }
    }
  }
  gradphi_pos += velo_inf;
}

// ---------------------------------------------

void BEM_VortexSheet_Coeff_0th
(double aC[4], 
const dfm2::CVector3& x0, const dfm2::CVector3& x1, const dfm2::CVector3& x2,
const dfm2::CVector3& y0, const dfm2::CVector3& y1, const dfm2::CVector3& y2,
const dfm2::CVector3& velo,
int ngauss)
{
  assert(ngauss>=0&&ngauss<6);
  const int nint = NIntTriGauss[ngauss]; // number of integral points
  //
  dfm2::CVector3 xm = (x0+x1+x2)*0.3333333333333333333;
  dfm2::CVector3 nx = Normal(x0, x1, x2);
//  const double areax = nx.Length()*0.5; // area
  nx.SetNormalizedVector(); // unit normal
  const dfm2::CVector3 ux = (x1-x0).Normalize();
  const dfm2::CVector3 vx = nx^ux;
  //
  dfm2::CVector3 ny = Normal(y0, y1, y2);
  const double areay = ny.Length()*0.5; // area
  ny.SetNormalizedVector(); // unit normal
  const dfm2::CVector3 uy = (y1-y0).Normalize();
  const dfm2::CVector3 vy = ny^uy;
  //
  aC[0] = aC[1] = aC[2] = aC[3] = 0.0;
  for (int iint = 0; iint<nint; iint++){
    double r0 = TriGauss[ngauss][iint][0];
    double r1 = TriGauss[ngauss][iint][1];
    double r2 = 1.0-r0-r1;
    double wb = TriGauss[ngauss][iint][2];
    dfm2::CVector3 yb = r0*y0+r1*y1+r2*y2;
    dfm2::CVector3 r = (xm-yb);
    double len = r.Length();
//    double G = -1.0/(4*M_PI*len);
//    double dGdn = -(r*ny)/(4*M_PI*len*len*len);
    dfm2::CVector3 dGdy = -r/(4*M_PI*len*len*len);
    dfm2::CVector3 pvycdGdy = -(vy^r)/(4*M_PI*len*len*len); // +vy ^ dGdy = (ny^uy)^dGdy
    dfm2::CVector3 muycdGdy = +(uy^r)/(4*M_PI*len*len*len); // -uy ^ dGdy = (ny^vy)^dGdy
    {
      aC[0] += wb*areay*(pvycdGdy*ux);
      aC[1] += wb*areay*(muycdGdy*ux);
      aC[2] += wb*areay*(pvycdGdy*vx);
      aC[3] += wb*areay*(muycdGdy*vx);
    } 
  }
}

void makeLinearSystem_VortexSheet_Order0th
(std::vector<double>& A,
std::vector<double>& f,
/////
const dfm2::CVector3& velo,
int ngauss,
const std::vector<double>& aXYZ,
const std::vector<int>& aTri)
{
  const int nt = (int)aTri.size()/3;
  A.assign(4*nt*nt, 0.0);
  f.assign(2*nt, 0.0);
  for (int it = 0; it<nt; ++it){
    const int ip0 = aTri[it*3+0];
    const int ip1 = aTri[it*3+1];
    const int ip2 = aTri[it*3+2];
    const dfm2::CVector3 p0(aXYZ[ip0*3+0], aXYZ[ip0*3+1], aXYZ[ip0*3+2]);
    const dfm2::CVector3 p1(aXYZ[ip1*3+0], aXYZ[ip1*3+1], aXYZ[ip1*3+2]);
    const dfm2::CVector3 p2(aXYZ[ip2*3+0], aXYZ[ip2*3+1], aXYZ[ip2*3+2]);
    {
      const dfm2::CVector3 nx = Normal(p0, p1, p2).Normalize();
      const dfm2::CVector3 ux = (p1-p0).Normalize();
      const dfm2::CVector3 vx = (nx^ux);
      f[it*2+0] = ux*velo;
      f[it*2+1] = vx*velo;
    }
    for (int jt = 0; jt<nt; ++jt){
      if (it==jt) continue;
      const int jq0 = aTri[jt*3+0];
      const int jq1 = aTri[jt*3+1];
      const int jq2 = aTri[jt*3+2];
      const dfm2::CVector3 q0(aXYZ[jq0*3+0], aXYZ[jq0*3+1], aXYZ[jq0*3+2]);
      const dfm2::CVector3 q1(aXYZ[jq1*3+0], aXYZ[jq1*3+1], aXYZ[jq1*3+2]);
      const dfm2::CVector3 q2(aXYZ[jq2*3+0], aXYZ[jq2*3+1], aXYZ[jq2*3+2]);
      double aC[4];
      BEM_VortexSheet_Coeff_0th(aC, 
        p0, p1, p2,
        q0, q1, q2, 
        velo, ngauss);
      A[(2*it+0)*(2*nt)+(2*jt+0)] = aC[0];
      A[(2*it+0)*(2*nt)+(2*jt+1)] = aC[1];
      A[(2*it+1)*(2*nt)+(2*jt+0)] = aC[2];
      A[(2*it+1)*(2*nt)+(2*jt+1)] = aC[3];
    }
    A[(2*it+0)*(2*nt)+(2*it+0)] = 0.5;
    A[(2*it+0)*(2*nt)+(2*it+1)] = 0.0;
    A[(2*it+1)*(2*nt)+(2*it+0)] = 0.0;
    A[(2*it+1)*(2*nt)+(2*it+1)] = 0.5;
  }
}

dfm2::CVector3 evaluateField_VortexSheet_Order0th
(const dfm2::CVector3& pos,
 const std::vector<double>& aValSrf,
 //
 int ngauss,
 const std::vector<double>& aXYZ,
 const std::vector<int>& aTri, 
 int jtri_exclude)
{
  assert(ngauss>=0&&ngauss<6);
  const int nt = (int)aTri.size()/3;
  dfm2::CVector3 velo_res(0,0,0);
  for (int jt = 0; jt<nt; ++jt){
    if (jt==jtri_exclude){ continue; }
    const int jq0 = aTri[jt*3+0];
    const int jq1 = aTri[jt*3+1];
    const int jq2 = aTri[jt*3+2];
    const dfm2::CVector3 q0(aXYZ[jq0*3+0], aXYZ[jq0*3+1], aXYZ[jq0*3+2]);
    const dfm2::CVector3 q1(aXYZ[jq1*3+0], aXYZ[jq1*3+1], aXYZ[jq1*3+2]);
    const dfm2::CVector3 q2(aXYZ[jq2*3+0], aXYZ[jq2*3+1], aXYZ[jq2*3+2]);
    dfm2::CVector3 ny = Normal(q0, q1, q2);
    const double areay = ny.Length()*0.5; // area
    ny.SetNormalizedVector(); // unit normal
    const dfm2::CVector3 uy = (q1-q0).Normalize();
    const dfm2::CVector3 vy = ny^uy;
    const int nint = NIntTriGauss[ngauss]; // number of integral points
    for (int iint = 0; iint<nint; iint++){
      const double r0 = TriGauss[ngauss][iint][0];
      const double r1 = TriGauss[ngauss][iint][1];
      const double r2 = 1.0-r0-r1;
      const double wb = TriGauss[ngauss][iint][2];
      dfm2::CVector3 yb = r0*q0+r1*q1+r2*q2;
      dfm2::CVector3 r = (pos-yb);
      const double len = r.Length();
      //    double G = -1.0/(4*M_PI*len);
      //    double dGdn = -(r*ny)/(4*M_PI*len*len*len);
      //    dfm2::CVector3 dGdy = -r/(4*M_PI*len*len*len);
      dfm2::CVector3 pvycdGdy = -(vy^r)/(4*M_PI*len*len*len); // +vy ^ dGdy = (ny^uy)^dGdy
      dfm2::CVector3 muycdGdy = +(uy^r)/(4*M_PI*len*len*len); // -uy ^ dGdy = (ny^vy)^dGdy
      velo_res -= wb*areay*(pvycdGdy*aValSrf[jt*2+0]+muycdGdy*aValSrf[jt*2+1]);
    }
  }
  return velo_res;
}


// -------------------------------------------------

dfm2::CVector3 veloVortexParticle
(const dfm2::CVector3& pos_eval,
const dfm2::CVector3& pos_vp,
const dfm2::CVector3& circ_vp,
double rad_vp)
{
  dfm2::CVector3 v = pos_eval-pos_vp;
  double len = v.Length();
  double ratio = len/rad_vp;
  double f0 = 1.0-exp(-ratio*ratio*ratio);
//  double f0 = 1.0;
  double g0 = f0/(4*M_PI*len*len*len);
  dfm2::CVector3 k0 = g0*(circ_vp^v);
  return g0*(circ_vp^v);
}

CMatrix3 gradveloVortexParticle
(dfm2::CVector3& velo_eval,
const dfm2::CVector3& pos_eval,
const dfm2::CVector3& pos_vp,
const dfm2::CVector3& circ_vp,
double rad_vp)
{
  dfm2::CVector3 v = pos_eval-pos_vp;
  double len = v.Length();
  double ratio = len/rad_vp;
  double f0 = 1.0-exp(-ratio*ratio*ratio);
  double g0 = f0/(4*M_PI*len*len*len);
  velo_eval = g0*(circ_vp^v);
  ///
  dfm2::CVector3 dlen = v.Normalize();
  dfm2::CVector3 dratio = dlen/rad_vp;
  dfm2::CVector3 df0 = (exp(-ratio*ratio*ratio)*3*ratio*ratio)*dratio;
  dfm2::CVector3 dg0 = (1.0/(4*M_PI*len*len*len))*df0-(3*f0/(4*M_PI*len*len*len*len))*dlen;
  return dfm2::Mat3_OuterProduct(circ_vp^v, dg0)+g0*dfm2::Mat3_Spin(circ_vp);
}

dfm2::CVector3 veloVortexParticles
(const dfm2::CVector3& p0,
const std::vector<CVortexParticle>& aVortexParticle,
int ivp_self)
{
  dfm2::CVector3 velo_res(0, 0, 0);
  for (int ivp = 0; ivp<aVortexParticle.size(); ++ivp){
    if (ivp==ivp_self) continue;
    const CVortexParticle& vp = aVortexParticle[ivp];
    velo_res += veloVortexParticle(p0, vp.pos, vp.circ, vp.rad);
  }
  return velo_res;
}

CMatrix3 gradveloVortexParticles
(dfm2::CVector3& velo,
const dfm2::CVector3& p0,
const std::vector<CVortexParticle>& aVortexParticle,
int ivp_self)
{
  CMatrix3 m_res; m_res.SetZero();
  velo = dfm2::CVector3(0, 0, 0);
  for (int ivp = 0; ivp<aVortexParticle.size(); ++ivp){
    if (ivp==ivp_self) continue;
    const CVortexParticle& vp = aVortexParticle[ivp];
    dfm2::CVector3 dv;
    m_res += gradveloVortexParticle(dv, p0, vp.pos, vp.circ, vp.rad);
    velo += dv;
  }
  return m_res;
}

void setGradVeloVortexParticles
(std::vector<CVortexParticle>& aVortexParticle)
{
  for (int ivp = 0; ivp<aVortexParticle.size(); ++ivp){
    CVortexParticle& vp = aVortexParticle[ivp];
    vp.velo_pre = vp.velo;
    vp.gradvelo_pre = vp.gradvelo;
  }
  for (int ivp = 0; ivp<aVortexParticle.size(); ++ivp){
//    dfm2::CVector3 velo = veloVortexParticles(aVortexParticle[ivp].pos, aVortexParticle, ivp);
    dfm2::CVector3 velo;
    CMatrix3 grad_velo = gradveloVortexParticles(velo, aVortexParticle[ivp].pos, aVortexParticle, ivp);
    aVortexParticle[ivp].velo = velo;
    aVortexParticle[ivp].gradvelo = grad_velo;
    /*
    aVortexParticle[ivp].gradvelo = grad_velo;
    CMatrix3 D = (grad_velo+grad_velo.Trans())*0.5;
    double tr = D.Trace()/3.0;
    CMatrix3 D2 = D-tr*CMatrix3::Identity();
    aVortexParticle[ivp].m = aVortexParticle[ivp].m*D2;
    */
  }
}


void CGrid_Vortex::drawBoundingBox() const
{
  if (nx<=1||ny<=1||nz<=1){ return; }
  double min_x = cnt.x()-nx*h*0.5;
  double max_x = cnt.x()+nx*h*0.5;
  double min_y = cnt.y()-ny*h*0.5;
  double max_y = cnt.y()+ny*h*0.5;
  double min_z = cnt.z()-nz*h*0.5;
  double max_z = cnt.z()+nz*h*0.5;
  ::glDisable(GL_LIGHTING);
  ::glBegin(GL_LINES);
  ::glVertex3d(min_x, min_y, min_z); ::glVertex3d(max_x, min_y, min_z);
  ::glVertex3d(min_x, min_y, max_z); ::glVertex3d(max_x, min_y, max_z);
  ::glVertex3d(min_x, max_y, max_z); ::glVertex3d(max_x, max_y, max_z);
  ::glVertex3d(min_x, max_y, min_z); ::glVertex3d(max_x, max_y, min_z);
  ////
  ::glVertex3d(min_x, min_y, min_z); ::glVertex3d(min_x, max_y, min_z);
  ::glVertex3d(min_x, min_y, max_z); ::glVertex3d(min_x, max_y, max_z);
  ::glVertex3d(max_x, min_y, max_z); ::glVertex3d(max_x, max_y, max_z);
  ::glVertex3d(max_x, min_y, min_z); ::glVertex3d(max_x, max_y, min_z);
  ////
  ::glVertex3d(min_x, min_y, min_z);  ::glVertex3d(min_x, min_y, max_z);
  ::glVertex3d(min_x, max_y, min_z);  ::glVertex3d(min_x, max_y, max_z);
  ::glVertex3d(max_x, max_y, min_z);  ::glVertex3d(max_x, max_y, max_z);
  ::glVertex3d(max_x, min_y, min_z);  ::glVertex3d(max_x, min_y, max_z);
  ::glEnd();
}


void viscousityVortexParticleGrid
(std::vector<CVortexParticle>& aVortexParticle, CGrid_Vortex& grid, double resolution)
{
  grid.h = resolution;
  const double h = grid.h;
  double min_x, max_x;
  double min_y, max_y;
  double min_z, max_z;
  if (aVortexParticle.size()==0){
    grid.nx = 0; grid.ny = 0; grid.nz = 0;
    grid.cnt = dfm2::CVector3(0, 0, 0);
    return;
  }
  min_x = max_x = aVortexParticle[0].pos.x();
  min_y = max_y = aVortexParticle[0].pos.y();
  min_z = max_z = aVortexParticle[0].pos.z();
  for (int ivp = 1; ivp<aVortexParticle.size(); ++ivp){
    dfm2::CVector3 p = aVortexParticle[ivp].pos;
    if (p.x()<min_x){ min_x = p.x(); }
    if (p.x()>max_x){ max_x = p.x(); }
    if (p.y()<min_y){ min_y = p.y(); }
    if (p.y()>max_y){ max_y = p.y(); }
    if (p.z()<min_z){ min_z = p.z(); }
    if (p.z()>max_z){ max_z = p.z(); }
  }
  grid.cnt = dfm2::CVector3(min_x+max_x, min_y+max_y, min_z+max_z)*0.5;
  max_x += h; 
  max_y += h;
  max_z += h;
  min_x -= h;
  min_y -= h;
  min_z -= h;
  double wx = max_x-min_x;
  double wy = max_y-min_y;
  double wz = max_z-min_z;
  grid.nx = wx/h+1;
  grid.ny = wy/h+1;
  grid.nz = wz/h+1;
  const int nx = grid.nx;
  const int ny = grid.ny;
  const int nz = grid.nz;
  ///////
  grid.aDataVtx.resize((nx+1)*(ny+1)*(nz+1));
  for (int igp = 0; igp<grid.aDataVtx.size(); ++igp){
    grid.aDataVtx[igp].aPairPtcleWeight.clear();
    grid.aDataVtx[igp].circ.SetZero();
  }
  for (int ivp = 0; ivp<aVortexParticle.size(); ++ivp){
    dfm2::CVector3 p = aVortexParticle[ivp].pos;
    int ix = (p.x()-(grid.cnt.x()-h*nx*0.5))/h;
    int iy = (p.y()-(grid.cnt.y()-h*ny*0.5))/h;
    int iz = (p.z()-(grid.cnt.z()-h*nz*0.5))/h;
    assert(ix>=0&&ix<nx);
    assert(iy>=0&&iy<ny);
    assert(iz>=0&&iz<nz);
    double rx = (p.x()-(grid.cnt.x()-h*nx*0.5)-ix*h)/h;
    double ry = (p.y()-(grid.cnt.y()-h*ny*0.5)-iy*h)/h;
    double rz = (p.z()-(grid.cnt.z()-h*nz*0.5)-iz*h)/h;
    assert(rx>=-1.0e-5&&rx<=1.0+1.0e-5);
    assert(ry>=-1.0e-5&&ry<=1.0+1.0e-5);
    assert(rz>=-1.0e-5&&rz<=1.0+1.0e-5);
    int igp0 = (ix+0)*(ny+1)*(nz+1)+(iy+0)*(nz+1)+(iz+0); double w0 = (1.0-rx)*(1.0-ry)*(1.0-rz);
    int igp1 = (ix+1)*(ny+1)*(nz+1)+(iy+0)*(nz+1)+(iz+0); double w1 = (rx+0.0)*(1.0-ry)*(1.0-rz);
    int igp2 = (ix+1)*(ny+1)*(nz+1)+(iy+1)*(nz+1)+(iz+0); double w2 = (rx+0.0)*(ry+0.0)*(1.0-rz);
    int igp3 = (ix+0)*(ny+1)*(nz+1)+(iy+1)*(nz+1)+(iz+0); double w3 = (1.0-rx)*(ry+0.0)*(1.0-rz);
    int igp4 = (ix+0)*(ny+1)*(nz+1)+(iy+0)*(nz+1)+(iz+1); double w4 = (1.0-rx)*(1.0-ry)*(rz+0.0);
    int igp5 = (ix+1)*(ny+1)*(nz+1)+(iy+0)*(nz+1)+(iz+1); double w5 = (rx+0.0)*(1.0-ry)*(rz+0.0);
    int igp6 = (ix+1)*(ny+1)*(nz+1)+(iy+1)*(nz+1)+(iz+1); double w6 = (rx+0.0)*(ry+0.0)*(rz+0.0);
    int igp7 = (ix+0)*(ny+1)*(nz+1)+(iy+1)*(nz+1)+(iz+1); double w7 = (1.0-rx)*(ry+0.0)*(rz+0.0);
    assert(igp0>=0&&igp0<grid.aDataVtx.size());
    assert(igp1>=0&&igp1<grid.aDataVtx.size());
    assert(igp2>=0&&igp2<grid.aDataVtx.size());
    assert(igp3>=0&&igp3<grid.aDataVtx.size());
    assert(igp4>=0&&igp4<grid.aDataVtx.size());
    assert(igp5>=0&&igp5<grid.aDataVtx.size());
    assert(igp6>=0&&igp6<grid.aDataVtx.size());
    assert(igp7>=0&&igp7<grid.aDataVtx.size());
    grid.aDataVtx[igp0].circ += w0*aVortexParticle[ivp].circ;
    grid.aDataVtx[igp1].circ += w1*aVortexParticle[ivp].circ;
    grid.aDataVtx[igp2].circ += w2*aVortexParticle[ivp].circ;
    grid.aDataVtx[igp3].circ += w3*aVortexParticle[ivp].circ;
    grid.aDataVtx[igp4].circ += w4*aVortexParticle[ivp].circ;
    grid.aDataVtx[igp5].circ += w5*aVortexParticle[ivp].circ;
    grid.aDataVtx[igp6].circ += w6*aVortexParticle[ivp].circ;
    grid.aDataVtx[igp7].circ += w7*aVortexParticle[ivp].circ;
    grid.aDataVtx[igp0].aPairPtcleWeight.push_back(std::make_pair(ivp, w0));
    grid.aDataVtx[igp1].aPairPtcleWeight.push_back(std::make_pair(ivp, w1));
    grid.aDataVtx[igp2].aPairPtcleWeight.push_back(std::make_pair(ivp, w2));
    grid.aDataVtx[igp3].aPairPtcleWeight.push_back(std::make_pair(ivp, w3));
    grid.aDataVtx[igp4].aPairPtcleWeight.push_back(std::make_pair(ivp, w4));
    grid.aDataVtx[igp5].aPairPtcleWeight.push_back(std::make_pair(ivp, w5));
    grid.aDataVtx[igp6].aPairPtcleWeight.push_back(std::make_pair(ivp, w6));
    grid.aDataVtx[igp7].aPairPtcleWeight.push_back(std::make_pair(ivp, w7));
  }
  double damp_ratio = 1.0;
  for (int ivp = 0; ivp<aVortexParticle.size(); ++ivp){
    aVortexParticle[ivp].circ *= (1.0-damp_ratio);
  }
  for (int igp = 0; igp<grid.aDataVtx.size(); ++igp){
    const CGrid_Vortex::CDataVtx& data = grid.aDataVtx[igp];
    if (data.aPairPtcleWeight.empty()) continue;
    double sum_w = 0;
    for (int iivp = 0; iivp<data.aPairPtcleWeight.size(); ++iivp){
      sum_w += data.aPairPtcleWeight[iivp].second;
    }
    if (sum_w<1.0e-5) continue;
    double inv_sum_w = 1.0/sum_w;    
    for (int iivp = 0; iivp<data.aPairPtcleWeight.size(); ++iivp){    
      int ivp0 = data.aPairPtcleWeight[iivp].first;
      double w = data.aPairPtcleWeight[iivp].second*inv_sum_w;
      assert(ivp0>=0&&ivp0<aVortexParticle.size());
      aVortexParticle[ivp0].circ += damp_ratio*w*grid.aDataVtx[igp].circ;
    }
  }
}


///////////////////////////////////////////////////////////////////////////////////////

COMPLEX evaluateField_Helmholtz_Order0th(
    const std::vector<COMPLEX>& aSol,
    const dfm2::CVector3& p,
    const dfm2::CVector3& pos_source,
    double k, // wave number
    //
    double Adm, // admittance
    const std::vector<unsigned int> &aTri,
    const std::vector<double>& aXYZ,
    bool is_inverted_norm)
{
  COMPLEX c1;
  {
    double rs = (p-pos_source).Length();
    c1 = exp(rs*k*IMG)/(4*M_PI*rs);
  }
  int ntri = (int)aTri.size()/3;
  for (int jtri = 0; jtri<ntri; jtri++){
    dfm2::CVector3 pmj = MidPoint(jtri, aTri, aXYZ);
    double rm = (p-pmj).Length();
    dfm2::CVector3 n = NormalTri(jtri, aTri, aXYZ);
    if (is_inverted_norm){ n *= -1; }
    double area = n.Length()*0.5;
    n.SetNormalizedVector();
    COMPLEX G = exp(rm*k*IMG)/(4*M_PI*rm);
    COMPLEX dGdr = G*(IMG*k-1.0/rm);
    double drdn = (1.0/rm)*((p-pmj)*n);
    c1 -= area*aSol[jtri]*(dGdr*drdn-IMG*k*Adm*G);
  }
  return c1;
}

//////////////////////////////////////////////////////////////////////////////////////

void Helmholtz_TransferOrder1st_PntTri
(COMPLEX aC[3],
const dfm2::CVector3& p0,
const dfm2::CVector3& q0, const dfm2::CVector3& q1, const dfm2::CVector3& q2,
double k, double beta,
int ngauss)
{  
  assert(ngauss>=0&&ngauss<3);
  const int nint = NIntTriGauss[ngauss]; // number of integral points
  dfm2::CVector3 n = Normal(q0, q1, q2);
  const double a = n.Length()*0.5; // area
  n.SetNormalizedVector(); // unit normal
  aC[0] = aC[1] = aC[2] = COMPLEX(0, 0);
  for (int iint = 0; iint<nint; iint++){
    double r0 = TriGauss[ngauss][iint][0];
    double r1 = TriGauss[ngauss][iint][1];
    double r2 = 1.0-r0-r1;
    double w = TriGauss[ngauss][iint][2];
    dfm2::CVector3 v = p0-(r0*q0+r1*q1+r2*q2);
    double d = v.Length();  
    COMPLEX G = exp(COMPLEX(0, k*d))/(4.0*M_PI*d);
    COMPLEX val = G*(-IMG*k*beta+v*n/(d*d)*COMPLEX(1.0, -k*d));
    const COMPLEX wav = w*a*val;
    aC[0] += r0*wav;
    aC[1] += r1*wav;
    aC[2] += r2*wav;
  }
}

COMPLEX evaluateField_Helmholtz_Order1st
(const std::vector<COMPLEX>& aSol,
const dfm2::CVector3& p,
const dfm2::CVector3& pos_source,
double k, // wave number
double beta, // admittance
const std::vector<int>& aTri,
const std::vector<double>& aXYZ,
bool is_inverted_norm,
int ngauss)
{
  COMPLEX c1;
  {
    double rs = (p-pos_source).Length();
    c1 = exp(rs*k*IMG)/(4*M_PI*rs);
  }
  int ntri = (int)aTri.size()/3;
  for (int jtri = 0; jtri<ntri; jtri++){
    const int jn0 = aTri[jtri*3+0];
    const int jn1 = aTri[jtri*3+1];
    const int jn2 = aTri[jtri*3+2];
    dfm2::CVector3 q0(aXYZ[jn0*3+0], aXYZ[jn0*3+1], aXYZ[jn0*3+2]);
    dfm2::CVector3 q1(aXYZ[jn1*3+0], aXYZ[jn1*3+1], aXYZ[jn1*3+2]);
    dfm2::CVector3 q2(aXYZ[jn2*3+0], aXYZ[jn2*3+1], aXYZ[jn2*3+2]);
    COMPLEX aC[3];  Helmholtz_TransferOrder1st_PntTri(aC, p, q0, q1, q2, k, beta, ngauss);
    c1 -= aC[0]*aSol[jn0]+aC[1]*aSol[jn1]+aC[2]*aSol[jn2];
  }
  return c1;
}

/*
void MakeMatrix_Helmholtz_Order1st
(Eigen::PartialPivLU<Eigen::MatrixXcd>& solver,
double k,
double beta,
const std::vector<int>& aTri,
const std::vector<double>& aXYZ,
const std::vector<double>& aSolidAngle,
bool is_inverted_norm,
int ngauss)
{
  const int nno = aXYZ.size()/3;
  const int ntri = (int)aTri.size()/3;
  Eigen::MatrixXcd A(nno, nno);
  A.setZero();
  for (int ino = 0; ino<nno; ino++){
    dfm2::CVector3 p(aXYZ[ino*3+0], aXYZ[ino*3+1], aXYZ[ino*3+2]);
    for (int jtri = 0; jtri<ntri; jtri++){
      const int jn0 = aTri[jtri*3+0]; if (jn0==ino){ continue; }
      const int jn1 = aTri[jtri*3+1]; if (jn1==ino){ continue; }
      const int jn2 = aTri[jtri*3+2]; if (jn2==ino){ continue; }
      dfm2::CVector3 q0(aXYZ[jn0*3+0], aXYZ[jn0*3+1], aXYZ[jn0*3+2]);
      dfm2::CVector3 q1(aXYZ[jn1*3+0], aXYZ[jn1*3+1], aXYZ[jn1*3+2]);
      dfm2::CVector3 q2(aXYZ[jn2*3+0], aXYZ[jn2*3+1], aXYZ[jn2*3+2]);
      COMPLEX aC[3];  Helmholtz_TransferOrder1st_PntTri(aC, p, q0, q1, q2, k, beta, ngauss);
      A(ino, jn0) += aC[0];
      A(ino, jn1) += aC[1];
      A(ino, jn2) += aC[2];
//      A(jn0, ino) += aC[0];
//      A(jn1, ino) += aC[1];
//      A(jn2, ino) += aC[2];
    }
    A(ino, ino) += aSolidAngle[ino]/(4*M_PI);
  }
  solver.compute(A);
}
*/

dfm2::CVector3 evaluateField_PotentialFlow
(const std::vector<double>& aSol,
 const dfm2::CVector3& p,
 const std::vector<unsigned int> &aTri,
 const std::vector<double>& aXYZ)
{
  int ntri = (int)aTri.size()/3;
  dfm2::CVector3 c(0, 0, 0);
  for (int jtri = 0; jtri<ntri; jtri++){
    dfm2::CVector3 pmj = MidPoint(jtri, aTri, aXYZ);
    dfm2::CVector3 nj = NormalTri(jtri, aTri, aXYZ);
    double areaj = nj.Length()*0.5;
    dfm2::CVector3 v = p-pmj;
    double lenv = v.Length();
    c -= areaj*aSol[jtri]/(lenv*lenv*lenv)*v;
  }
  return c;
}

/*
void MakeMatrix_PotentialFlow
(Eigen::PartialPivLU<Eigen::MatrixXd>& solver,
const dfm2::CVector3& Velo,
const dfm2::CVector3& Omg,
const std::vector<int>& aTri,
const std::vector<double>& aXYZ)
{
  int ntri = (int)aTri.size()/3;
  Eigen::MatrixXd A(ntri, ntri);
  for (int itri = 0; itri<ntri; itri++){
    dfm2::CVector3 pmi = MidPoint(itri, aTri, aXYZ);
    dfm2::CVector3 ni = NormalTri(itri, aTri, aXYZ);
    ni.SetNormalizedVector();
    for (int jtri = 0; jtri<ntri; jtri++){
      dfm2::CVector3 pmj = MidPoint(jtri, aTri, aXYZ);
      dfm2::CVector3 nj = NormalTri(jtri, aTri, aXYZ);
      double areaj = nj.Length()*0.5;
      if (itri==jtri){ continue; }
      dfm2::CVector3 v = pmi-pmj;
      double lenv = v.Length();
      A(itri, jtri) = areaj/(lenv*lenv*lenv)*(v*ni);
    }
    double sum = 0;
    for (int jtri = 0; jtri<ntri; jtri++){
      if (itri==jtri){ continue; }
      sum += A(itri, jtri);
    }
    A(itri, itri) = (4*M_PI-sum);

  }
  solver.compute(A);
}
*/
