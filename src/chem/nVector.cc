/*
    File: nVector.cc
*/
/*
Open Source License
Copyright (c) 2016, Christian E. Schafmeister
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 
This is an open source license for the CANDO software from Temple University, but it is not the only one. Contact Temple University at mailto:techtransfer@temple.edu if you would like a different license.
*/
/* -^- */
       
#define	DEBUG_LEVEL_NONE

//
// (C) 2004 Christian E. Schafmeister
//

#include <clasp/core/foundation.h>
#include <clasp/core/object.h>
#include <clasp/core/array.h>
#include <cando/geom/matrix.h>
#include <cando/geom/vector3.h>
#include <cando/geom/boundingBox.h>
#include <cando/chem/nVector.h>
#include <cando/chem/aggregate.h>

namespace chem {
/*
 *      dotProduct
 *
 *	Author:	Christian Schafmeister (1991)
 *
 *      Return the DOT product of two vectors.
 *      return:  (this) . x
 */
double	dotProduct( NVector_sp x, NVector_sp y )
{
  double          dDot;
  ASSERT(x->length()==y->length());
  dDot = 0.0;
  double* px = &(*x)[0];
  double* py = &(*y)[0];
  for ( size_t i(0), iEnd(x->length()); i<iEnd; ++i ) dDot += px[i]*py[i];
  return dDot;
}


double	squared(NVector_sp x)
{
  double          dDot;
  dDot = 0.0;
  double* dp = &(*x)[0];
  for ( size_t i(0), iEnd(x->length()); i<iEnd; ++i ) {
    dDot += dp[i]*dp[i];
  }
  return(dDot);
}

double magnitude(NVector_sp me)
{
  double  dDot;
  dDot = squared(me);
  return(sqrt(dDot));
}


double	rmsMagnitude(NVector_sp me)
{
  ASSERT(me->length()>0);
  double  dDot;
  dDot = squared(me);
  dDot /= (double)me->length();
  return(sqrt(dDot));
}


double	angleWithVector(NVector_sp me, NVector_sp other)
{
#define	VERY_SMALL 1.0e-6
  double	lenThis, lenOther, dot;
  lenThis = magnitude(me);
  lenOther = magnitude(other);
  dot = dotProduct(me->asSmartPtr(),other);
  if ( fabs(lenThis) > VERY_SMALL && fabs(lenOther)>VERY_SMALL) {
    dot /= (lenThis*lenOther);
    if ( dot > 1.0 ) dot = 1.0;
    if ( dot < -1.0 ) dot = -1.0;
    return acos(dot);
  }
  return 0.0;
}

void XPlusYTimesScalar( NVector_sp o, NVector_sp x, NVector_sp y, double s )
{
  LOG(BF("this->_Values.size() = %d") % o->length() );
  LOG(BF("x->size() = %d") % x->length() );
  LOG(BF("y->size() = %d") % y->length() );
  ASSERTP( o->length() == x->length(),"NVector_O::addTimesScalar>>mismatched length");
  ASSERT( o->length() == y->length());
  double* po = &(*o)[0];
  double* px = &(*x)[0];
  double* py = &(*y)[0];
  for ( int i(0),iEnd(o->length()); i<iEnd; ++i ) {
    po[i] = px[i]+py[i]*s;
  }
}

/*
 *      copyVector
 *
 *	Author:	Christian Schafmeister (1991)
 *
 *      Copy the contents of nvVector into another.
 */
void	copyVector(NVector_sp dest, NVector_sp orig)
{
  ASSERTP(dest->length()==orig->length(),"NVector_O::copy>>mismatched length");
  double* ddest = &(*dest)[0];
  double* dorig = &(*orig)[0];
  for ( size_t i(0),iEnd(orig->length()); i<iEnd; ++i ) {
    ddest[i] = dorig[i];
  }
}

void inPlaceAddTimesScalar( NVector_sp result, NVector_sp dir, double s )
{_OF();
  ASSERT( result->length() == dir->length());
  double* presult = &(*result)[0];
  double* pdir = &(*dir)[0];
  for ( size_t i(0), iEnd(result->length()); i<iEnd; ++i ) {
    presult[i] += pdir[i]*s;
  }
}


double	rmsDistanceFrom(NVector_sp u, NVector_sp v)
{_OF();
  ASSERT(u->length() == v->length());
  double	e, sum;
  sum = 0.0;
  double* du = &(*u)[0];
  double* dv = &(*v)[0];
  for ( size_t i(0), iEnd(u->length()); i<iEnd; ++i ) {
    e = du[i]-dv[i];
    sum += e*e;
  }
  sum /= (float)(u->length());
  return sqrt(sum);
}


NVector_sp copy_nvector(NVector_sp orig )
{
//  GC_COPY(NVector_O,vv,*orig);
  NVector_sp vv = NVector_O::make(orig->length(),0.0,false,orig->length(),(double*)&(*orig)[0]);
  return vv;
}


CL_DEFUN
NVector_sp chem__make_nvector(size_t size)
{
  return NVector_O::make(size,0.0,true);
}

CL_DEFUN
NVector_sp chem__copy_nvector(NVector_sp source)
{
  return copy_nvector(source);
}



CL_DOCSTRING("Apply the transform matrix to the coordinates.");
CL_DEFUN
NVector_sp chem__apply_transform_to_coordinates(NVector_sp destination, NVector_sp coordinates, const Matrix& transform ) __attribute__((optnone))
{
  Vector3* pos;
  Vector3* dest;
  size_t len = coordinates->length();
  for ( size_t index = 0; index < len; index+=3) {
//    printf("%s:%d  coordinates[%lu/%lu] -> %lf %lf %lf\n", __FILE__, __LINE__, index, len, (*coordinates)[index],(*coordinates)[index+1],(*coordinates)[index+2]);
    transform.transform_nvector_point((*destination)[index],
                                      (*destination)[index+1],
                                      (*destination)[index+2],
                                      coordinates,index);
//    printf("%s:%d  destination[%lu/%lu] -> %lf %lf %lf\n", __FILE__, __LINE__, index, len, (*destination)[index],(*destination)[index+1],(*destination)[index+2]);
  }
  return destination;
};


inline double dist_squared(const Vector3& pos1, const Vector3& pos2)
{
  double dx = pos1.getX()-pos2.getX();
  double dy = pos1.getY()-pos2.getY();
  double dz = pos1.getZ()-pos2.getZ();
  return dx*dx+dy*dy+dz*dz;
}

inline double dist_squared_bounding_box(NVector_sp coord1, size_t i1, NVector_sp coord2, size_t i2, double x_size, double y_size, double z_size, double x_rsize, double y_rsize, double z_rsize )
{
  
  double dx = fabs((*coord1)[i1]-(*coord2)[i2]);
  dx -= static_cast<int>(dx*x_rsize+0.5)*x_size;
  double dy = fabs((*coord1)[i1+1]-(*coord2)[i2+1]);
  dy -= static_cast<int>(dy*y_rsize+0.5)*y_size;
  double dz = fabs((*coord1)[i1+2]-(*coord2)[i2+2]);
  dz -= static_cast<int>(dz*z_rsize+0.5)*z_size;
  return dx*dx+dy*dy+dz*dz;
}
  
CL_DOCSTRING("Determine close contact. Return a pair of integers indexing into the coordinates");
CL_DEFUN
core::T_mv chem__find_close_contact(NVector_sp coord1, NVector_sp coord2)
{
  Vector3* pos1 = (Vector3*)&(*coord1)[0];
  Vector3* pos2 = (Vector3*)&(*coord2)[0];
  size_t min_i1 = 0;
  size_t min_i2 = 0;
  double min_dist_squared = dist_squared(*pos1,*pos2);
  for ( size_t i1=0; i1<coord1->length(); i1+=3 ) {
    pos1 = (Vector3*)&(*coord1)[i1];
    for ( size_t i2=0; i2<coord2->length(); i2+=3 ) {
      pos2 = (Vector3*)&(*coord2)[i2];
      double d_squared = dist_squared(*pos1,*pos2);
      if (d_squared < min_dist_squared) {
        min_dist_squared = d_squared;
        min_i1 = i1;
        min_i2 = i2;
      }
    }
  }
  return Values(core::make_fixnum(min_i1), core::make_fixnum(min_i2), core::clasp_make_double_float(sqrt(min_dist_squared)));
};




CL_DOCSTRING("Determine close contacts within the context of a bounding box. Return a pair of integers indexing into the coordinates.  Only the coordinates less than length1 and length2 are valid. Keep a count of how many close contacts are closer than close_distance.");
CL_DEFUN
core::T_mv chem__find_close_contact_in_bounding_box(NVector_sp coord1, size_t length1, NVector_sp coord2, size_t length2, BoundingBox_sp bounding_box, double close_distance)
{
  double close_distance_squared = close_distance*close_distance;
  size_t close_counts = 0;
  double x_size = bounding_box->get_x_width();
  double y_size = bounding_box->get_y_width();
  double z_size = bounding_box->get_z_width();
  double x_rsize = 1.0 / x_size;
  double y_rsize = 1.0 / y_size;
  double z_rsize = 1.0 / z_size;
  Vector3* pos1 = (Vector3*)&(*coord1)[0];
  Vector3* pos2 = (Vector3*)&(*coord2)[0];
  size_t min_i1 = 0;
  size_t min_i2 = 0;
  double min_dist_squared = dist_squared_bounding_box(coord1,0,coord2,0,x_size,y_size,z_size,x_rsize,y_rsize,z_rsize);
  for ( size_t i1=0; i1<length1; i1+=3 ) {
    pos1 = (Vector3*)&(*coord1)[i1];
    for ( size_t i2=0; i2<length2; i2+=3 ) {
      pos2 = (Vector3*)&(*coord2)[i2];
      double d_squared = dist_squared_bounding_box(coord1,i1,coord2,i2,x_size,y_size,z_size,x_rsize,y_rsize,z_rsize);
      if (d_squared < min_dist_squared) {
        min_dist_squared = d_squared;
        min_i1 = i1;
        min_i2 = i2;
      }
      if (d_squared < close_distance_squared) {
        close_counts++;
      }
    }
  }
  return Values(core::make_fixnum(min_i1), core::make_fixnum(min_i2), core::clasp_make_double_float(sqrt(min_dist_squared)), core::make_fixnum(close_counts));
};



CL_DOCSTRING("Determine close contact. Return a pair of integers indexing into the coordinates");
CL_LAMBDA(coordinates &optional coordinates-length);
CL_DEFUN
geom::BoundingCuboid_sp chem__nvector_bounding_cuboid(NVector_sp coords, core::T_sp coordinates_length)
{
  Vector3 cur;
  geom::BoundingCuboid_sp bcuboid = geom::BoundingCuboid_O::create();
  size_t coord_len = coords->length();
  if (coordinates_length.fixnump()) {
    if (coordinates_length.unsafe_fixnum()<=coords->length())
      coord_len = coordinates_length.unsafe_fixnum();
    else {
      SIMPLE_ERROR(BF("coordinates-length %s is out of bounds - must be less than or equal to %lu")
                   % _rep_(coordinates_length) % coords->length());
    }
  } else if (coordinates_length.notnilp()) {
    SIMPLE_ERROR(BF("coordinates-length must be NIL or a fixnum - it was %s") % _rep_(coordinates_length));
  }
  for ( size_t i1=0; i1<coords->length(); i1+=3 ) {
    geom::geom__vec_extract(cur,coords,i1);
    bcuboid->expandToEncompassPoint(cur);
  }
  return bcuboid;
}




}
