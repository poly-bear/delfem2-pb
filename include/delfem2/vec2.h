/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef DFM2_VEC2_H
#define DFM2_VEC2_H

#include <cstdlib>
#include <cassert>
#include <iostream>
#include <cmath>
#include <vector>

// -----------------------------------------------------

namespace delfem2 {
  
template <typename T>
T TriArea2D(const T v1[2], const T v2[2], const T v3[2]);

template <typename T>
T Dot2(const T w[2], const T v[2]);
 
template <typename T>
T Length2(const T v[2]);

template <typename T>
T Distance2(const T v1[2], const T v2[2]);
  
template <typename T>
void MatVec2(T w[2], const T A[4], const T v[2]);
  
template <typename T>
void MatMat2(T AB[4], const T A[4], const T B[4]);

template <typename T>
T SquareLength2(const T v[2]);

template <typename T>
T SquareDistance2(const T v1[2], const T v2[2]);
  
template <typename T>
void GaussianDistribution2(T noise[2]);
  
template <typename T>
void Normalize2(T w[2]);
}



bool InverseMat2(double invB[4], const double B[4]);
void gramian2(double AtA[3], const double A[4]);
void VLVt2(double A[4], double l0, double l1, const double V[4]);
void RotationalComponentOfMatrix2(double R[4], const double M[4]);

// -----------------------------------------------------

namespace delfem2 {

class CVector2;

CVector2 operator*(double, const CVector2&);
CVector2 operator*(const CVector2&, double);
std::ostream &operator<<(std::ostream &output, const CVector2& v);
std::istream &operator>>(std::istream &input, CVector2& v);

/**
 * @brief 2 dimensional vector class
 */
class CVector2{
public:
  CVector2(): p{0.0, 0.0} {}
	CVector2( const CVector2& rhs ){
		this->p[0] = rhs.p[0];
		this->p[1] = rhs.p[1];
	}
	CVector2(double x, double y){
		this->p[0] = x;
		this->p[1] = y;
	}
	
	friend double Dot(const CVector2&, const CVector2&);
  CVector2 operator-() const{
    return CVector2(-p[0],-p[1]);
  }
	inline CVector2& operator+=(const CVector2& rhs){
		p[0] += rhs.p[0];
		p[1] += rhs.p[1];
		return *this;
	}
	inline CVector2& operator-=(const CVector2& rhs){
		p[0] -= rhs.p[0];
		p[1] -= rhs.p[1];
		return *this;
	}
	inline CVector2& operator*=(double scale){
		p[0] *= scale;
		p[1] *= scale;
		return *this;
  }
  inline CVector2& operator/=(double d){
    if (fabs(d) < 1.0e-6){ assert(0); return *this; }
    p[0] /= d; p[1] /= d;
    return *this;
  }
	inline CVector2 operator+(const CVector2& rhs) const {
		CVector2 v = *this;
		return v += rhs;
	}
	inline CVector2 operator-(const CVector2& rhs) const {
		CVector2 v = *this;
		return v -= rhs;
	}
  inline double operator[](int i) const{
    if (i==0) return p[0];
    if (i==1) return p[1];
    return 0;
  }
  inline double& operator[](int i){
    if (i==0) return p[0];
    if (i==1) return p[1];
    assert(0);
    return p[0];
  }
	//! @brief normalize length
	inline void SetNormalizedVector(){
		const double mag = Length();
		p[0] /= mag;
		p[1] /= mag;
	}
  CVector2 Normalize() const {
    CVector2 r(*this);
    r.SetNormalizedVector();
    return r;
  }
	//! set zero vector
	inline void SetZero(){
		p[0] = 0.0;
		p[1] = 0.0;
	}
  inline double x() const { return p[0]; }
  inline double y() const { return p[1]; }
  double Length() const{
		return sqrt( p[0]*p[0]+p[1]*p[1] );
	}
  double SqLength() const{
		return p[0]*p[0]+p[1]*p[1];
	}
public:
  double p[2];
};

CVector2 operator*(double c, const CVector2& v0);
CVector2 operator*(const CVector2& v0, double c);
double operator * (const CVector2& lhs, const CVector2& rhs);
double operator ^ (const CVector2& lhs, const CVector2& rhs);
CVector2 operator/ (const CVector2& vec, double d); //! divide by real number
CVector2 rotate(const CVector2& p0, double theta);
CVector2 rotate90(const CVector2& p0);

// --------------------------------------------

CVector2 Mat2Vec(const double A[4], const CVector2& v);

//! @brief Area of the Triangle
double Area_Tri(const CVector2& v1,
               const CVector2& v2,
               const CVector2& v3);
double Cross(const CVector2& v1, const CVector2& v2);
double SquareDistance(const CVector2& ipo0, const CVector2& ipo1);
double SquareLength(const CVector2& point);
double Length(const CVector2& point);

//! @brief Length between two points
double Distance(const CVector2& ipo0, const CVector2& ipo1);

//! @brief Length between two points
double SquareDistance(const CVector2& ipo0, const CVector2& ipo1);

//! @brief Hight of a triangle : between v1 and line of v2-v3
double TriHeight(const CVector2& v1, const CVector2& v2, const CVector2& v3);

//! @brief compute dot product
inline double Dot(const CVector2& ipo0, const CVector2& ipo1);

//! @details get parameter 't' of the line against point. t=0 is po_s, t=1 is po_e
double FindNearestPointParameter_Line_Point(const CVector2& po_c,
                                            const CVector2& po_s, const CVector2& po_e);

//! @details  get parameter 't' of the line against point. t=0 is po_s, t=1 is po_e
CVector2 GetNearest_LineSeg_Point(const CVector2& po_c,
                                  const CVector2& po_s, const CVector2& po_e);

//! @details  get parameter 't' of the line against point. t=0 is po_s, t=1 is po_e
double GetDist_LineSeg_Point(const CVector2& po_c,
                             const CVector2& po_s, const CVector2& po_e);
  
bool IsCross_LineSeg_LineSeg(const CVector2& po_s0, const CVector2& po_e0,
                             const CVector2& po_s1, const CVector2& po_e1 );
  
double GetDist_LineSeg_LineSeg(const CVector2& po_s0, const CVector2& po_e0,
                               const CVector2& po_s1, const CVector2& po_e1);
  
/**
 * @brief square root of circumradius
 */
double SquareCircumradius(const CVector2& p0,
                          const CVector2& p1,
                          const CVector2& p2 );

/**
 * @brief center of the circumcircle
 */
bool CenterCircumcircle(const CVector2& p0,
                        const CVector2& p1,
                        const CVector2& p2,
                        CVector2& center);

/**
 * @brief check if Delaunay condition satisfied
 * @return
 * 0 : p3 is inside circum circle on the p0,p1,p2
 * 1 :       on
 * 2 :       outsdie
 */
int DetDelaunay(const CVector2& p0,
                const CVector2& p1,
                const CVector2& p2,
                const CVector2& p3);

// move to paramgeo2d?
CVector2 pointCurve_BezierCubic
(double t,
 const CVector2& p1, const CVector2& p2, const CVector2& p3, const CVector2& p4);


// ---------------------------------------------------------------


//! @brief translate all the points
void Translate(std::vector<CVector2>& aP,
               double dx, double dy);

void Rotate(std::vector<CVector2>& aP,
            double dt);

//! @brief Area of the Triangle (3 indexes and vertex array)
double Area_Tri(const int iv1, const int iv2, const int iv3,
               const std::vector<CVector2>& point );

void Polyline_CubicBezierCurve(std::vector<CVector2>& aP,
                               const int n,
                               const std::vector<CVector2>& aCP);

void Polyline_BezierCubic(std::vector<CVector2>& aP,
                          const unsigned int n,
                          const CVector2& p1, const CVector2& p2,
                          const CVector2& p3, const CVector2& p4);


std::vector<CVector2> Polygon_Resample_Polygon(const std::vector<CVector2>& stroke0,
                                               double l);
std::vector<CVector2> Polyline_Resample_Polyline(const std::vector<CVector2>& stroke0,
                                                 double l);
void SecondMomentOfArea_Polygon(CVector2& cg,  double& area,
                               CVector2& pa1, double& I1,
                               CVector2& pa2, double& I2,
                              const std::vector<CVector2>& aVec2D);
double Length_Polygon(const std::vector<CVector2>& aP);
double Area_Polygon(const std::vector<CVector2>& aP);


void MeanValueCoordinate2D(double *aW,
                           double px, double py,
                           const double *aXY, unsigned int nXY);

void MeanValueCoordinate(std::vector<double>& aW,
                         CVector2& p,
                         std::vector<CVector2>& aVtx);

void makeRandomLoop(unsigned int nCV,
                    std::vector<double>& aCV);

void makeSplineLoop(const std::vector<double>& aCV,
                    std::vector<double>& aVecCurve);

void FixLoopOrientation(std::vector<int>& loopIP,
                        const std::vector<int>& loopIP_ind,
                        const std::vector<CVector2>& aXY);

std::vector<CVector2> Polygon_Invert(const std::vector<CVector2>& aP);

std::vector<double> XY_Polygon(const std::vector<CVector2>& aP);

void ResamplingLoop(std::vector<int>& loopIP1_ind,
                    std::vector<int>& loopIP1,
                    std::vector<CVector2>& aXY,
                    double max_edge_length);

void JArray_FromVecVec_XY(std::vector<int>& aIndXYs,
                          std::vector<int>& loopIP0,
                          std::vector<CVector2>& aXY,
                          const std::vector< std::vector<double> >& aaXY);

void MakeMassMatrixTri(double M[9],
                       double rho,
                       const unsigned int aIP[3],
                       const std::vector<CVector2>& aVec2);
  
bool IsInclude_Loop(const double co[],
                    const int ixy_stt, const int ixy_end,
                    const std::vector<CVector2>& aXY);

bool CheckInputBoundaryForTriangulation (const std::vector<int>& loopIP_ind,
                                         const std::vector<CVector2>& aXY);
  

// -------------------------------------------------------------

/**
 * @brief 2D bounding box
 * @details inactive if x_min > x_max
 */
class CBoundingBox2D
{
public:
  CBoundingBox2D(){
    x_min=1;	x_max=-1;
    y_min=0;	y_max=0;
  }
  CBoundingBox2D(double x_min0,double x_max0,  double y_min0,double y_max0)
  : x_min(x_min0),x_max(x_max0),  y_min(y_min0),y_max(y_max0)
  {}
  CBoundingBox2D( const CBoundingBox2D& bb )
  : x_min(bb.x_min),x_max(bb.x_max), y_min(bb.y_min),y_max(bb.y_max) {}
  
  // -------------------------
  // const functions from here
  
  bool isActive() const { return x_min <= x_max; }
  bool IsIntersectSphere(const CVector2& vec, const double radius ) const
  {
    if( !isActive() ) return false;
    if( vec.p[0] < x_min-radius || vec.p[0] > x_max+radius ||
       vec.p[1] < y_min-radius || vec.p[1] > y_max+radius ) return false;
    return true;
  }
  bool IsIntersect(const CBoundingBox2D& bb_j, double clearance) const
  {
    if( bb_j.x_min > x_max+clearance || bb_j.x_max < x_min-clearance ) return false;
    if( bb_j.y_min > y_max+clearance || bb_j.y_max < y_min-clearance ) return false;
    return true;
  }
  std::vector<double> MinMaxXYZ() const {
    const double tmp[6] = {x_min,x_max, y_min,y_max, 0.0, 0.0};
    std::vector<double> bb(tmp,tmp+6);
    return bb;
  }
  
  // -------------------------------
  // non const functions from here
  
  void Add(double x0, double y0){
    if( !isActive() ){
      x_min = x_max = x0;
      y_min = y_max = y0;
      return;
    }
    x_max = ( x_max > x0 ) ? x_max : x0;
    x_min = ( x_min < x0 ) ? x_min : x0;
    y_max = ( y_max > y0 ) ? y_max : y0;
    y_min = ( y_min < y0 ) ? y_min : y0;
  }
  CBoundingBox2D& operator+=(const CBoundingBox2D& bb)
  {
    if( !bb.isActive() ){ return *this; }
    if( !isActive() ){
      x_max = bb.x_max;	x_min = bb.x_min;
      y_max = bb.y_max;	y_min = bb.y_min;
      return *this;
    }
    x_max = ( x_max > bb.x_max ) ? x_max : bb.x_max;
    x_min = ( x_min < bb.x_min ) ? x_min : bb.x_min;
    y_max = ( y_max > bb.y_max ) ? y_max : bb.y_max;
    y_min = ( y_min < bb.y_min ) ? y_min : bb.y_min;
    return *this;
  }
  bool IsInside(const CVector2& vec)
  {
    if( !isActive() ) return false;
    if(   vec.p[0] >= x_min && vec.p[0] <= x_max
       && vec.p[1] >= y_min && vec.p[1] <= y_max ) return true;
    return false;
  }
public:
  double x_min,x_max,  y_min,y_max;
};
  
}

#endif // VEC_2


