####################################################################
# Copyright (c) 2019 Nobuyuki Umetani                              #
#                                                                  #
# This source code is licensed under the MIT license found in the  #
# LICENSE file in the root directory of this source tree.          #
####################################################################


import sys
sys.path.append("../module_py")
import delfem2 as dfm2
import delfem2.glfw
import delfem2.cadmshsim

def mesh():
  cmf = dfm2.CadMesh2D(edge_length=0.1)
  cmf.add_polygon(list_xy=[-1,-1, +1,-1, +1,+1, -1,+1])
  dfm2.glfw.winDraw3d([cmf])

def poisson():
  cmf = dfm2.cadmshsim.CadMesh2D_FEMPoisson(edge_length=0.1)
  cmf.add_polygon([-1,-1, +1,-1, +1,+1, -1,+1])
  dfm2.glfw.winDraw3d([cmf])

def solid_linear_static():
  cmf = dfm2.cadmshsim.CadMesh2D_FEMSolidLinearStatic(edge_length=0.1)
  cmf.fem.param_gravity_y = -0.1
  cmf.add_polygon([-1,-1, +1,-1, +1,+1, -1,+1])
  dfm2.glfw.winDraw3d([cmf])

def solid_linear_eigen():
  cmf = dfm2.cadmshsim.CadMesh2D_FEMSolidLinearEigen(edge_length=0.1)
  cmf.add_polygon([-1,-0.2, +1,-0.2, +1,+0.2, -1,+0.2])
  dfm2.glfw.winDraw3d([cmf])

def pbd2d():
  cmf = dfm2.cadmshsim.CadMesh2D_PBD(edge_length=0.2)
  cmf.add_polygon([-1,-1, +1,-1, +1,+1, -1,+1])
  dfm2.glfw.winDraw3d([cmf])

def pbd_cloth():
  cmf = dfm2.cadmshsim.CadMesh2D_PBDCloth(edge_length=0.2)
  cmf.add_polygon([-1,-1, +1,-1, +1,+1, +0.8,+1, -0.8,+1, -1,+1])
  cmf.pbd.param_gravity_y = -0.5
  dfm2.glfw.winDraw3d([cmf])

if __name__ == "__main__":
  pbd_cloth()
#  exit()

  mesh()
  poisson()
  solid_linear_static()
  solid_linear_eigen()
  pbd2d()