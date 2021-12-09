/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef DFM2_CURVE_BSPLINE_H
#define DFM2_CURVE_BSPLINE_H

#include <cassert>
#include <cstdio>
#include <vector>
#include <algorithm>

namespace delfem2 {

void FlatKnot(
  std::vector<double> &aKnotFlat,
  const std::vector<int> &aKnotMulti,
  const std::vector<double> &aKnot) {
  assert(aKnot.size() == aKnotMulti.size());
  aKnotFlat.clear();
  for (size_t ik = 0; ik < aKnot.size(); ++ik) {
    for (int iik = 0; iik < aKnotMulti[ik]; ++iik) {
      aKnotFlat.push_back(aKnot[ik]);
    }
  }
}

template<typename T>
T DeBoorBSpline(
  double u,
  int ndegree,
  const std::vector<T> &aCP,
  const std::vector<double> &aKnot) {
  assert(ndegree > 0);
  assert(aKnot.size() == aCP.size() + ndegree + 1);
  const double eps = 1.0e-10;
  //
  unsigned int iks;
  {
    for (iks = ndegree; iks < aKnot.size() - ndegree; ++iks) {
      double u0 = aKnot[iks];
      double u1 = aKnot[iks + 1];
      if (u >= u0 - eps && u <= u1 + eps) { break; }
    }
  }
  std::vector<T> aTmp;
  {
    aTmp.reserve(ndegree + 1);
    for (unsigned int ik = iks - ndegree; ik < iks + 1; ++ik) {
      assert(ik >= 0);
      aTmp.push_back(aCP[ik]);
    }
  }
  for (int r = 0; r < ndegree; ++r) {
    for (int j = 0; j < ndegree - r; ++j) {
      double u0 = aKnot[j + iks - ndegree + 1 + r];
      double u1 = aKnot[j + iks + 1];
//      assert(u>=u0-eps && u<u1+eps);
      double a = (u - u0) / (u1 - u0);
      aTmp[j] = (1 - a) * aTmp[j] + a * aTmp[j + 1];
    }
  }
  return aTmp[0];
}

template<typename T>
void SampleBSpline(
  std::vector<T> &polyline0,
  const int nsmpl,
  const int ndegree,
  const std::vector<double> &aKnotFlat,
  const std::vector<T> &aCtrlPoint) {
  polyline0.clear();
  double u0 = aKnotFlat[0];
  double u1 = aKnotFlat[aKnotFlat.size() - 1];
  for (int i = 0; i < nsmpl + 1; ++i) {
    double u = (double) i * (u1 - u0) / nsmpl + u0;
    T p = DeBoorBSpline(u, ndegree, aCtrlPoint, aKnotFlat);
    polyline0.push_back(p);
  }
}

/**
 * Quadratic B-Spline with "open and uniform knot vector"
 * knot vector = [0,0,0,1,2,3,...,N-1,N,N,N] / N where N is poly.size()-2
 * @param t parameter of curve that takes [0,1]
 * @param poly position of the the control points
 * @return sampled point
 */
template<typename VEC>
VEC Sample_QuadraticBsplineCurve(
  double t,
  const std::vector<VEC> &poly) {

  const auto safe_divide = [](double a, int b) {
    return (b == 0) ? 0. : a / static_cast<double>(b);
  };

  const int N = poly.size() - 2;
  t = (double) N * t;
  const int i = (t == N) ? (int) t + 1 : (int) t + 2;
  assert(i - 2 >= 0 && i < poly.size());

  const int a = std::clamp<int>(i-3,0,N);
  const int b = std::clamp<int>(i-2,0,N);
  const int c = std::clamp<int>(i-1,0,N);
  const int d = std::clamp<int>(i,0,N);

  const double w0 = safe_divide((t - b) * (t - b), (d - b) * (c - b));
  const double w1 = safe_divide((t - a) * (c - t), (c - a) * (c - b));
  const double w2 = safe_divide((d - t) * (t - b), (d - b) * (c - b));
  const double w3 = safe_divide((c - t) * (c - t), (c - a) * (c - b));
  assert(fabs(w0 + w1 + w2 + w3 - 1.) < 1.0e-10);
  assert(w0 >= 0 && w1 >= 0 && w2 >= 0 && w3 >= 0);
  return poly[i] * w0 + poly[i - 1] * (w1 + w2) + poly[i - 2] * w3;
}

}

#endif /* DFM2_CURVE_BSPLINE_H */