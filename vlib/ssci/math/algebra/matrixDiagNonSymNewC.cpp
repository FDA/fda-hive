/*
 *  ::718604!
 * 
 * Copyright(C) November 20, 2014 U.S. Food and Drug Administration
 * Authors: Dr. Vahan Simonyan (1), Dr. Raja Mazumder (2), et al
 * Affiliation: Food and Drug Administration (1), George Washington University (2)
 * 
 * All rights Reserved.
 * 
 * The MIT License (MIT)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include <ssci/math/algebra/algebra.hpp>
#include <slib/core/vec.hpp>
#include <math.h>

using namespace slib;
typedef real REAL;
#define ZERO 0.0
#define ABS sAbs
#define SWAP(typ, a, b)  { typ temp; temp = a; a = b; b = temp; }
#define ONE ((REAL)1.)
#define TWO ((REAL)2.)
#define SQRT(x)    sqrtl((x))
#define SQR(X) ((X) * (X))
#define MACH_EPS  (REAL)FLT_EPSILON
#define VEKTOR   0
#define BASIS     2






idx comdiv
           (
            REAL   ar,
            REAL   ai,
            REAL   br,
            REAL   bi,
            REAL * cr,
            REAL * ci
           )
{
  REAL tmp;

  if (br == ZERO && bi == ZERO) return (1);

  if (ABS (br) > ABS (bi))
  {
    tmp  = bi / br;
    br   = tmp * bi + br;
    *cr  = (ar + tmp * ai) / br;
    *ci  = (ai - tmp * ar) / br;
  }
  else
  {
    tmp  = br / bi;
    bi   = tmp * br + bi;
    *cr  = (tmp * ar + ai) / bi;
    *ci  = (tmp * ai - ar) / bi;
 }

 return (0);
}


REAL comabs
              (
               REAL  ar,
               REAL  ai
              )
{
  if (ar == ZERO && ai == ZERO) return (ZERO);

  ar = ABS (ar);
  ai = ABS (ai);

  if (ai > ar)
    SWAP (REAL, ai, ar)

  return ((ai == ZERO) ? (ar) : (ar * SQRT (ONE + ai / ar * ai / ar)));
}



#define mat(_v_i,_v_j) mat_[(_v_i)*n+(_v_j)]
#define eivec(_v_i,_v_j) eivec_[(_v_i)*n+(_v_j)]
#define hmat(_v_i,_v_j) hmat_[(_v_i)*n+(_v_j)]
#define vmat(_v_i,_v_j) vmat_[(_v_i)*n+(_v_j)]




#define MAXIT 50



static idx balance
                   (idx       n,
                    REAL *    mat_,
                    REAL      scal[],
                    idx *     low,
                    idx *     high,
                    idx       basis
                   )
{
  idx i, j;
  idx      iter, k, m;
  REAL     b2, r, c, f, g, s;

  b2 = (REAL) (basis * basis);
  m = 0;
  k = n - 1;

  do
  {
    iter = FALSE;
    for (j = k; j >= 0; j--)
    {
      for (r = ZERO, i = 0; i <= k; i++)
        if (i != j)  r += ABS (mat(j,i));

      if (r == ZERO)
      {
        scal[k] = (REAL) j;
        if (j != k)
        {
          for (i = 0; i <= k; i++) SWAP (REAL, mat(i,j), mat(i,k))
          for (i = m; i < n; i++)  SWAP (REAL, mat(j,i), mat(k,i))
        }
        k--;
        iter = TRUE;
      }
    }
  }
  while (iter);

  do
  {
    iter = FALSE;
    for (j = m; j <= k; j++)
    {
      for (c = ZERO, i = m; i <= k; i++)
        if (i != j) c += ABS (mat(i,j));
      if (c == ZERO)
      {
        scal[m] = (REAL) j;
        if ( j != m )
        {
          for (i = 0; i <= k; i++) SWAP (REAL, mat(i,j), mat(i,m))
          for (i = m; i < n; i++)  SWAP (REAL, mat(j,i), mat(m,i))
        }
        m++;
        iter = TRUE;
      }
    }
  }
  while (iter);

  *low = m;
  *high = k;
  for (i = m; i <= k; i++) scal[i] = ONE;

  do
  {
    iter = FALSE;
    for (i = m; i <= k; i++)
    {
      for (c = r = ZERO, j = m; j <= k; j++)
      if (j !=i)
      {
        c += ABS (mat(j,i));
        r += ABS (mat(i,j));
      }
      g = r / basis;
      f = ONE;
      s = c + r;

      while (c < g)
      {
        f *= basis;
        c *= b2;
      }

      g = r * basis;
      while (c >= g)
      {
        f /= basis;
        c /= b2;
      }

      if ((c + r) / f < (REAL)0.95 * s)
      {
        g = ONE / f;
        scal[i] *= f;
        iter = TRUE;
        for (j = m; j < n; j++ ) mat(i,j) *= g;
        for (j = 0; j <= k; j++ ) mat(j,i) *= f;
      }
    }
  }
  while (iter);

  return (0);
}


static idx balback
                   (idx     n,
                    idx     low,
                    idx     high,
                    REAL    scal[],
                    REAL *  eivec_
                   )
{
  idx i, j, k;
  REAL s;

  for (i = low; i <= high; i++)
  {
    s = scal[i];
    for (j = 0; j < n; j++) eivec(i,j) *= s;
  }

  for (i = low - 1; i >= 0; i--)
  {
    k = (int) scal[i];
    if (k != i)
      for (j = 0; j < n; j++) SWAP (REAL, eivec(i,j), eivec(k,j))
  }

  for (i = high + 1; i < n; i++)
  {
    k = (int) scal[i];
    if (k != i)
      for (j = 0; j < n; j++) SWAP (REAL, eivec(i,j), eivec(k,j))
  }
  return (0);
}



static idx elmhes
                  (idx       n,
                   idx       low,
                   idx       high,
                   REAL *    mat_,
                   idx       perm[]
                  )
{
  idx i, j, m;
  REAL   x, y;

  for (m = low + 1; m < high; m++)
  {
    i = m;
    x = ZERO;
    for (j = m; j <= high; j++)
      if (ABS (mat(j,m-1)) > ABS (x))
      {
        x = mat(j,m-1);
        i = j;
      }

    perm[m] = i;
    if (i != m)
    {
      for (j = m - 1; j < n; j++) SWAP (REAL, mat(i,j), mat(m,j))
      for (j = 0; j <= high; j++) SWAP (REAL, mat(j,i), mat(j,m))
    }

    if (x != ZERO)
    {
      for (i = m + 1; i <= high; i++)
      {
        y = mat(i,m-1);
        if (y != ZERO)
        {
          y = mat(i,m-1) = y / x;
          for (j = m; j < n; j++) mat(i,j) -= y * mat(m,j);
          for (j = 0; j <= high; j++) mat(j,m) += y * mat(j,i);
        }
      }
    }
  }

  return (0);
}


static idx elmtrans
                    (idx     n,
                     idx     low,
                     idx     high,
                     REAL *  mat_,
                     idx     perm[],
                     REAL *  hmat_
                    )
{
  idx k, i, j;

  for (i = 0; i < n; i++)
  {
    for (k = 0; k < n; k++) hmat(i,k) = ZERO;
    hmat(i,i) = ONE;
  }

  for (i = high - 1; i > low; i--)
  {
    j = perm[i];
    for (k = i + 1; k <= high; k++) hmat(k,i) = mat(k,i-1);
    if ( i != j )
    {
      for (k = i; k <= high; k++)
      {
        hmat(i,k) = hmat(j,k);
        hmat(j,k) = ZERO;
      }
      hmat(j,i) = ONE;
    }
  }

  return (0);
}



static idx orthes
                 (
                  idx  n,
                  idx  low,
                  idx  high,
                  REAL *mat_,
                  REAL d[]
                 )


{
  idx  i, j, m;
  REAL s,
       x = ZERO,
       y,
       eps;

  eps = (REAL)128.0 * MACH_EPS;

  for (m = low + 1; m < high; m++)
  {
    for (y = ZERO, i = high; i >= m; i--)
      x    = mat(i,m - 1),
      d[i] = x,
      y    = y + x * x;
    if (y <= eps)
      s = ZERO;
    else
    {
      s = (x >= ZERO) ? -SQRT(y) : SQRT(y);
      y    -= x * s;
      d[m] =  x - s;

      for (j = m; j < n; j++)
      {
        for (x = ZERO, i = high; i >= m; i--)
          x += d[i] * mat(i,j);
        for (x /= y, i = m; i <= high; i++)
          mat(i,j) -= x * d[i];
      }

      for (i = 0; i <= high; i++)
      {
        for (x = ZERO, j = high; j >= m; j--)
          x += d[j] * mat(i,j);
        for (x /= y, j = m; j <= high; j++)
          mat(i,j) -= x * d[j];
      }
    }

    mat(m,m - 1) = s;
  }

  return 0;

}



static idx orttrans
                   (
                    idx  n,
                    idx  low,
                    idx  high,
                    REAL *mat_,
                    REAL d[],
                    REAL *vmat_
                   )


{
  idx  i, j, m;
  REAL x,
       y;

  for (i = 0; i < n; i++)
  {
    for (j = 0; j < n; j++)
      vmat(i,j) = ZERO;
    vmat(i,i) = ONE;
  }

  for (m = high - 1; m > low; m--)
  {
    y = mat(m,m - 1);
    if (y != ZERO)
    {
      y *= d[m];
      for (i = m + 1; i <= high; i++)
        d[i] = mat(i,m - 1);
      for (j = m; j <= high; j++)
      {
        for (x = ZERO, i = m; i <= high; i++)
          x += d[i] * vmat(i,j);
        for (x /= y, i = m; i <= high; i++)
          vmat(i,j) += x * d[i];
      }
    }
  }

  return 0;

}


static idx hqrvec
                  (idx     n,
                   idx     low,
                   idx     high,
                   REAL *  hmat_,
                   REAL    wr[],
                   REAL    wi[],
                   REAL *  eivec_
                  )
{
  idx   i, j, k;
  idx   l, m, en, na;
  REAL  p, q, r = ZERO, s = ZERO, t, w, x, y, z = ZERO,
        ra, sa, vr, vi, norm;

  for (norm = ZERO, i = 0; i < n; i++)
  {
    for (j = i; j < n; j++) norm += ABS(hmat(i,j));
  }

  if (norm == ZERO) return (1);

  for (en = n - 1; en >= 0; en--)
  {
    p = wr[en];
    q = wi[en];
    na = en - 1;
    if (q == ZERO)
    {
      m = en;
      hmat(en,en) = ONE;
      for (i = na; i >= 0; i--)
      {
        w = hmat(i,i) - p;
        r = hmat(i,en);
        for (j = m; j <= na; j++) r += hmat(i,j) * hmat(j,en);
        if (wi[i] < ZERO)
        {
          z = w;
          s = r;
        }
        else
        {
          m = i;
          if (wi[i] == ZERO)
            hmat(i,en) = -r / ((w != ZERO) ? (w) : (MACH_EPS * norm));
          else
          {

             x = hmat(i,i+1);
             y = hmat(i+1,i);
             q = SQR (wr[i] - p) + SQR (wi[i]);
             hmat(i,en) = t = (x * s - z * r) / q;
             hmat(i+1,en) = ( (ABS(x) > ABS(z) ) ?
                                (-r -w * t) / x : (-s -y * t) / z);
          }
        }
      }
    }

    else if (q < ZERO)
    {
      m = na;
      if (ABS(hmat(en,na)) > ABS(hmat(na,en)))
      {
        hmat(na,na) = - (hmat(en,en) - p) / hmat(en,na);
        hmat(na,en) = - q / hmat(en,na);
      }
      else
       comdiv(-hmat(na,en), ZERO, hmat(na,na)-p, q, &hmat(na,na), &hmat(na,en));

      hmat(en,na) = ONE;
      hmat(en,en) = ZERO;
      for (i = na - 1; i >= 0; i--)
      {
        w = hmat(i,i) - p;
        ra = hmat(i,en);
        sa = ZERO;
        for (j = m; j <= na; j++)
        {
          ra += hmat(i,j) * hmat(j,na);
          sa += hmat(i,j) * hmat(j,en);
        }

        if (wi[i] < ZERO)
        {
          z = w;
          r = ra;
          s = sa;
        }
        else
        {
          m = i;
          if (wi[i] == ZERO)
            comdiv (-ra, -sa, w, q, &hmat(i,na), &hmat(i,en));
          else
          {


            x = hmat(i,i+1);
            y = hmat(i+1,i);
            vr = SQR (wr[i] - p) + SQR (wi[i]) - SQR (q);
            vi = TWO * q * (wr[i] - p);
            if (vr == ZERO && vi == ZERO)
              vr = MACH_EPS * norm *
                  (ABS (w) + ABS (q) + ABS (x) + ABS (y) + ABS (z));

            comdiv (x * r - z * ra + q * sa, x * s - z * sa -q * ra,
                    vr, vi, &hmat(i,na), &hmat(i,en));
            if (ABS (x) > ABS (z) + ABS (q))
            {
              hmat(i+1,na) = (-ra - w * hmat(i,na) + q * hmat(i,en)) / x;
              hmat(i+1,en) = (-sa - w * hmat(i,en) - q * hmat(i,na)) / x;
            }
            else
              comdiv (-r - y * hmat(i,na), -s - y * hmat(i,en), z, q,
                                              &hmat(i+1,na), &hmat(i+1,en));

          }
        }
      }
    }
  }

  for (i = 0; i < n; i++)
    if (i < low || i > high)
      for (k = i + 1; k < n; k++) eivec(i,k) = hmat(i,k);

  for (j = n - 1; j >= low; j--)
  {
    m = (j <= high) ? j : high;
    if (wi[j] < ZERO)
    {
      for (l = j - 1, i = low; i <= high; i++)
      {
        for (y = z = ZERO, k = low; k <= m; k++)
        {
          y += eivec(i,k) * hmat(k,l);
          z += eivec(i,k) * hmat(k,j);
        }

        eivec(i,l) = y;
        eivec(i,j) = z;
      }
    }
    else
      if (wi[j] == ZERO)
      {
        for (i = low; i <= high; i++)
        {
          for (z = ZERO, k = low; k <= m; k++)
            z += eivec(i,k) * hmat(k,j);
          eivec(i,j) = z;
        }
      }

  }

  return (0);
}


static idx hqr2
                (idx     vec,
                 idx     n,
                 idx     low,
                 idx     high,
                 REAL *  hmat_,
                 REAL    wr[],
                 REAL    wi[],
                 REAL *  eivec_,
                 idx     cnt[]
                )
{
  idx  i, j;
  idx  na, en, iter, k, l, m;
  REAL p = ZERO, q = ZERO, r = ZERO, s, t, w, x, y, z;

  for (i = 0; i < n; i++)
    if (i < low || i > high)
    {
      wr[i] = hmat(i,i);
      wi[i] = ZERO;
      cnt[i] = 0;
    }

  en = high;
  t = ZERO;

  while (en >= low)
  {
    iter = 0;
    na = en - 1;

    for ( ; ; )
    {
      for (l = en; l > low; l--)
        if ( ABS(hmat(l,l-1)) <=
              MACH_EPS * (ABS(hmat(l-1,l-1)) + ABS(hmat(l,l))) )  break;

      x = hmat(en,en);
      if (l == en)
      {
        wr[en] = hmat(en,en) = x + t;
        wi[en] = ZERO;
        cnt[en] = iter;
        en--;
        break;
      }

      y = hmat(na,na);
      w = hmat(en,na) * hmat(na,en);

      if (l == na)
      {
        p = (y - x) * 0.5;
        q = p * p + w;
        z = SQRT (ABS (q));
        x = hmat(en,en) = x + t;
        hmat(na,na) = y + t;
        cnt[en] = -iter;
        cnt[na] = iter;
        if (q >= ZERO)
        {
          z = (p < ZERO) ? (p - z) : (p + z);
          wr[na] = x + z;
          wr[en] = s = x - w / z;
          wi[na] = wi[en] = ZERO;
          x = hmat(en,na);
          r = SQRT (x * x + z * z);

          if (vec)
          {
            p = x / r;
            q = z / r;
            for (j = na; j < n; j++)
            {
              z = hmat(na,j);
              hmat(na,j) = q * z + p * hmat(en,j);
              hmat(en,j) = q * hmat(en,j) - p * z;
            }

            for (i = 0; i <= en; i++)
            {
              z = hmat(i,na);
              hmat(i,na) = q * z + p * hmat(i,en);
              hmat(i,en) = q * hmat(i,en) - p * z;
            }

            for (i = low; i <= high; i++)
            {
              z = eivec(i,na);
              eivec(i,na) = q * z + p * eivec(i,en);
              eivec(i,en) = q * eivec(i,en) - p * z;
            }
          }
        }
        else
        {
          wr[na] = wr[en] = x + p;
          wi[na] =   z;
          wi[en] = - z;
        }

        en -= 2;
        break;
      }

      if (iter >= MAXIT)
      {
        cnt[en] = MAXIT + 1;
        return (en);
      }

      if ( (iter != 0) && (iter % 10 == 0) )
      {
        t += x;
        for (i = low; i <= en; i++) hmat(i,i) -= x;
        s = ABS (hmat(en,na)) + ABS (hmat(na,en-2));
        x = y = (REAL)0.75 * s;
        w = - (REAL)0.4375 * s * s;
      }

      iter ++;

      for (m = en - 2; m >= l; m--)
      {
        z = hmat(m,m);
        r = x - z;
        s = y - z;
        p = ( r * s - w ) / hmat(m+1,m) + hmat(m,m+1);
        q = hmat(m + 1,m + 1) - z - r - s;
        r = hmat(m + 2,m + 1);
        s = ABS (p) + ABS (q) + ABS (r);
        p /= s;
        q /= s;
        r /= s;
        if (m == l) break;
        if ( ABS (hmat(m,m-1)) * (ABS (q) + ABS (r)) <=
                 MACH_EPS * ABS (p)
                 * ( ABS (hmat(m-1,m-1)) + ABS (z) + ABS (hmat(m+1,m+1))) )
          break;
      }

      for (i = m + 2; i <= en; i++) hmat(i,i-2) = ZERO;
      for (i = m + 3; i <= en; i++) hmat(i,i-3) = ZERO;

      for (k = m; k <= na; k++)
      {
        if (k != m)
        {
          p = hmat(k,k-1);
          q = hmat(k+1,k-1);
          r = (k != na) ? hmat(k+2,k-1) : ZERO;
          x = ABS (p) + ABS (q) + ABS (r);
          if (x == ZERO) continue;
          p /= x;
          q /= x;
          r /= x;
        }
        s = SQRT (p * p + q * q + r * r);
        if (p < ZERO) s = -s;

        if (k != m) hmat(k,k-1) = -s * x;
          else if (l != m)
                 hmat(k,k-1) = -hmat(k,k-1);
        p += s;
        x = p / s;
        y = q / s;
        z = r / s;
        q /= p;
        r /= p;

        for (j = k; j < n; j++)
        {
          p = hmat(k,j) + q * hmat(k+1,j);
          if (k != na)
          {
            p += r * hmat(k+2,j);
            hmat(k+2,j) -= p * z;
          }
          hmat(k+1,j) -= p * y;
          hmat(k,j)   -= p * x;
        }

        j = (k + 3 < en) ? (k + 3) : en;
        for (i = 0; i <= j; i++)
        {
          p = x * hmat(i,k) + y * hmat(i,k+1);
          if (k != na)
          {
            p += z * hmat(i,k+2);
            hmat(i,k+2) -= p * r;
          }
          hmat(i,k+1) -= p * q;
          hmat(i,k)   -= p;
        }

        if (vec)
        {
          for (i = low; i <= high; i++)
          {
            p = x * eivec(i,k) + y * eivec(i,k+1);
            if (k != na)
            {
              p += z * eivec(i,k+2);
              eivec(i,k+2) -= p * r;
            }
            eivec(i,k+1) -= p * q;
            eivec(i,k)   -= p;
          }
        }
      }

    }

  }

  if (vec)
    if (hqrvec (n, low, high, hmat_, wr, wi, eivec_)) return (99);
  return (0);
}


static idx norm_1
                  (idx     n,
                   REAL *  vmat_,
                   REAL    wi[]
                  )
{
  idx  i, j;
  REAL maxi, tr, ti;

  if (n < 1) return (1);

  for (j = 0; j < n; j++)
  {
    if (wi[j] == ZERO)
    {
      maxi = vmat(0,j);
      for (i = 1; i < n; i++)
        if (ABS (vmat(i,j)) > ABS (maxi))  maxi = vmat(i,j);

      if (maxi != ZERO)
      {
        maxi = ONE / maxi;
        for (i = 0; i < n; i++) vmat(i,j) *= maxi;
      }
    }
    else
    {
      tr = vmat(0,j);
      ti = vmat(0,j+1);
      for (i = 1; i < n; i++)
        if ( comabs (vmat(i,j), vmat(i,j+1)) > comabs (tr, ti) )
        {
          tr = vmat(i,j);
          ti = vmat(i,j+1);
        }

      if (tr != ZERO || ti != ZERO)
        for (i = 0; i < n; i++)
          comdiv (vmat(i,j), vmat(i,j+1), tr, ti, &vmat(i,j), &vmat(i,j+1));

      j++;
    }
  }
  return (0);
}


idx sAlgebra::matrix::eigen
          (
           idx     vec,
           idx     ortho,
           idx     ev_norm,
           idx     n,
           REAL * mat_,
           REAL * eivec_,
           REAL  * valre,
           REAL  * valim,
           idx   * cnt
          )
{
  idx i;
  idx      low, high, rc;
  sVec < REAL > scale, d ;

  if (n < 1) return (1);



  for (i = 0; i < n; i++) cnt[i] = 0;

  if (n == 1)
  {
    eivec(0,0) = ONE;
    valre[0]    = mat(0,0);
    valim[0]    = ZERO;
    return (0);
  }


  scale.resize(n);

  if (vec && ortho)
  {
    d.resize(n);
  }

  rc = balance (n, mat_, scale,
                  &low, &high, BASIS);
  if (rc)
  {
    return (100 + rc);
  }

  if (ortho)
    rc = orthes(n, low, high, mat_, d);
  else
    rc = elmhes (n, low, high, mat_, cnt);
  if (rc)
  {
    return (200 + rc);
  }

  if (vec)
  {
    if (ortho)
      rc = orttrans(n, low, high, mat_, d, eivec_);
    else
      rc = elmtrans (n, low, high, mat_, cnt, eivec_);
    if (rc)
    {
      return (300 + rc);
    }
  }

  rc = hqr2 (vec, n, low, high, mat_,
             valre, valim, eivec_, cnt);
  if (rc)
  {
    return (400 + rc);
  }

  if (vec)
  {
    rc = balback (n, low, high,
                      scale, eivec_);
    if (rc)
    {
      return (500 + rc);
    }
    if (ev_norm)
      rc = norm_1 (n, eivec_, valim);
    if (rc)
    {
      return (600 + rc);
    }
  }



  return (0);
}

