/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef DFM2_VIEWER_GLFW_H
#define DFM2_VIEWER_GLFW_H

#include <stdio.h>
#include <iostream>

#include "delfem2/opengl/glfw/cam_glfw.h" // for CNav3D_GLFW
#include "delfem2/dfm2_inline.h"

// ------------------------------------------------------

namespace delfem2{
namespace opengl{

class CViewer_GLFW{
public:
  void Init_oldGL();
  void DrawBegin_oldGL();
  void DrawEnd_oldGL();
  
  void Init_newGL();
  
  virtual void mouse_press(const float src[3], const float dir[3]) {} // for function override. Do nothing here
  virtual void mouse_drag(const float src0[3], const float src1[3], const float dir[3]) {} // for function override. Do nothing here
  
public:
  GLFWwindow* window;
  CNav3D_GLFW nav;
};
  
}
}

#ifdef DFM2_HEADER_ONLY
# include "delfem2/opengl/glfw/viewer_glfw.cpp"
#endif

#endif /* glfw_viewer_hpp */
