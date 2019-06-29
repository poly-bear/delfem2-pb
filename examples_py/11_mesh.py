####################################################################
# Copyright (c) 2019 Nobuyuki Umetani                              #
#                                                                  #
# This source code is licensed under the MIT license found in the  #
# LICENSE file in the root directory of this source tree.          #
####################################################################

import sys
sys.path.append("..")
import pydelfem2 as dfm2
import pydelfem2.gl._glfw

import numpy, math

def height_map():
  A0 = numpy.zeros((16,32))
  for iy in range(A0.shape[0]):
    for ix in range(A0.shape[1]):
      A0[iy,ix] = 5*math.sin(ix*0.4)*math.cos(iy*0.6) + 5

  msh = dfm2.Mesh()
  msh.set_grid((A0.shape[1],A0.shape[0]))
  msh.np_pos = numpy.hstack((msh.np_pos,A0.reshape((-1,1))))
  axis = dfm2.gl.AxisXYZ(32)
  print("hight_map")
  dfm2.gl._glfw.winDraw3d([msh,axis],(400,400))

  print("edge_quad_mesh")
  msh_edge = msh.mesh_edge()
  dfm2.gl._glfw.winDraw3d([msh_edge,axis])


def edge_tri():
  msh = dfm2.Mesh()
  msh.read("../test_inputs/bunny_2k.ply")
  msh_edge = msh.mesh_edge()
  aabb = dfm2.AABB3( msh.minmax_xyz() )
  dfm2.gl._glfw.winDraw3d([msh_edge,aabb])


def edge_quad_hex():
  grid = dfm2.VoxelGrid()
  grid.add(0, 0, 0)
  grid.add(1, 0, 0)
  grid.add(1, 1, 0)
  msh = grid.mesh_hex()
  msh = msh.subdiv()
  msh_edge = msh.mesh_edge()
  dfm2.gl._glfw.winDraw3d([msh_edge])

  msh = grid.mesh_quad()
  msh = msh.subdiv()
  msh_edge = msh.mesh_edge()
  dfm2.gl._glfw.winDraw3d([msh_edge])

def primitive():
  msh = dfm2.Mesh()
  msh.set_cylinder(1.0,1.0, 16, 4)
  dfm2.gl._glfw.winDraw3d([msh])

  msh.set_sphere(1.0,16, 32)
  dfm2.gl._glfw.winDraw3d([msh])

if __name__ == "__main__":
  height_map()
  edge_tri()
  edge_quad_hex()
  primitive()