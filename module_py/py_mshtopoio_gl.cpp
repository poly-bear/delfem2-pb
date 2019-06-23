#include <vector>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "delfem2/mshtopoio_gl.h"
#include "delfem2/funcs_gl.h"
#include "delfem2/dyntri.h"
#include "delfem2/dyntri_v2.h"
#include "delfem2/dyntri_v3.h"

namespace py = pybind11;

std::tuple<py::array_t<double>,py::array_t<int>> PyMeshQuad2D_Grid
(int mx, int my)
{
  std::vector<double> aXY;
  std::vector<int> aQuad;
  MeshQuad2D_Grid(aXY, aQuad,
                  mx-1, my-1);
  py::array_t<double> npXY({(int)aXY.size()/2,2}, aXY.data());
  py::array_t<int> npQuad({(int)aQuad.size()/4,4}, aQuad.data());
  return std::forward_as_tuple(npXY,npQuad);
}

void PySetTopology_ExtrudeTri2Tet
(py::array_t<int>& npTet,
 int nlayer, int nXY,
 const py::array_t<int>& npTri)
{
  SetTopology_ExtrudeTri2Tet((int*)(npTet.request().ptr),
                             nXY,
                             npTri.data(), npTri.shape()[0],
                             nlayer);
}

/////////////////////////////////////////////////////////////////////

std::tuple<py::array_t<double>,py::array_t<int>> PyMeshTri3D_ReadPly
(const std::string& fname)
{
  std::vector<double> aXYZ;
  std::vector<int> aTri;
  Read_Ply(fname, aXYZ, aTri);
  py::array_t<double> np_XYZ({(int)aXYZ.size()/3,3}, aXYZ.data());
  py::array_t<int> np_Tri({(int)aTri.size()/3,3}, aTri.data());
  return std::forward_as_tuple(np_XYZ,np_Tri);
}

std::tuple<py::array_t<double>,py::array_t<int>> PyMeshTri3D_ReadObj
(const std::string& fname)
{
  std::vector<double> aXYZ;
  std::vector<int> aTri;
  Read_Obj(fname, aXYZ, aTri);
  py::array_t<double> npXYZ({(int)aXYZ.size()/3,3}, aXYZ.data());
  py::array_t<int> npTri({(int)aTri.size()/3,3}, aTri.data());
  return std::forward_as_tuple(npXYZ,npTri);
}

void PyMeshTri3D_WriteObj
(const std::string& fname,
 const py::array_t<double>& aXYZ,
 const py::array_t<int>& aTri)
{
  Write_Obj(fname,
            aXYZ.data(), aXYZ.shape()[0],
            aTri.data(), aTri.shape()[0]);
}

std::tuple<py::array_t<double>,py::array_t<int>> PyMeshTri3D_ReadNastran
(const std::string& fname)
{
  std::vector<double> aXYZ;
  std::vector<int> aTri;
  Read_MeshTri3D_Nas(aXYZ, aTri, fname.c_str());
  py::array_t<double> npXYZ({(int)aXYZ.size()/3,3}, aXYZ.data());
  py::array_t<int> npTri({(int)aTri.size()/3,3}, aTri.data());
  return std::forward_as_tuple(npXYZ,npTri);
}

std::tuple<py::array_t<double>,py::array_t<int>> PyMeshQuad3D_Subviv
(const py::array_t<double>& aXYZ0, const py::array_t<int>& aQuad0)
{
  std::vector<int> aQuad1;
  std::vector<int> psupIndQuad0, psupQuad0;
  std::vector<int> aEdgeFace0;
  QuadSubdiv(aQuad1,
             psupIndQuad0,psupQuad0, aEdgeFace0,
             aQuad0.data(), aQuad0.shape()[0], aXYZ0.shape()[0]);
  ///////
  std::vector<double> aXYZ1;
  SubdivisionPoints_QuadCatmullClark(aXYZ1,
                                     aQuad1,aEdgeFace0,psupIndQuad0,psupQuad0,
                                     aQuad0.data(),aQuad0.shape()[0],
                                     aXYZ0.data(), aXYZ0.shape()[0]);
  py::array_t<double> npXYZ1({(int)aXYZ1.size()/3,3}, aXYZ1.data());
  py::array_t<int> npQuad1({(int)aQuad1.size()/4,4}, aQuad1.data());
  return std::forward_as_tuple(npXYZ1,npQuad1);
}

std::tuple<py::array_t<double>,py::array_t<int>> PyMeshHex3D_Subviv
(const py::array_t<double>& aXYZ0, const py::array_t<int>& aHex0)
{
  std::vector<int> aHex1;
  std::vector<int> psupIndHex0, psupHex0;
  std::vector<int> aQuadHex0;
  HexSubdiv(aHex1,
            psupIndHex0, psupHex0,
            aQuadHex0,
            ///
            aHex0.data(), aHex0.shape()[0], aXYZ0.shape()[0]);
  ///////
  std::vector<double> aXYZ1;
  SubdivisionPoints_Hex(aXYZ1,
                        psupIndHex0,psupHex0,aQuadHex0,
                        aHex0.data(), aHex0.shape()[0],
                        aXYZ0.data(), aXYZ0.shape()[0]);
  py::array_t<double> npXYZ1({(int)aXYZ1.size()/3,3}, aXYZ1.data());
  py::array_t<int> npHex1({(int)aHex1.size()/8,8}, aHex1.data());
  return std::forward_as_tuple(npXYZ1,npHex1);
}


///////////////////////////////////////////////////////////////////////////////////

void PyMeshDynTri3D_Initialize
(CMeshDynTri3D& mesh,
 const py::array_t<double>& po,
 const py::array_t<int>& tri)
{
  mesh.Initialize(po.data(), po.shape()[0], po.shape()[1],
                  tri.data(), tri.shape()[0]);
}

void PyMeshDynTri2D_Initialize
(CMeshDynTri2D& mesh,
 const py::array_t<double>& po,
 const py::array_t<int>& tri)
{
  assert(po.shape()[1]==2);
  mesh.Initialize(po.data(), po.shape()[0],
                  tri.data(), tri.shape()[0]);
}

void PySetXY_MeshDynTri2D
(CMeshDynTri2D& mesh,
 const py::array_t<double>& npXY)
{
  assert(npXY.shape()[1]==2);
  mesh.setXY(npXY.data(), npXY.shape()[0]);
}

void PyCopyMeshDynTri2D
(py::array_t<double>& npPos,
 py::array_t<int>& npElm,
 const CMeshDynTri2D& mesh)
{
  assert(npPos.ndim()==2);
  assert(npElm.ndim()==2);
  assert(npPos.shape()[1]==2);
  assert(npElm.shape()[1]==3);
  const int np = mesh.aEPo.size();
  const int ntri = mesh.aETri.size();
  assert(npPos.shape()[0]==np);
  assert(npElm.shape()[0]==ntri);
  {
    double* pP = (double*)(npPos.request().ptr);
    for(int ip=0;ip<np;++ip){
      pP[ip*2+0] = mesh.aVec2[ip].x;
      pP[ip*2+1] = mesh.aVec2[ip].y;
    }
  }
  {
    int* pT = (int*)(npElm.request().ptr);
    for(int it=0;it<ntri;++it){
      pT[it*3+0] = mesh.aETri[it].v[0];
      pT[it*3+1] = mesh.aETri[it].v[1];
      pT[it*3+2] = mesh.aETri[it].v[2];
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////

void PyDrawMesh_FaceNorm
(const py::array_t<double>& pos,
 const py::array_t<int>& elm,
 const MESHELEM_TYPE type)
{
  assert(pos.ndim()==2);
  assert(elm.ndim()==2);
  const auto shape_pos = pos.shape();
  const auto shape_elm = elm.shape();
  if( shape_pos[1] == 3 ){ // 3D Mesh
    if( type == MESHELEM_TRI  ){  DrawMeshTri3D_FaceNorm( pos.data(), elm.data(), shape_elm[0]); }
    if( type == MESHELEM_QUAD ){  DrawMeshQuad3D_FaceNorm(pos.data(), elm.data(), shape_elm[0]); }
    if( type == MESHELEM_HEX  ){  DrawMeshHex3D_FaceNorm( pos.data(), elm.data(), shape_elm[0]); }
    if( type == MESHELEM_TET  ){  DrawMeshTet3D_FaceNorm( pos.data(), elm.data(), shape_elm[0]); }
  }
}

void PyDrawMesh_Edge
(const py::array_t<double>& pos,
 const py::array_t<int>& elm,
 const MESHELEM_TYPE type)
{
  assert(pos.ndim()==2);
  assert(elm.ndim()==2);
  const auto shape_pos = pos.shape();
  const auto shape_elm = elm.shape();
  if( shape_pos[1] == 3 ){ // 3D Mesh
    if( type == MESHELEM_TRI  ){  DrawMeshTri3D_Edge( pos.data(), shape_pos[0], elm.data(), shape_elm[0]); }
    if( type == MESHELEM_QUAD ){  DrawMeshQuad3D_Edge(pos.data(), shape_pos[0], elm.data(), shape_elm[0]); }
    if( type == MESHELEM_HEX  ){  DrawMeshHex3D_Edge( pos.data(), shape_pos[0], elm.data(), shape_elm[0]); }
    if( type == MESHELEM_TET  ){  DrawMeshTet3D_Edge( pos.data(), shape_pos[0], elm.data(), shape_elm[0]); }
  }
  if( shape_pos[1] == 2 ){ // 2D Mesh
    if( type == MESHELEM_TRI  ){  DrawMeshTri2D_Edge( pos.data(), shape_pos[0], elm.data(), shape_elm[0]); }
    if( type == MESHELEM_QUAD ){  DrawMeshQuad2D_Edge(pos.data(), shape_pos[0], elm.data(), shape_elm[0]); }
  }
}


/*
std::tuple<std::vector<double>,std::vector<int>>
PyTriangulation
(const std::vector< std::vector<double> >& aaXY,
 double edge_length)
{
  std::vector<int> loopIP_ind,loopIP;
  std::vector<CVector2> aVec2;
  {
    JArray_FromVecVec_XY(loopIP_ind,loopIP, aVec2,
                         aaXY);
    if( !CheckInputBoundaryForTriangulation(loopIP_ind,aVec2) ){
      std::vector<double> aXY;
      std::vector<int> aTri;
      return std::forward_as_tuple(aXY,aTri);
    }
    FixLoopOrientation(loopIP,
                       loopIP_ind,aVec2);
    if( edge_length > 10e-10 ){
      ResamplingLoop(loopIP_ind,loopIP,aVec2,
                     edge_length );
    }
  }
  std::vector<CEPo2> aPo2D;
  std::vector<ETri> aETri;
  Meshing_SingleConnectedShape2D(aPo2D, aVec2, aETri,
                                 loopIP_ind, loopIP);
  if( edge_length > 1.0e-10 ){
    CInputTriangulation_Uniform param(1.0);
    std::vector<int> aFlgPnt(aVec2.size(),0);  // TODO: make this flag
    std::vector<int> aFlgTri(aETri.size(),0);
    MeshingInside(aPo2D,aETri,aVec2, aFlgPnt,aFlgTri,
                  aVec2.size(), 0, edge_length, param);
  }
  std::vector<double> aXY;
  std::vector<int> aTri;
  MeshTri2D_Export(aXY,aTri, aVec2,aETri);
  return std::forward_as_tuple(aXY,aTri);
}
 */

std::tuple<py::array_t<int>, py::array_t<int>>
PyJArray_MeshPsup(const py::array_t<int>& elm, int npoint)
{
  std::vector<int> psup_ind, psup;
  JArray_MeshOneRingNeighborhood(psup_ind, psup,
                                      elm.data(), elm.shape()[0], elm.shape()[1], npoint);
  py::array_t<int> np_psup_ind((pybind11::size_t)psup_ind.size(), psup_ind.data());
  py::array_t<int> np_psup((pybind11::size_t)psup.size(), psup.data());
  return std::forward_as_tuple(np_psup_ind, np_psup);
}

void PyJArray_Sort
(py::array_t<int>& psup_ind,
 py::array_t<int>& psup)
{
  //  std::cout << "hoge " << psup_ind.size() << " " << psup.size() << std::endl;
  auto buff_psup = psup.request();
  JArray_Sort(psup_ind.data(), psup_ind.shape()[0]-1, (int*)buff_psup.ptr);
}

std::tuple<py::array_t<int>, py::array_t<int>>
PyJArray_AddDiagonal(py::array_t<int>& psup_ind0, py::array_t<int>& psup0)
{
  std::vector<int> psup_ind, psup;
  JArray_AddDiagonal(psup_ind,psup,
                          psup_ind0.data(),psup_ind0.shape()[0], psup0.data(),psup0.shape()[0]);
  py::array_t<int> np_psup_ind((pybind11::size_t)psup_ind.size(), psup_ind.data());
  py::array_t<int> np_psup((pybind11::size_t)psup.size(), psup.data());
  return std::forward_as_tuple(np_psup_ind, np_psup);
}


void PyMassLumped
(py::array_t<double>& mass_lumped,
 double rho,
 const py::array_t<double>& np_pos,
 const py::array_t<int>& np_elm,
 MESHELEM_TYPE elem_type)
{
  assert( mass_lumped.ndim() == 1 );
  assert( np_pos.ndim() == 2 );
  assert( np_elm.ndim() == 2 );
  assert( mass_lumped.shape()[0] == np_pos.shape()[0] );
  if( elem_type ==  MESHELEM_TET ){
    std::cout << "mass_lumped " << np_pos.shape()[0] << " " << np_elm.shape()[0] << std::endl;
    MassLumped_Tet3D((double*)(mass_lumped.request().ptr),
                     rho,
                     np_pos.data(), np_pos.shape()[0],
                     np_elm.data(), np_elm.shape()[0]);
  }
}



py::array_t<int> PyElemQuad_DihedralTri
(py::array_t<int>& aTri,
 int np)
{
  assert( aTri.shape()[1] == 3 );
  const int nTri = aTri.shape()[0];
  std::vector<int> aQuad;
  ElemQuad_DihedralTri(aQuad, aTri.data(), nTri, np);
  py::array_t<int> npQuad({(int)aQuad.size()/4,4}, aQuad.data());
  return npQuad;
}

std::tuple<double,double>
PyQuality_MeshTri2D
(const py::array_t<double>& np_xy,
 const py::array_t<int>& np_tri)
{
  double max_aspect;
  double min_area;
  Quality_MeshTri2D(max_aspect, min_area,
                    np_xy.data(),
                    np_tri.data(), np_tri.shape()[0]);
   return std::tie(max_aspect, min_area);
}


void PyMapValue
(py::array_t<double>& npV,
 CCmdRefineMesh& mpr)
{
  /*
  assert(npOut.shape()[0]==(int)mpr.iv_ind.size()-1);
  assert(npIn.shape()[0]==(int)mpr.nv_in);
  assert(npIn.shape()[1]==npIn.shape()[1]);
  const int ndimval = npIn.shape()[1];
  const int np1 = npOut.shape()[0];
  double* pV = (double*)(npOut.request().ptr);
  for(int i=0;i<np1*ndimval;++i){ pV[i] = 0.0; }
  for(int ip=0;ip<np1;++ip){
    for(int iip=mpr.iv_ind[ip];iip<mpr.iv_ind[ip+1];++iip){
      int jp = mpr.iv[iip];
      double w0 = mpr.w[iip];
      for(int idim=0;idim<ndimval;++idim){
        pV[ip*ndimval+idim] += w0*npIn.at(jp,idim);
      }
    }
  }
   */
  assert( npV.ndim() == 2 );
  const int np = npV.shape()[0];
  const int ndim = npV.shape()[1];
  double* pV = (double*)(npV.request().ptr);
  mpr.Interpolate(pV, np, ndim);
}


///////////////////////////////////////////////////////////////////////////////////////

void init_mshtopoio_gl(py::module &m){
  py::enum_<MESHELEM_TYPE>(m, "MESH_ELEM_TYPE")
  .value("TRI",     MESHELEM_TYPE::MESHELEM_TRI)
  .value("QUAD",    MESHELEM_TYPE::MESHELEM_QUAD)
  .value("TET",     MESHELEM_TYPE::MESHELEM_TET)
  .value("PYRAMID", MESHELEM_TYPE::MESHELEM_PYRAMID)
  .value("WEDGE",   MESHELEM_TYPE::MESHELEM_WEDGE)
  .value("HEX",     MESHELEM_TYPE::MESHELEM_HEX)
  .export_values();
  
  py::class_<CMeshMultiElem>(m,"MeshMultiElem")
  .def(py::init<>())
  .def("read_obj", &CMeshMultiElem::ReadObj)
  .def("minmax_xyz", &CMeshMultiElem::AABB3_MinMax)
  .def("draw",&CMeshMultiElem::Draw)
  .def("scaleXYZ",&CMeshMultiElem::ScaleXYZ)
  .def("translateXYZ",&CMeshMultiElem::TranslateXYZ);
  
  py::class_<CMeshDynTri3D>(m, "CppMeshDynTri3D")
  .def(py::init<>())
  .def("draw",              &CMeshDynTri3D::draw)
  .def("draw_face",         &CMeshDynTri3D::Draw_FaceNorm)
  .def("draw_edge",         &CMeshDynTri3D::Draw_Edge)
  .def("check",             &CMeshDynTri3D::Check)
  .def("ntri",              &CMeshDynTri3D::nTri)
  .def("delete_tri_edge",   &CMeshDynTri3D::DeleteTriEdge)
  .def("minmax_xyz",        &CMeshDynTri3D::MinMax_XYZ)
  .def("insert_point_elem", &CMeshDynTri3D::insertPointElem)
  .def("delaunay_around_point", &CMeshDynTri3D::DelaunayAroundPoint);
  
  
  py::class_<CCmdRefineMesh>(m, "CppMapper")
  .def(py::init<>());
  
  m.def("map_value",    &PyMapValue);
  
  py::class_<CMeshDynTri2D>(m, "CppMeshDynTri2D")
  .def(py::init<>())
  .def("draw",              &CMeshDynTri2D::draw)
  .def("draw_face",         &CMeshDynTri2D::Draw_FaceNorm)
  .def("draw_edge",         &CMeshDynTri2D::Draw_Edge)
  .def("check",             &CMeshDynTri2D::Check)
  .def("ntri",              &CMeshDynTri2D::nTri)
  .def("npoint",            &CMeshDynTri2D::nPoint)
  .def("delete_tri_edge",   &CMeshDynTri2D::DeleteTriEdge)
  .def("minmax_xyz",        &CMeshDynTri2D::MinMax_XYZ)
  .def("insert_point_elem", &CMeshDynTri2D::insertPointElem)
  .def("delaunay_around_point", &CMeshDynTri2D::DelaunayAroundPoint)
  .def("meshing_loops",     &CMeshDynTri2D::meshing_loops)
  .def("refinementPlan_EdgeLongerThan_InsideCircle",   &CMeshDynTri2D::RefinementPlan_EdgeLongerThan_InsideCircle);
  
  
  m.def("meshdyntri3d_initialize",&PyMeshDynTri3D_Initialize);
  m.def("meshdyntri2d_initialize",&PyMeshDynTri2D_Initialize);
  m.def("copyMeshDynTri2D",       &PyCopyMeshDynTri2D);
  m.def("setXY_MeshDynTri2D",     &PySetXY_MeshDynTri2D);
  
  m.def("meshtri3d_read_ply",     &PyMeshTri3D_ReadPly,     py::return_value_policy::move);
  m.def("meshtri3d_read_obj",     &PyMeshTri3D_ReadObj,     py::return_value_policy::move);
  m.def("meshtri3d_read_nastran", &PyMeshTri3D_ReadNastran, py::return_value_policy::move);
  m.def("meshtri3d_write_obj",    &PyMeshTri3D_WriteObj);
  m.def("meshquad3d_subdiv",      &PyMeshQuad3D_Subviv,     py::return_value_policy::move);
  m.def("meshhex3d_subdiv",       &PyMeshHex3D_Subviv,      py::return_value_policy::move);
  m.def("meshquad2d_grid",        &PyMeshQuad2D_Grid,       py::return_value_policy::move);
  
  m.def("setTopology_ExtrudeTri2Tet", &PySetTopology_ExtrudeTri2Tet);
  
  m.def("mass_lumped", &PyMassLumped);
  
  
  /*
  m.def("triangulation",&PyTriangulation,
        py::arg("aXY"),
        py::arg("edge_length")=0.03);
   */
  
  m.def("jarray_mesh_psup",    &PyJArray_MeshPsup,    py::return_value_policy::move);
  m.def("jarray_add_diagonal", &PyJArray_AddDiagonal, py::return_value_policy::move);
  m.def("jarray_sort",         &PyJArray_Sort);
  
  m.def("elemQuad_dihedralTri",&PyElemQuad_DihedralTri);
  m.def("quality_meshTri2D",   &PyQuality_MeshTri2D);
  
  m.def("draw_mesh_facenorm",  &PyDrawMesh_FaceNorm);
  m.def("draw_mesh_edge",      &PyDrawMesh_Edge);
}
