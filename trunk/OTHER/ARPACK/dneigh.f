c-----------------------------------------------------------------------
c\BeginDoc
c
c\Name: dneigh
c
c\Description:
c  Compute the eigenvalues of the current upper Hessenberg matrix
c  and the corresponding Ritz estimates given the current residual norm.
c
c\Usage:
c  call dneigh
c     ( RNORM, N, H, LDH, RITZR, RITZI, BOUNDS, Q, LDQ, WORKL, IERR )
c
c\Arguments
c  RNORM   Double precision scalar.  (INPUT)
c          Residual norm corresponding to the current upper Hessenberg 
c          matrix H.
c
c  N       Integer.  (INPUT)
c          Size of the matrix H.
c
c  H       Double precision N by N array.  (INPUT)
c          H contains the current upper Hessenberg matrix.
c
c  LDH     Integer.  (INPUT)
c          Leading dimension of H exactly as declared in the calling
c          program.
c
c  RITZR,  Double precision arrays of length N.  (OUTPUT)
c  RITZI   On output, RITZR(1:N) (resp. RITZI(1:N)) contains the real 
c          (respectively imaginary) parts of the eigenvalues of H.
c
c  BOUNDS  Double precision array of length N.  (OUTPUT)
c          On output, BOUNDS contains the Ritz estimates associated with
c          the eigenvalues RITZR and RITZI.  This is equal to RNORM 
c          times the last components of the eigenvectors corresponding 
c          to the eigenvalues in RITZR and RITZI.
c
c  Q       Double precision N by N array.  (WORKSPACE)
c          Workspace needed to store the eigenvectors of H.
c
c  LDQ     Integer.  (INPUT)
c          Leading dimension of Q exactly as declared in the calling
c          program.
c
c  WORKL   Double precision work array of length N**2 + 3*N.  (WORKSPACE)
c          Private (replicated) array on each PE or array allocated on
c          the front end.  This is needed to keep the full Schur form
c          of H and also in the calculation of the eigenvectors of H.
c
c  IERR    Integer.  (OUTPUT)
c          Error exit flag from dlaqrb or dtrevc.
c
c\EndDoc
c
c-----------------------------------------------------------------------
c
c\BeginLib
c
c\Local variables:
c     xxxxxx  real
c
c\Routines called:
c     dlaqrb  ARPACK routine to compute the real Schur form of an
c             upper Hessenberg matrix and last row of the Schur vectors.
c     second  ARPACK utility routine for timing.
c     dmout   ARPACK utility routine that prints matrices
c     dvout   ARPACK utility routine that prints vectors.
c     dlacpy  LAPACK matrix copy routine.
c     dlapy2  LAPACK routine to compute sqrt(x**2+y**2) carefully.
c     dtrevc  LAPACK routine to compute the eigenvectors of a matrix
c             in upper quasi-triangular form
c     dgemv   Level 2 BLAS routine for matrix vector multiplication.
c     dcopy   Level 1 BLAS that copies one vector to another .
c     dnrm2   Level 1 BLAS that computes the norm of a vector.
c     dscal   Level 1 BLAS that scales a vector.
c     
c
c\Author
c     Danny Sorensen               Phuong Vu
c     Richard Lehoucq              CRPC / Rice University
c     Dept. of Computational &     Houston, Texas
c     Applied Mathematics
c     Rice University           
c     Houston, Texas    
c
c\Revision history:
c     xx/xx/92: Version ' 2.1'
c
c\SCCS Information: @(#) 
c FILE: neigh.F   SID: 2.3   DATE OF SID: 4/20/96   RELEASE: 2
c
c\Remarks
c     None
c
c\EndLib
c
c-----------------------------------------------------------------------
c
      subroutine dneigh (rnorm, n, h, ldh, ritzr, ritzi, bounds, 
     &                   q, ldq, workl, ierr)
c
c     %----------------------------------------------------%
c     | Include files for debugging and timing information |
c     %----------------------------------------------------%
c
      include   'debug.h'
      include   'stat.h'
c
c     %------------------%
c     | Scalar Arguments |
c     %------------------%
c
      integer    ierr, n, ldh, ldq
      Double precision     
     &           rnorm
c
c     %-----------------%
c     | Array Arguments |
c     %-----------------%
c
      Double precision     
     &           bounds(n), h(ldh,n), q(ldq,n), ritzi(n), ritzr(n),
     &           workl(n*(n+3))
c 
c     %------------%
c     | Parameters |
c     %------------%
c
      Double precision     
     &           one, zero
      parameter (one = 1.0D+0, zero = 0.0D+0)
c 
c     %------------------------%
c     | Local Scalars & Arrays |
c     %------------------------%
c
      logical    select(1)
      integer    i, iconj, msglvl
      Double precision     
     &           temp, vl(1)
c
c     %----------------------%
c     | External Subroutines |
c     %----------------------%
c
      external   dcopy, dlacpy, dlaqrb, dtrevc, dvout, second
c
c     %--------------------%
c     | External Functions |
c     %--------------------%
c
      Double precision
     &           dlapy2, dnrm2
      external   dlapy2, dnrm2
c
c     %---------------------%
c     | Intrinsic Functions |
c     %---------------------%
c
      intrinsic  abs
c
c     %-----------------------%
c     | Executable Statements |
c     %-----------------------%
c
c
c     %-------------------------------%
c     | Initialize timing statistics  |
c     | & message level for debugging |
c     %-------------------------------%
c
      call second (t0)
      msglvl = mneigh
c 
      if (msglvl .gt. 2) then
          call dmout (logfil, n, n, h, ldh, ndigit, 
     &         '_neigh: Entering upper Hessenberg matrix H ')
      end if
c 
c     %-----------------------------------------------------------%
c     | 1. Compute the eigenvalues, the last components of the    |
c     |    corresponding Schur vectors and the full Schur form T  |
c     |    of the current upper Hessenberg matrix H.              |
c     | dlaqrb returns the full Schur form of H in WORKL(1:N**2)  |
c     | and the last components of the Schur vectors in BOUNDS.   |
c     %-----------------------------------------------------------%
c
      call dlacpy ('All', n, n, h, ldh, workl, n)
      call dlaqrb (.true., n, 1, n, workl, n, ritzr, ritzi, bounds,
     &             ierr)
      if (ierr .ne. 0) go to 9000
c
      if (msglvl .gt. 1) then
         call dvout (logfil, n, bounds, ndigit,
     &              '_neigh: last row of the Schur matrix for H')
      end if
c
c     %-----------------------------------------------------------%
c     | 2. Compute the eigenvectors of the full Schur form T and  |
c     |    apply the last components of the Schur vectors to get  |
c     |    the last components of the corresponding eigenvectors. |
c     | Remember that if the i-th and (i+1)-st eigenvalues are    |
c     | complex conjugate pairs, then the real & imaginary part   |
c     | of the eigenvector components are split across adjacent   |
c     | columns of Q.                                             |
c     %-----------------------------------------------------------%
c
      call dtrevc ('R', 'A', select, n, workl, n, vl, n, q, ldq,
     &             n, n, workl(n*n+1), ierr)
c
      if (ierr .ne. 0) go to 9000
c
c     %------------------------------------------------%
c     | Scale the returning eigenvectors so that their |
c     | euclidean norms are all one. LAPACK subroutine |
c     | dtrevc returns each eigenvector normalized so  |
c     | that the element of largest magnitude has      |
c     | magnitude 1; here the magnitude of a complex   |
c     | number (x,y) is taken to be |x| + |y|.         |
c     %------------------------------------------------%
c
      iconj = 0
      do 10 i=1, n
         if ( abs( ritzi(i) ) .le. zero ) then
c
c           %----------------------%
c           | Real eigenvalue case |
c           %----------------------%
c    
            temp = dnrm2( n, q(1,i), 1 )
            call dscal ( n, one / temp, q(1,i), 1 )
         else
c
c           %-------------------------------------------%
c           | Complex conjugate pair case. Note that    |
c           | since the real and imaginary part of      |
c           | the eigenvector are stored in consecutive |
c           | columns, we further normalize by the      |
c           | square root of two.                       |
c           %-------------------------------------------%
c
            if (iconj .eq. 0) then
               temp = dlapy2( dnrm2( n, q(1,i), 1 ), 
     &                        dnrm2( n, q(1,i+1), 1 ) )
               call dscal ( n, one / temp, q(1,i), 1 )
               call dscal ( n, one / temp, q(1,i+1), 1 )
               iconj = 1
            else
               iconj = 0
            end if
         end if         
   10 continue
c
      call dgemv ('T', n, n, one, q, ldq, bounds, 1, zero, workl, 1)
c
      if (msglvl .gt. 1) then
         call dvout (logfil, n, workl, ndigit,
     &              '_neigh: Last row of the eigenvector matrix for H')
      end if
c
c     %----------------------------%
c     | Compute the Ritz estimates |
c     %----------------------------%
c
      iconj = 0
      do 20 i = 1, n
         if ( abs( ritzi(i) ) .le. zero ) then
c
c           %----------------------%
c           | Real eigenvalue case |
c           %----------------------%
c    
            bounds(i) = rnorm * abs( workl(i) )
         else
c
c           %-------------------------------------------%
c           | Complex conjugate pair case. Note that    |
c           | since the real and imaginary part of      |
c           | the eigenvector are stored in consecutive |
c           | columns, we need to take the magnitude    |
c           | of the last components of the two vectors |
c           %-------------------------------------------%
c
            if (iconj .eq. 0) then
               bounds(i) = rnorm * dlapy2( workl(i), workl(i+1) )
               bounds(i+1) = bounds(i)
               iconj = 1
            else
               iconj = 0
            end if
         end if
   20 continue
c
      if (msglvl .gt. 2) then
         call dvout (logfil, n, ritzr, ndigit,
     &              '_neigh: Real part of the eigenvalues of H')
         call dvout (logfil, n, ritzi, ndigit,
     &              '_neigh: Imaginary part of the eigenvalues of H')
         call dvout (logfil, n, bounds, ndigit,
     &              '_neigh: Ritz estimates for the eigenvalues of H')
      end if
c
      call second (t1)
      tneigh = tneigh + (t1 - t0)
c
 9000 continue
      return
c
c     %---------------%
c     | End of dneigh |
c     %---------------%
c
      end
