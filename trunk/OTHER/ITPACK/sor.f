      SUBROUTINE SOR (NN,IA,JA,A,RHS,U,IWKSP,NW,WKSP,IPARM,RPARM,IERR)
C       
C     ITPACK 2C MAIN SUBROUTINE  SOR  (SUCCESSIVE OVERRELATION)     
C     EACH OF THE MAIN SUBROUTINES:   
C           JCG, JSI, SOR, SSORCG, SSORSI, RSCG, RSSI     
C     CAN BE USED INDEPENDENTLY OF THE OTHERS   
C       
C ... FUNCTION:   
C       
C          THIS SUBROUTINE, SOR, DRIVES THE  SUCCESSIVE   
C          OVERRELAXATION ALGORITHM.  
C       
C ... PARAMETER LIST:       
C       
C          N      INPUT INTEGER.  DIMENSION OF THE MATRIX. (= NN)   
C          IA,JA  INPUT INTEGER VECTORS.  THE TWO INTEGER ARRAYS OF 
C                 THE SPARSE MATRIX REPRESENTATION.       
C          A      INPUT D.P. VECTOR.  THE D.P. ARRAY OF THE SPARSE  
C                 MATRIX REPRESENTATION 
C          RHS    INPUT D.P. VECTOR.  CONTAINS THE RIGHT HAND SIDE  
C                 OF THE MATRIX PROBLEM.
C          U      INPUT/OUTPUT D.P. VECTOR.  ON INPUT, U CONTAINS THE 
C                 INITIAL GUESS TO THE SOLUTION. ON OUTPUT, IT CONTAINS 
C                 THE LATEST ESTIMATE TO THE SOLUTION.    
C          IWKSP  INTEGER VECTOR WORKSPACE OF LENGTH 3*N  
C          NW     INPUT INTEGER.  LENGTH OF AVAILABLE WKSP.  ON OUTPUT, 
C                 IPARM(8) IS AMOUNT USED.      
C          WKSP   D.P. VECTOR USED FOR WORKING SPACE.  SOR NEEDS THIS 
C                 TO BE IN LENGTH AT LEAST  N   
C          IPARM  INTEGER VECTOR OF LENGTH 12.  ALLOWS USER TO SPECIFY
C                 SOME INTEGER PARAMETERS WHICH AFFECT THE METHOD.  
C          RPARM  D.P. VECTOR OF LENGTH 12. ALLOWS USER TO SPECIFY SOME 
C                 D.P. PARAMETERS WHICH AFFECT THE METHOD.
C          IER    OUTPUT INTEGER.  ERROR FLAG. (= IERR)   
C       
C ... SOR SUBPROGRAM REFERENCES:      
C       
C          FROM ITPACK   BISRCH, DFAULT, ECHALL, ECHOUT, IPSTR, ITERM,
C                        TIMER, ITSOR, IVFILL, PERMAT, PERR,      
C                        PERVEC, PFSOR1, PMULT, PRBNDX, PSTOP, QSORT, 
C                        SBELM, SCAL, DCOPY, DDOT, TAU, UNSCAL, VFILL,
C                        VOUT, WEVMW  
C          SYSTEM        DABS, DLOG10, DBLE(AMAX0), DMAX1, DBLE(FLOAT), 
C                        DSQRT
C       
C     VERSION:  ITPACK 2C (MARCH 1982)
C       
C     CODE WRITTEN BY:  DAVID KINCAID, ROGER GRIMES, JOHN RESPESS   
C                       CENTER FOR NUMERICAL ANALYSIS     
C                       UNIVERSITY OF TEXAS     
C                       AUSTIN, TX  78712       
C                       (512) 471-1242
C       
C     FOR ADDITIONAL DETAILS ON THE   
C          (A) SUBROUTINE SEE TOMS ARTICLE 1982 
C          (B) ALGORITHM  SEE CNA REPORT 150    
C       
C     BASED ON THEORY BY:  DAVID YOUNG, DAVID KINCAID, LOU HAGEMAN  
C       
C     REFERENCE THE BOOK:  APPLIED ITERATIVE METHODS      
C                          L. HAGEMAN, D. YOUNG 
C                          ACADEMIC PRESS, 1981 
C       
C     **************************************************  
C     *               IMPORTANT NOTE                   *  
C     *                                                *  
C     *      WHEN INSTALLING ITPACK ROUTINES ON A      *  
C     *  DIFFERENT COMPUTER, RESET SOME OF THE VALUES  *  
C     *  IN  SUBROUTNE DFAULT.   MOST IMPORTANT ARE    *  
C     *                                                *  
C     *   DRELPR      MACHINE RELATIVE PRECISION       *  
C     *   RPARM(1)    STOPPING CRITERION               *  
C     *                                                *  
C     *   ALSO CHANGE SYSTEM-DEPENDENT ROUTINE         *  
C     *   SECOND USED IN TIMER                         *  
C     *                                                *  
C     **************************************************  
C       
C     SPECIFICATIONS FOR ARGUMENTS    
C       
      INTEGER IA(1),JA(1),IWKSP(1),IPARM(12),NN,NW,IERR   
      DOUBLE PRECISION A(1),RHS(NN),U(NN),WKSP(NW),RPARM(12)
C       
C     SPECIFICATIONS FOR LOCAL VARIABLES
C       
      INTEGER IB1,IB2,IB3,IDGTS,IER,IERPER,ITMAX1,LOOP,N,NB,N3      
      DOUBLE PRECISION DIGIT1,DIGIT2,TEMP,TIME1,TIME2,TOL 
C       
C *** BEGIN: ITPACK COMMON  
C       
      INTEGER IN,IS,ISYM,ITMAX,LEVEL,NOUT       
      COMMON /ITCOM1/ IN,IS,ISYM,ITMAX,LEVEL,NOUT 
C       
      LOGICAL ADAPT,BETADT,CASEII,HALT,PARTAD   
      COMMON /ITCOM2/ ADAPT,BETADT,CASEII,HALT,PARTAD     
C       
      DOUBLE PRECISION BDELNM,BETAB,CME,DELNNM,DELSNM,FF,GAMMA,OMEGA,QA,
     *   QT,RHO,RRR,SIGE,SME,SPECR,SPR,DRELPR,STPTST,UDNM,ZETA      
      COMMON /ITCOM3/ BDELNM,BETAB,CME,DELNNM,DELSNM,FF,GAMMA,OMEGA,QA, 
     *   QT,RHO,RRR,SIGE,SME,SPECR,SPR,DRELPR,STPTST,UDNM,ZETA      
C       
C *** END  : ITPACK COMMON  
C       
C ... VARIABLES IN COMMON BLOCK - ITCOM1
C       
C     IN     - ITERATION NUMBER       
C     IS     - ITERATION NUMBER WHEN PARAMETERS LAST CHANGED
C     ISYM   - SYMMETRIC/NONSYMMETRIC STORAGE FORMAT SWITCH 
C     ITMAX  - MAXIMUM NUMBER OF ITERATIONS ALLOWED       
C     LEVEL  - LEVEL OF OUTPUT CONTROL SWITCH   
C     NOUT   - OUTPUT UNIT NUMBER     
C       
C ... VARIABLES IN COMMON BLOCK - ITCOM2
C       
C     ADAPT  - FULLY ADAPTIVE PROCEDURE SWITCH  
C     BETADT - SWITCH FOR ADAPTIVE DETERMINATION OF BETA  
C     CASEII - ADAPTIVE PROCEDURE CASE SWITCH   
C     HALT   - STOPPING TEST SWITCH   
C     PARTAD - PARTIALLY ADAPTIVE PROCEDURE SWITCH
C       
C ... VARIABLES IN COMMON BLOCK - ITCOM3
C       
C     BDELNM - TWO NORM OF B TIMES DELTA-SUPER-N
C     BETAB  - ESTIMATE FOR THE SPECTRAL RADIUS OF LU MATRIX
C     CME    - ESTIMATE OF LARGEST EIGENVALUE   
C     DELNNM - INNER PRODUCT OF PSEUDO-RESIDUAL AT ITERATION N      
C     DELSNM - INNER PRODUCT OF PSEUDO-RESIDUAL AT ITERATION S      
C     FF     - ADAPTIVE PROCEDURE DAMPING FACTOR
C     GAMMA  - ACCELERATION PARAMETER 
C     OMEGA  - OVERRELAXATION PARAMETER FOR SOR AND SSOR  
C     QA     - PSEUDO-RESIDUAL RATIO  
C     QT     - VIRTUAL SPECTRAL RADIUS
C     RHO    - ACCELERATION PARAMETER 
C     RRR    - ADAPTIVE PARAMETER     
C     SIGE   - PARAMETER SIGMA-SUB-E  
C     SME    - ESTIMATE OF SMALLEST EIGENVALUE  
C     SPECR  - SPECTRAL RADIUS ESTIMATE FOR SSOR
C     DRELPR - MACHINE RELATIVE PRECISION       
C     STPTST - STOPPING PARAMETER     
C     UDNM   - TWO NORM OF U
C     ZETA   - STOPPING CRITERION     
C       
C ... INITIALIZE COMMON BLOCKS
C       
      LEVEL = IPARM(2)      
      NOUT = IPARM(4)       
      IF (LEVEL.GE.1) WRITE (NOUT,10) 
   10 FORMAT ('0'///1X,'BEGINNING OF ITPACK SOLUTION MODULE  SOR')  
      IER = 0     
      IF (IPARM(1).LE.0) RETURN       
      N = NN      
      IF (IPARM(11).EQ.0) TIMJ1 = TIMER(DUMMY)  
      IF (LEVEL.GE.3) GO TO 20
      CALL ECHOUT (IPARM,RPARM,3)     
      GO TO 30    
   20 CALL ECHALL (N,IA,JA,A,RHS,IPARM,RPARM,1) 
   30 TEMP = 5.0D2*DRELPR   
      IF (ZETA.GE.TEMP) GO TO 50      
      IF (LEVEL.GE.1) WRITE (NOUT,40) ZETA,DRELPR,TEMP    
   40 FORMAT ('0','*** W A R N I N G ************'/'0',   
     *   '    IN ITPACK ROUTINE SOR'/' ','    RPARM(1) =',D10.3,    
     *   ' (ZETA)'/' ','    A VALUE THIS SMALL MAY HINDER CONVERGENCE '/
     *   ' ','    SINCE MACHINE PRECISION DRELPR =',D10.3/' ',      
     *   '    ZETA RESET TO ',D10.3)  
      ZETA = TEMP 
   50 CONTINUE    
      TIME1 = RPARM(9)      
      TIME2 = RPARM(10)     
      DIGIT1 = RPARM(11)    
      DIGIT2 = RPARM(12)    
C       
C ... VERIFY N    
C       
      IF (N.GT.0) GO TO 70  
      IER = 31    
      IF (LEVEL.GE.0) WRITE (NOUT,60) N 
   60 FORMAT ('0','*** F A T A L     E R R O R ************'/'0',   
     *   '    CALLED FROM ITPACK ROUTINE SOR '/' ',       
     *   '    INVALID MATRIX DIMENSION, N =',I8)
      GO TO 360   
   70 CONTINUE    
C       
C ... REMOVE ROWS AND COLUMNS IF REQUESTED      
C       
      IF (IPARM(10).EQ.0) GO TO 90    
      TOL = RPARM(8)
      CALL IVFILL (N,IWKSP,0) 
      CALL VFILL (N,WKSP,0.0D0)       
      CALL SBELM (N,IA,JA,A,RHS,IWKSP,WKSP,TOL,ISYM,LEVEL,NOUT,IER) 
      IF (IER.EQ.0) GO TO 90
      IF (LEVEL.GE.0) WRITE (NOUT,80) IER,TOL   
   80 FORMAT ('0','*** F A T A L     E R R O R ************'/'0',   
     *   '    CALLED FROM ITPACK ROUTINE SOR '/' ',       
     *   '    ERROR DETECTED IN SUBROUTINE  SBELM '/' ',  
     *   '    WHICH REMOVES ROWS AND COLUMNS OF SYSTEM '/' ',       
     *   '    WHEN DIAGONAL ENTRY TOO LARGE  '/' ','    IER = ',I5,5X,
     *   ' RPARM(8) = ',D10.3,' (TOL)') 
      GO TO 360   
C       
C ... INITIALIZE WKSP BASE ADDRESSES. 
C       
   90 IB1 = 1     
      IB2 = IB1+N 
      IB3 = IB2+N 
      IPARM(8) = N
      IF (NW.GE.IPARM(8)) GO TO 110   
      IER = 32    
      IF (LEVEL.GE.0) WRITE (NOUT,100) NW,IPARM(8)
  100 FORMAT ('0','*** F A T A L     E R R O R ************'/'0',   
     *   '    CALLED FROM ITPACK ROUTINE SOR '/' ',       
     *   '    NOT ENOUGH WORKSPACE AT ',I10/' ','    SET IPARM(8) =',I10
     *   ,' (NW)')
      GO TO 360   
C       
C ... PERMUTE TO  RED-BLACK SYSTEM IF REQUESTED 
C       
  110 NB = IPARM(9) 
      IF (NB.LT.0) GO TO 170
      N3 = 3*N    
      CALL IVFILL (N3,IWKSP,0)
      CALL PRBNDX (N,NB,IA,JA,IWKSP,IWKSP(IB2),LEVEL,NOUT,IER)      
      IF (IER.EQ.0) GO TO 130 
      IF (LEVEL.GE.0) WRITE (NOUT,120) IER,NB   
  120 FORMAT ('0','*** F A T A L     E R R O R ************'/'0',   
     *   '    CALLED FROM ITPACK ROUTINE SOR '/' ',       
     *   '    ERROR DETECTED IN SUBROUTINE  PRBNDX'/' ',  
     *   '    WHICH COMPUTES THE RED-BLACK INDEXING'/' ','    IER = ',I5
     *   ,' IPARM(9) = ',I5,' (NB)')  
      GO TO 360   
C       
C ... PERMUTE MATRIX AND RHS
C       
  130 IF (LEVEL.GE.2) WRITE (NOUT,140) NB       
  140 FORMAT (/10X,'ORDER OF BLACK SUBSYSTEM = ',I5,' (NB)')
      CALL PERMAT (N,IA,JA,A,IWKSP,IWKSP(IB3),ISYM,LEVEL,NOUT,IER)  
      IF (IER.EQ.0) GO TO 160 
      IF (LEVEL.GE.0) WRITE (NOUT,150) IER      
  150 FORMAT ('0','*** F A T A L     E R R O R ************'/'0',   
     *   '    CALLED FROM ITPACK ROUTINE SOR '/' ',       
     *   '    ERROR DETECTED IN SUBROUTINE  PERMAT'/' ',  
     *   '    WHICH DOES THE RED-BLACK PERMUTATION'/' ','    IER = ',I5)
      GO TO 360   
  160 CALL PERVEC (N,RHS,IWKSP)       
      CALL PERVEC (N,U,IWKSP) 
C       
C ... SCALE LINEAR SYSTEM, U, AND RHS BY THE SQUARE ROOT OF THE     
C ... DIAGONAL ELEMENTS.    
C       
  170 CONTINUE    
      CALL VFILL (IPARM(8),WKSP,0.0D0)
      CALL SCAL (N,IA,JA,A,RHS,U,WKSP,LEVEL,NOUT,IER)     
      IF (IER.EQ.0) GO TO 190 
      IF (LEVEL.GE.0) WRITE (NOUT,180) IER      
  180 FORMAT ('0','*** F A T A L     E R R O R ************'/'0',   
     *   '    CALLED FROM ITPACK ROUTINE SOR '/' ',       
     *   '    ERROR DETECTED IN SUBROUTINE  SCAL  '/' ',  
     *   '    WHICH SCALES THE SYSTEM   '/' ','    IER = ',I5)      
      GO TO 360   
  190 IF (LEVEL.LE.2) GO TO 220       
      IF (ADAPT) WRITE (NOUT,200)     
  200 FORMAT (///1X,'CME IS THE ESTIMATE OF THE LARGEST EIGENVALUE OF', 
     *   ' THE JACOBI MATRIX')
      WRITE (NOUT,210)      
  210 FORMAT (1X,'OMEGA IS THE RELAXATION FACTOR')
  220 IF (IPARM(11).NE.0) GO TO 230   
      TIMI1 = TIMER(DUMMY)  
C       
C ... ITERATION SEQUENCE    
C       
  230 ITMAX1 = ITMAX+1      
      DO 240 LOOP = 1,ITMAX1
         IN = LOOP-1
C       
C ... CODE FOR ONE ITERATION. 
C       
C     U           = U(IN)   
C       
         CALL ITSOR (N,IA,JA,A,RHS,U,WKSP(IB1)) 
C       
         IF (HALT) GO TO 270
  240 CONTINUE    
C       
C ... ITMAX HAS BEEN REACHED
C       
      IF (IPARM(11).NE.0) GO TO 250   
      TIMI2 = TIMER(DUMMY)  
      TIME1 = DBLE(TIMI2-TIMI1)       
  250 IF (LEVEL.GE.1) WRITE (NOUT,260) ITMAX    
  260 FORMAT ('0','*** W A R N I N G ************'/'0',   
     *   '    IN ITPACK ROUTINE SOR'/' ','    FAILURE TO CONVERGE IN',I5
     *   ,' ITERATIONS')    
      IER = 33    
      IF (IPARM(3).EQ.0) RPARM(1) = STPTST      
      GO TO 300   
C       
C ... METHOD HAS CONVERGED  
C       
  270 IF (IPARM(11).NE.0) GO TO 280   
      TIMI2 = TIMER(DUMMY)  
      TIME1 = DBLE(TIMI2-TIMI1)       
  280 IF (LEVEL.GE.1) WRITE (NOUT,290) IN       
  290 FORMAT (/1X,'SOR  HAS CONVERGED IN ',I5,' ITERATIONS')
C       
C ... UNSCALE THE MATRIX, SOLUTION, AND RHS VECTORS.      
C       
  300 CALL UNSCAL (N,IA,JA,A,RHS,U,WKSP)
C       
C ... UN-PERMUTE MATRIX,RHS, AND SOLUTION       
C       
      IF (IPARM(9).LT.0) GO TO 330    
      CALL PERMAT (N,IA,JA,A,IWKSP(IB2),IWKSP(IB3),ISYM,LEVEL,NOUT, 
     *   IERPER)  
      IF (IERPER.EQ.0) GO TO 320      
      IF (LEVEL.GE.0) WRITE (NOUT,310) IERPER   
  310 FORMAT ('0','*** F A T A L     E R R O R ************'/'0',   
     *   '    CALLED FROM ITPACK ROUTINE SOR '/' ',       
     *   '    ERROR DETECTED IN SUBROUTINE  PERMAT'/' ',  
     *   '    WHICH UNDOES THE RED-BLACK PERMUTATION   '/' ',       
     *   '    IER = ',I5)   
      IF (IER.EQ.0) IER = IERPER      
      GO TO 360   
  320 CALL PERVEC (N,RHS,IWKSP(IB2))  
      CALL PERVEC (N,U,IWKSP(IB2))    
C       
C ... OPTIONAL ERROR ANALYSIS 
C       
  330 IDGTS = IPARM(12)     
      IF (IDGTS.LT.0) GO TO 340       
      IF (IPARM(2).LE.0) IDGTS = 0    
      CALL PERR (N,IA,JA,A,RHS,U,WKSP,DIGIT1,DIGIT2,IDGTS)
C       
C ... SET RETURN PARAMETERS IN IPARM AND RPARM  
C       
  340 IF (IPARM(11).NE.0) GO TO 350   
      TIMJ2 = TIMER(DUMMY)  
      TIME2 = DBLE(TIMJ2-TIMJ1)       
  350 IF (IPARM(3).NE.0) GO TO 360    
      IPARM(1) = IN 
      IPARM(9) = NB 
      RPARM(2) = CME
      RPARM(3) = SME
      RPARM(5) = OMEGA      
      RPARM(9) = TIME1      
      RPARM(10) = TIME2     
      RPARM(11) = DIGIT1    
      RPARM(12) = DIGIT2    
C       
  360 CONTINUE    
      IERR = IER  
      IF (LEVEL.GE.3) CALL ECHALL (N,IA,JA,A,RHS,IPARM,RPARM,2)     
C       
      RETURN      
      END 

      SUBROUTINE ITSOR (NN,IA,JA,A,RHS,U,WK)    
C       
C ... FUNCTION:   
C       
C          THIS SUBROUTINE, ITSOR, PERFORMS ONE ITERATION OF THE    
C          SUCCESSIVE OVERRELAXATION ALGORITHM.  IT IS CALLED BY SOR. 
C       
C ... PARAMETER LIST:       
C       
C          N      INPUT INTEGER.  DIMENSION OF THE MATRIX. (= NN)   
C          IA,JA  INPUT INTEGER VECTORS.  THE TWO INTEGER ARRAYS OF 
C                 THE SPARSE MATRIX REPRESENTATION.       
C          A      INPUT D.P. VECTOR.  THE D.P. ARRAY OF THE SPARSE  
C                 MATRIX REPRESENTATION.
C          RHS    INPUT D.P. VECTOR.  CONTAINS THE RIGHT HAND SIDE  
C                 OF THE MATRIX PROBLEM.
C          U      INPUT/OUTPUT D.P. VECTOR.  ON INPUT, U CONTAINS THE 
C                 SOLUTION VECTOR AFTER IN ITERATIONS.  ON OUTPUT,  
C                 IT WILL CONTAIN THE NEWEST ESTIMATE FOR THE SOLUTION
C                 VECTOR.   
C          WK     D.P. ARRAY.  WORK VECTOR OF LENGTH N.   
C       
C ... SPECIFICATIONS FOR ARGUMENTS    
C       
      INTEGER IA(1),JA(1),NN
      DOUBLE PRECISION A(1),RHS(NN),U(NN),WK(NN)
C       
C ... SPECIFICATIONS FOR LOCAL VARIABLES
C       
      INTEGER IP,IPHAT,IPSTAR,ISS,N   
      DOUBLE PRECISION DNRM,H,OMEGAP,SPCRM1     
      LOGICAL CHANGE,Q1     
C       
      DOUBLE PRECISION TAU  
C       
C *** BEGIN: ITPACK COMMON  
C       
      INTEGER IN,IS,ISYM,ITMAX,LEVEL,NOUT       
      COMMON /ITCOM1/ IN,IS,ISYM,ITMAX,LEVEL,NOUT 
C       
      LOGICAL ADAPT,BETADT,CASEII,HALT,PARTAD   
      COMMON /ITCOM2/ ADAPT,BETADT,CASEII,HALT,PARTAD     
C       
      DOUBLE PRECISION BDELNM,BETAB,CME,DELNNM,DELSNM,FF,GAMMA,OMEGA,QA,
     *   QT,RHO,RRR,SIGE,SME,SPECR,SPR,DRELPR,STPTST,UDNM,ZETA      
      COMMON /ITCOM3/ BDELNM,BETAB,CME,DELNNM,DELSNM,FF,GAMMA,OMEGA,QA, 
     *   QT,RHO,RRR,SIGE,SME,SPECR,SPR,DRELPR,STPTST,UDNM,ZETA      
C       
C *** END  : ITPACK COMMON  
C       
C     DESCRIPTION OF VARIABLES IN COMMON BLOCKS IN SUBROUTINE SOR   
C       
C ... SET INITIAL PARAMETERS NOT ALREADY SET    
C       
      N = NN      
      IF (IN.NE.0) GO TO 20 
      CALL PSTOP (N,U,0.D0,0.D0,0,Q1) 
      IF (ADAPT) GO TO 10   
      CHANGE = .FALSE.      
      IP = 0      
      IPHAT = 2   
      ISS = 0     
      GO TO 30    
C       
   10 CHANGE = .TRUE.       
      IP = 0      
      OMEGAP = OMEGA
      OMEGA = 1.D0
      ISS = 0     
      IPHAT = 2   
      IPSTAR = 4  
      IF (OMEGAP.LE.1.D0) CHANGE = .FALSE.      
C       
C ... RESET OMEGA, IPHAT, AND IPSTAR (CIRCLE A IN FLOWCHART)
C       
   20 IF (.NOT.CHANGE) GO TO 30       
      CHANGE = .FALSE.      
      IS = IS+1   
      IP = 0      
      ISS = 0     
      OMEGA = DMIN1(OMEGAP,TAU(IS))   
      IPHAT = MAX0(3,IFIX(SNGL((OMEGA-1.D0)/(2.D0-OMEGA)))) 
      IPSTAR = IPSTR(OMEGA) 
C       
C ... COMPUTE U (IN + 1) AND NORM OF DEL(S,P) - CIRCLE B IN FLOW CHART
C       
   30 CONTINUE    
      DELSNM = DELNNM       
      SPCRM1 = SPECR
      CALL DCOPY (N,RHS,1,WK,1)       
      CALL PFSOR1 (N,IA,JA,A,U,WK)    
      IF (DELNNM.EQ.0.D0) GO TO 40    
      IF (IN.NE.0) SPECR = DELNNM/DELSNM
      IF (IP.LT.IPHAT) GO TO 70       
C       
C ... STOPPING TEST, SET H  
C       
      IF (SPECR.GE.1.D0) GO TO 70     
      IF (.NOT.(SPECR.GT.(OMEGA-1.D0))) GO TO 40
      H = SPECR   
      GO TO 50    
   40 ISS = ISS+1 
      H = OMEGA-1.D0
C       
C ... PERFORM STOPPING TEST.
C       
   50 CONTINUE    
      DNRM = DELNNM**2      
      CALL PSTOP (N,U,DNRM,H,1,Q1)    
      IF (HALT) GO TO 70    
C       
C ... METHOD HAS NOT CONVERGED YET, TEST FOR CHANGING OMEGA 
C       
      IF (.NOT.ADAPT) GO TO 70
      IF (IP.LT.IPSTAR) GO TO 70      
      IF (OMEGA.GT.1.D0) GO TO 60     
      CME = DSQRT(DABS(SPECR))
      OMEGAP = 2.D0/(1.D0+DSQRT(DABS(1.D0-SPECR)))
      CHANGE = .TRUE.       
      GO TO 70    
   60 IF (ISS.NE.0) GO TO 70
      IF (SPECR.LE.(OMEGA-1.D0)**FF) GO TO 70   
      IF ((SPECR+5.D-5).LE.SPCRM1) GO TO 70     
C       
C ... CHANGE PARAMETERS     
C       
      CME = (SPECR+OMEGA-1.D0)/(DSQRT(DABS(SPECR))*OMEGA) 
      OMEGAP = 2.D0/(1.D0+DSQRT(DABS(1.D0-CME*CME)))      
      CHANGE = .TRUE.       
C       
C ... OUTPUT INTERMEDIATE INFORMATION 
C       
   70 CALL ITERM (N,A,U,WK,3) 
      IP = IP+1   
C       
      RETURN      
      END 
