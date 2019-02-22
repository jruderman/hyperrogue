// Hyperbolic Rogue
// This file contains hyperbolic points and matrices.
// Copyright (C) 2011-2018 Zeno Rogue, see 'hyper.cpp' for details

namespace hr {

#if DIM == 3
eGeometry geometry = gBinaryTiling;
eVariation variation = eVariation::pure;
#else
eGeometry geometry;
eVariation variation;
#endif


// hyperbolic points and matrices

// basic functions and types
//===========================

#ifdef SINHCOSH
// ld sinh(ld alpha) { return (exp(alpha) - exp(-alpha)) / 2; }
// ld cosh(ld alpha) { return (exp(alpha) + exp(-alpha)) / 2; }

/* ld inverse_sinh(ld z) {
  return log(z+sqrt(1+z*z));
  }
 
double inverse_cos(double c) {
  double s = sqrt(1-c*c);
  double r = atan(s/c);
  if(r < 0) r = -r;
  return r;
  }

// ld tanh(ld x) { return sinh(x) / cosh(x); }
ld inverse_tanh(ld x) { return log((1+x)/(1-x)) / 2; } */

#endif
#ifndef M_PI
#define M_PI 3.14159265358979
#endif

ld squar(ld x) { return x*x; }

int sig(int z) { return (sphere || z<DIM)?1:-1; }

int curvature() {
  switch(cgclass) {
    case gcEuclid: return 0;
    case gcHyperbolic: return -1;
    case gcSphere: return 1;
    default: return 0;
    }
  }

ld sin_auto(ld x) {
  switch(cgclass) {
    case gcEuclid: return x;
    case gcHyperbolic: return sinh(x);
    case gcSphere: return sin(x);
    default: return x;
    }
  }

ld asin_auto(ld x) {
  switch(cgclass) {
    case gcEuclid: return x;
    case gcHyperbolic: return asinh(x);
    case gcSphere: return asin(x);
    default: return x;
    }
  }

ld asin_auto_clamp(ld x) {
  switch(cgclass) {
    case gcEuclid: return x;
    case gcHyperbolic: return asinh(x);
    case gcSphere: return x>1 ? M_PI/2 : x<-1 ? -M_PI/2 : std::isnan(x) ? 0 : asin(x);
    default: return x;
    }
  }

ld cos_auto(ld x) {
  switch(cgclass) {
    case gcEuclid: return 1;
    case gcHyperbolic: return cosh(x);
    case gcSphere: return cos(x);
    default: return 1;
    }
  }

ld tan_auto(ld x) {
  switch(cgclass) {
    case gcEuclid: return x;
    case gcHyperbolic: return tanh(x);
    case gcSphere: return tan(x);
    default: return 1;
    }
  }

ld atan_auto(ld x) {
  switch(cgclass) {
    case gcEuclid: return x;
    case gcHyperbolic: return atanh(x);
    case gcSphere: return atan(x);
    default: return 1;
    }
  }

ld atan2_auto(ld y, ld x) {
  switch(cgclass) {
    case gcEuclid: return y/x;
    case gcHyperbolic: return atanh(y/x);
    case gcSphere: return atan2(y, x);
    default: return 1;
    }
  }

// hyperbolic point:
//===================

// we represent the points on the hyperbolic plane
// by points in 3D space (Minkowski space) such that x^2+y^2-z^2 == -1, z > 0
// (this is analogous to representing a sphere with points such that x^2+y^2+z^2 == 1)

hyperpoint hpxy(ld x, ld y) { 
  return hpxyz(x,y,euclid ? 1 : sphere ? sqrt(1-x*x-y*y) : sqrt(1+x*x+y*y));
  }

hyperpoint hpxy3(ld x, ld y, ld z) { 
  return hpxyz3(x,y,z, euclid ? 1 : sphere ? sqrt(1-x*x-y*y-z*z) : sqrt(1+x*x+y*y+z*z));
  }

// center of the pseudosphere
const hyperpoint Hypc = hpxyz(0,0,0);

// origin of the hyperbolic plane
const hyperpoint C0 = hpxyz(0,0,1);

// a point (I hope this number needs no comments ;) )
const hyperpoint Cx1 = hpxyz(1,0,1.41421356237);

// this function returns approximate square of distance between two points
// (in the spherical analogy, this would be the distance in the 3D space,
// through the interior, not on the surface)
// also used to verify whether a point h1 is on the hyperbolic plane by using Hypc for h2

bool zero_d(hyperpoint h, int d) { 
  for(int i=0; i<d; i++) if(h[i]) return false;
  return true;
  }

ld intval(const hyperpoint &h1, const hyperpoint &h2) {
  ld res = 0;
  for(int i=0; i<MDIM; i++) res += squar(h1[i] - h2[i]) * sig(i);
  if(elliptic) {
    ld res2 = 0;
    for(int i=0; i<MDIM; i++) res2 += squar(h1[i] + h2[i]) * sig(i);
    return min(res, res2);
    }
  return res;
  }

ld sqhypot_d(const hyperpoint& h, int d) {
  ld sum = 0;
  for(int i=0; i<d; i++) sum += h[i]*h[i];
  return sum;
  }
  
ld hypot_d(const hyperpoint& h, int d) {
  return sqrt(sqhypot_d(h, d));
  }
  
ld zlevel(const hyperpoint &h) {
  if(euclid) return h[DIM];
  else if(sphere) return sqrt(intval(h, Hypc));
  else return (h[DIM] < 0 ? -1 : 1) * sqrt(-intval(h, Hypc));
  }

ld hypot_auto(ld x, ld y) {
  switch(cgclass) {
    case gcEuclid:
      return hypot(x, y);
    case gcHyperbolic:
      return acosh(cosh(x) * cosh(y));
    case gcSphere:
      return acos(cos(x) * cos(y));
    default:
      return hypot(x, y);
    }
  }

// move H back to the sphere/hyperboloid/plane
hyperpoint normalize(hyperpoint H) {
  ld Z = zlevel(H);
  for(int c=0; c<MDIM; c++) H[c] /= Z;
  return H;
  }

// get the center of the line segment from H1 to H2
hyperpoint mid(const hyperpoint& H1, const hyperpoint& H2) {
  using namespace hyperpoint_vec;
  return normalize(H1 + H2);
  }

// like mid, but take 3D into account
hyperpoint midz(const hyperpoint& H1, const hyperpoint& H2) {
  using namespace hyperpoint_vec;
  hyperpoint H3 = H1 + H2;
  
  ld Z = 2;
  
  if(!euclid) Z = zlevel(H3) * 2 / (zlevel(H1) + zlevel(H2));
  for(int c=0; c<MDIM; c++) H3[c] /= Z;
  
  return H3;
  }

// matrices
//==========

// matrices represent isometries of the hyperbolic plane
// (just like isometries of the sphere are represented by rotation matrices)

// rotate by alpha degrees
transmatrix cspin(int a, int b, ld alpha) {
  transmatrix T = Id;
  T[a][a] = +cos(alpha); T[a][b] = +sin(alpha);
  T[b][a] = -sin(alpha); T[b][b] = +cos(alpha);
  return T;
  }

transmatrix spin(ld alpha) { return cspin(0, 1, alpha); }

transmatrix eupush(ld x, ld y) {
  transmatrix T = Id;
  T[0][DIM] = x;
  T[1][DIM] = y;
  return T;
  }

transmatrix eupush3(ld x, ld y, ld z) {
  transmatrix T = Id;
  T[0][DIM] = x;
  T[1][DIM] = y;
  if(DIM == 3) T[2][DIM] = z;
  return T;
  }

transmatrix eupush(hyperpoint h) {
  transmatrix T = Id;
  for(int i=0; i<DIM; i++) T[i][DIM] = h[i];
  return T;
  }

transmatrix euscalezoom(hyperpoint h) {
  transmatrix T = Id;
  T[0][0] = h[0];
  T[0][1] = -h[1];
  T[1][0] = h[1];
  T[1][1] = h[0];
  return T;
  }

transmatrix euaffine(hyperpoint h) {
  transmatrix T = Id;
  T[0][1] = h[0];
  T[1][1] = exp(h[1]);
  return T;
  }

transmatrix cpush(int cid, ld alpha) {
  transmatrix T = Id;
  T[DIM][DIM] = T[cid][cid] = cos_auto(alpha);
  T[cid][DIM] = sin_auto(alpha);
  T[DIM][cid] = -curvature() * sin_auto(alpha);
  return T;
  }

// push alpha units to the right
transmatrix xpush(ld alpha) { return cpush(0, alpha); }

inline hyperpoint cpush0(int c, ld x) { 
  hyperpoint h = Hypc;
  h[DIM] = cos_auto(x);
  h[c] = sin_auto(x);
  return h;
  }

inline hyperpoint xpush0(ld x) { return cpush0(0, x); }

inline hyperpoint ypush0(ld x) { return cpush0(1, x); }

inline hyperpoint xspinpush0(ld alpha, ld x) { 
  hyperpoint h = Hypc;
  h[DIM] = cos_auto(x);
  h[0] = sin_auto(x) * cos(alpha);
  h[1] = sin_auto(x) * -sin(alpha);
  return h;
  }

// push alpha units vertically
transmatrix ypush(ld alpha) { return cpush(1, alpha); }

transmatrix matrix3(ld a, ld b, ld c, ld d, ld e, ld f, ld g, ld h, ld i) {
  #if DIM==2
  return transmatrix {{{a,b,c},{d,e,f},{g,h,i}}};
  #else
  return transmatrix {{{a,b,0,c},{d,e,0,f},{0,0,1,0},{g,h,0,i}}};
  #endif
  }

transmatrix matrix4(ld a, ld b, ld c, ld d, ld e, ld f, ld g, ld h, ld i, ld j, ld k, ld l, ld m, ld n, ld o, ld p) {
  #if DIM==2
  return transmatrix {{{a,b,d},{e,f,h},{m,n,p}}};
  #else
  return transmatrix {{{a,b,c,d},{e,f,g,h},{i,j,k,l},{m,n,o,p}}};
  #endif
  }

transmatrix parabolic1(ld u) {
  if(euclid)
    return ypush(u);
  else {
    ld diag = u*u/2;
    return matrix3(
      -diag+1, u, diag,
      -u, 1, u,
      -diag, u, diag+1
      );
    }
  }

transmatrix parabolic13(ld u, ld v) {
  if(euclid)
    return ypush(u);
  else {
    ld diag = (u*u+v*v)/2;
    return matrix4(
      -diag+1, u, v, diag,
      -u, 1, 0, u,
      -v, 0, 1, v,
      -diag, u, v, diag+1
      );
    }
  }

transmatrix spintoc(const hyperpoint& H, int t, int f) {
  transmatrix T = Id;
  ld R = hypot(H[f], H[t]);
  if(R >= 1e-12) {
    T[t][t] = +H[t]/R; T[t][f] = +H[f]/R;
    T[f][t] = -H[f]/R; T[f][f] = +H[t]/R;
    }
  return T;
  }

transmatrix rspintoc(const hyperpoint& H, int t, int f) {
  transmatrix T = Id;
  ld R = hypot(H[f], H[t]);
  if(R >= 1e-12) {
    T[t][t] = +H[t]/R; T[t][f] = -H[f]/R;
    T[f][t] = +H[f]/R; T[f][f] = +H[t]/R;
    }
  return T;
  }

// rotate the hyperbolic plane around C0 such that H[1] == 0 and H[0] >= 0
transmatrix spintox(const hyperpoint& H) { 
  #if DIM==2
  return spintoc(H, 0, 1); 
  #else
  transmatrix T1 = spintoc(H, 0, 1);
  return spintoc(T1*H, 0, 2) * T1;
  #endif
  }

void set_column(transmatrix& T, int i, const hyperpoint& H) {
  for(int j=0; j<MDIM; j++)
    T[j][i] = H[j];
  }

transmatrix build_matrix(hyperpoint h1, hyperpoint h2, hyperpoint h3) {
  transmatrix T = Id;
  for(int i=0; i<MDIM; i++)
    T[i][0] = h1[i],
    T[i][1] = h2[i],
    T[i][2] = h3[i];
  return T;
  }

// reverse of spintox(H)
transmatrix rspintox(const hyperpoint& H) { 
  #if DIM==2
  return rspintoc(H, 0, 1); 
  #else
  transmatrix T1 = spintoc(H, 0, 1);
  return rspintoc(H, 0, 1) * rspintoc(T1*H, 0, 2);
  #endif
  }

// for H such that H[1] == 0, this matrix pushes H to C0
transmatrix pushxto0(const hyperpoint& H) {
  transmatrix T = Id;
  T[0][0] = +H[DIM]; T[0][DIM] = -H[0];
  T[DIM][0] = curvature() * H[0]; T[DIM][DIM] = +H[DIM];
  return T;
  }

// reverse of pushxto0(H)
transmatrix rpushxto0(const hyperpoint& H) {
  transmatrix T = Id;
  T[0][0] = +H[DIM]; T[0][DIM] = H[0];
  T[DIM][0] = -curvature() * H[0]; T[DIM][DIM] = +H[DIM];
  return T;
  }

transmatrix ggpushxto0(const hyperpoint& H, ld co) {
  if(euclid) {
    using namespace hyperpoint_vec;
    return eupush(co * H);
    }
  transmatrix res = Id;
  if(sqhypot_d(H, DIM) < 1e-12) return res;
  ld fac = (H[DIM]-1) / sqhypot_d(H, DIM);
  for(int i=0; i<DIM; i++)
  for(int j=0; j<DIM; j++)
    res[i][j] += H[i] * H[j] * fac;
    
  for(int d=0; d<DIM; d++)
    res[d][DIM] = co * H[d],
    res[DIM][d] = -curvature() * co * H[d];
  res[DIM][DIM] = H[DIM];
  
  return res;
  }

// generalization: H[1] can be non-zero
transmatrix gpushxto0(const hyperpoint& H) {
  return ggpushxto0(H, -1);
  }

transmatrix rgpushxto0(const hyperpoint& H) {
  return ggpushxto0(H, 1);
  }

// fix the matrix T so that it is indeed an isometry
// (without using this, imprecision could accumulate)

void fixmatrix(transmatrix& T) {
  if(euclid) {
    for(int x=0; x<DIM; x++) for(int y=0; y<=x; y++) {
      ld dp = 0;
      for(int z=0; z<DIM; z++) dp += T[z][x] * T[z][y];
      
      if(y == x) dp = 1 - sqrt(1/dp);
      
      for(int z=0; z<DIM; z++) T[z][x] -= dp * T[z][y];
      }
    for(int x=0; x<DIM; x++) T[2][x] = 0;
    T[2][2] = 1;
    }
  else for(int x=0; x<MDIM; x++) for(int y=0; y<=x; y++) {
    ld dp = 0;
    for(int z=0; z<MDIM; z++) dp += T[z][x] * T[z][y] * sig(z);
    
    if(y == x) dp = 1 - sqrt(sig(x)/dp);
    
    for(int z=0; z<MDIM; z++) T[z][x] -= dp * T[z][y];
    }
  }

// show the matrix on screen

ld det(const transmatrix& T) {
  #if DIM == 2
  ld det = 0;
  for(int i=0; i<3; i++) 
    det += T[0][i] * T[1][(i+1)%3] * T[2][(i+2)%3];
  for(int i=0; i<3; i++) 
    det -= T[0][i] * T[1][(i+2)%3] * T[2][(i+1)%3];
  #else
  ld det = 1;
  transmatrix M = T;
  for(int a=0; a<MDIM; a++) {
    for(int b=a; b<=DIM; b++)
      if(M[b][a]) {
        if(b != a)
          for(int c=a; c<MDIM; c++) tie(M[b][c], M[a][c]) = make_pair(-M[a][c], M[b][c]);
        break;
        }
    if(!M[a][a]) return 0;
    for(int b=a+1; b<=DIM; b++) {
      ld co = -M[b][a] / M[a][a];
      for(int c=a; c<MDIM; c++) M[b][c] += M[a][c] * co;
      }
    det *= M[a][a];
    }
  #endif
  return det;
  }

void inverse_error(const transmatrix& T) {
  println(hlog, "Warning: inverting a singular matrix: ", T);
  }

transmatrix inverse(const transmatrix& T) {
  profile_start(7);
  
  #if DIM == 2
  ld d = det(T);
  transmatrix T2;
  if(d == 0) {
    inverse_error(T); 
    return Id;
    }
  
  for(int i=0; i<3; i++) 
  for(int j=0; j<3; j++) 
    T2[j][i] = (T[(i+1)%3][(j+1)%3] * T[(i+2)%3][(j+2)%3] - T[(i+1)%3][(j+2)%3] * T[(i+2)%3][(j+1)%3]) / d;
  
  #else
  transmatrix T1 = T;
  transmatrix T2 = Id;

  for(int a=0; a<MDIM; a++) {
    for(int b=a; b<=DIM; b++)
      if(T1[b][a]) {
        if(b != a)
          for(int c=0; c<MDIM; c++) 
            swap(T1[b][c], T1[a][c]), swap(T2[b][c], T2[a][c]);
        break;
        }
    if(!T1[a][a]) { inverse_error(T); return Id; }
    for(int b=a+1; b<=DIM; b++) {
      ld co = -T1[b][a] / T1[a][a];
      for(int c=0; c<MDIM; c++) T1[b][c] += T1[a][c] * co, T2[b][c] += T2[a][c] * co;
      }
    }
  
  for(int a=MDIM-1; a>=0; a--) {
    for(int b=0; b<a; b++) {
      ld co = -T1[b][a] / T1[a][a];
      for(int c=0; c<MDIM; c++) T1[b][c] += T1[a][c] * co, T2[b][c] += T2[a][c] * co;
      }
    ld co = 1 / T1[a][a];
    for(int c=0; c<MDIM; c++) T1[a][c] *= co, T2[a][c] *= co;
    }
  #endif

  profile_stop(7);
  return T2;
  }

// distance between mh and 0
double hdist0(const hyperpoint& mh) {
  switch(cgclass) {
    case gcHyperbolic:
      if(mh[DIM] < 1) return 0;
      return acosh(mh[DIM]);
    case gcEuclid: {
      ld d = sqhypot_d(mh, DIM);
      return d;
      }
    case gcSphere: {
      ld res = mh[DIM] >= 1 ? 0 : mh[DIM] <= -1 ? M_PI : acos(mh[DIM]);
      if(elliptic && res > M_PI/2) res = M_PI-res;
      return res;
      }
    default:
      return 0;
    }
  }

ld circlelength(ld r) {
  switch(cgclass) {
    case gcEuclid:
      return 2 * M_PI * r;
    case gcHyperbolic:
      return 2 * M_PI * sinh(r);
    case gcSphere:
      return 2 * M_PI * sin(r);
    default:
      return 0;
    }
  }

// distance between two points
double hdist(const hyperpoint& h1, const hyperpoint& h2) {
  return hdist0(gpushxto0(h1) * h2);
  ld iv = intval(h1, h2);
  switch(cgclass) {
    case gcEuclid:
      return sqrt(iv);
    case gcHyperbolic:
      return 2 * asinh(sqrt(iv) / 2);
    case gcSphere:
      return 2 * asin_auto_clamp(sqrt(iv) / 2);
    default:
      return 0;
    }
  }

hyperpoint mscale(const hyperpoint& t, double fac) {
  hyperpoint res;
  for(int i=0; i<MDIM; i++) 
    res[i] = t[i] * fac;
  return res;
  }

transmatrix mscale(const transmatrix& t, double fac) {
  transmatrix res;
  for(int i=0; i<MDIM; i++) for(int j=0; j<MDIM; j++)
    res[i][j] = t[i][j] * fac;
  return res;
  }

transmatrix xyscale(const transmatrix& t, double fac) {
  transmatrix res;
  for(int i=0; i<MDIM; i++) for(int j=0; j<DIM; j++)
    res[i][j] = t[i][j] * fac;
  return res;
  }

transmatrix xyzscale(const transmatrix& t, double fac, double facz) {
  transmatrix res;
  for(int i=0; i<MDIM; i++) for(int j=0; j<DIM; j++)
    res[i][j] = t[i][j] * fac;
  for(int i=0; i<MDIM; i++) 
    res[i][DIM] = t[i][DIM] * facz;
  return res;
  }

// double downspin_zivory;

transmatrix mzscale(const transmatrix& t, double fac) {
  // take only the spin
  transmatrix tcentered = gpushxto0(tC0(t)) * t;
  // tcentered = tcentered * spin(downspin_zivory);
  fac -= 1;
  transmatrix res = t * inverse(tcentered) * ypush(-fac) * tcentered;
  fac *= .2;
  fac += 1;
  for(int i=0; i<MDIM; i++) for(int j=0; j<MDIM; j++)
    res[i][j] = res[i][j] * fac;
  return res;
  }

transmatrix pushone() { return xpush(sphere?.5 : 1); }

bool operator == (hyperpoint h1, hyperpoint h2) {
  for(int i=0; i<MDIM; i++) if(h1[i] != h2[i]) return false;
  return true;
  }

// rotation matrix in R^3

transmatrix rotmatrix(double rotation, int c0, int c1) {
  transmatrix t = Id;
  t[c0][c0] = cos(rotation);
  t[c1][c1] = cos(rotation);
  t[c0][c1] = sin(rotation);
  t[c1][c0] = -sin(rotation);
  return t;
  }

hyperpoint mid3(hyperpoint h1, hyperpoint h2, hyperpoint h3) {
  using namespace hyperpoint_vec;
  return mid(h1+h2+h3, h1+h2+h3);
  }

hyperpoint mid_at(hyperpoint h1, hyperpoint h2, ld v) {
  using namespace hyperpoint_vec;
  hyperpoint h = h1 * (1-v) + h2 * v;
  return mid(h, h);
  }  

hyperpoint mid_at_actual(hyperpoint h, ld v) {
  using namespace hyperpoint_vec;
  return rspintox(h) * xpush0(hdist0(h) * v);
  }

}
