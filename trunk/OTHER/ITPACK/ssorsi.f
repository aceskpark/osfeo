      SUBROUTINE SSORSI (NN,IA,JA,A,RHS,U,IWKSP,NW,WKSP,IPARM,RPARM,
     *   IERR)    
C       
C     ITPACK 2C MAIN SUBROUTINE  SSORSI  (SYMMETRIC SUCCESSIVE RELAX- 
C                                         ATION SEMI-ITERATION)     
C     EACH OF THE MAIN SUBROUTINES:   
C           JCG, JSI, SOR, SSORCG, SSORSI, RSCG, RSSI     
C     CAN BE USED INDEPENDENTLY OF THE OTHERS   
C       
C ... FUNCTION:   
C       
C          THIS SUBROUTINE, SSORSI, DRIVES THE  SYMMETRIC SOR-SI    
C          ALGORITHM.       
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
C                 INITIAL GUESS TO THE SOLUTION. ON OUTPUT, IT CONTAINS 
C                 THE LATEST ESTIMATE TO THE SOLUTION.    
C          IWKSP  INTEGER VECTOR WORKSPACE OF LENGTH 3*N  
C          NW     INPUT INTEGER.  LENGTH OF AVAILABLE WKSP.  ON OUTPUT, 
C                 IPARM(8) IS AMOUNT USED.      
C          WKSP   D.P. VECTOR USED FOR WORKING SPACE.  SSORSI       
C                 NEEDS THIS TO BE IN LENGTH AT LEAST  5*N
C          IPARM  INTEGER VECTOR OF LENGTH 12.  ALLOWS USER TO SPECIFY
C                 SOME INTEGER PARAMETERS WHICH AFFECT THE METHOD.  IF
C          RPARM  D.P. VECTOR OF LENGTH 12. ALLOWS USER TO SPECIFY SOME 
C                 D.P. PARAMETERS WHICH AFFECT THE METHOD.
C          IER    OUTPUT INTEGER.  ERROR FLAG. (= IERR)   
C       
C ... SSORSI SUBPROGRAM REFERENCES:   
C       
C          FROM ITPACK    BISRCH, CHEBY, CHGSI, DFAULT, ECHALL, ECHOUT, 
C                         ITERM, TIMER, ITSRSI, IVFILL, OMEG,       
C                         OMGSTR, PARSI, PBETA, PERMAT, PERR,     
C                         PERVEC, PFSOR, PMULT, PRBNDX, PSSOR1,     
C                         PSTOP, PVTBV, QSORT, SBELM, SCAL, DCOPY,  
C                         DDOT, SUM3, TSTCHG, UNSCAL, VEVPW, VFILL, 
C                         VOUT, WEVMW 
C          SYSTEM         DABS, DLOG, DLOG10, DBLE(AMAX0), DMAX1, DBLE(F
C                         MOD, DSQRT  
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
      INTEGER IB1,IB2,IB3,IB4,IB5,IDGTS,IER,IERPER,ITMAX1,LOOP,N,NB,N3
      DOUBLE PRECISION BETNEW,DIGIT1,DIGIT2,PBETA,TEMP,TIME1,TIME2,TOL
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
C     ISYM   - SYMMETRIC/NONSYMMETRIC STORAGE FORMAT SWITCH 
C     IS     - ITERATION NUMBER WHEN PARAMETERS LAST CHANGED
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
      IF (IPARM(9).GE.0) IPARM(6) = 2 
      IF (LEVEL.GE.1) WRITE (NOUT,10) 
   10 FORMAT ('0'///1X,'BEGINNING OF ITPACK SOLUTION MODULE  SSORSI') 
      IER = 0     
      IF (IPARM(1).LE.0) RETURN       
      N = NN      
      IF (IPARM(11).EQ.0) TIMJ1 = TIMER(DUMMY)  
      IF (LEVEL.GE.3) GO TO 20
      CALL ECHOUT (IPARM,RPARM,5)     
      GO TO 30    
   20 CALL ECHALL (N,IA,JA,A,RHS,IPARM,RPARM,1) 
   30 TEMP = 5.0D2*DRELPR   
      IF (ZETA.GE.TEMP) GO TO 50      
      IF (LEVEL.GE.1) WRITE (NOUT,40) ZETA,DRELPR,TEMP    
   40 FORMAT ('0','*** W A R N I N G ************'/'0',   
     *   '    IN ITPACK ROUTINE SSORSI'/' ','    RPARM(1) =',D10.3, 
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
      IER = 51    
      IF (LEVEL.GE.0) WRITE (NOUT,60) N 
   60 FORMAT ('0','*** F A T A L     E R R O R ************'/'0',   
     *   '    CALLED FROM ITPACK ROUTINE SSORSI '/' ',    
     *   '    INVALID MATRIX DIMENSION, N =',I8)
      GO TO 380   
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
     *   '    CALLED FROM ITPACK ROUTINE SSORSI '/' ',    
     *   '    ERROR DETECTED IN SUBROUTINE  SBELM '/' ',  
     *   '    WHICH REMOVES ROWS AND COLUMNS OF SYSTEM '/' ',       
     *   '    WHEN DIAGONAL ENTRY TOO LARGE  '/' ','    IER = ',I5,5X,
     *   ' RPARM(8) = ',D10.3,' (TOL)') 
      GO TO 380   
C       
C ... INITIALIZE WKSP BASE ADDRESSES. 
C       
   90 IB1 = 1     
      IB2 = IB1+N 
      IB3 = IB2+N 
      IB4 = IB3+N 
      IB5 = IB4+N 
      IPARM(8) = 5*N
      IF (NW.GE.IPARM(8)) GO TO 110   
      IER = 52    
      IF (LEVEL.GE.0) WRITE (NOUT,100) NW,IPARM(8)
  100 FORMAT ('0','*** F A T A L     E R R O R ************'/'0',   
     *   '    CALLED FROM ITPACK ROUTINE SSORSI '/' ',    
     *   '    NOT ENOUGH WORKSPACE AT ',I10/' ','    SET IPARM(8) =',I10
     *   ,' (NW)')
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
     *   '    CALLED FROM ITPACK ROUTINE SSORSI '/' ',    
     *   '    ERROR DETECTED IN SUBROUTINE  PRBNDX'/' ',  
     *   '    WHICH COMPUTES THE RED-BLACK INDEXING'/' ','    IER = ',I5
     *   ,' IPARM(9) = ',I5,' (NB)')  
      GO TO 380   
C       
C ... PERMUTE MATRIX AND RHS
C       
  130 IF (LEVEL.GE.2) WRITE (NOUT,140) NB       
  140 FORMAT (/10X,'ORDER OF BLACK SUBSYSTEM = ',I5,' (NB)')
      CALL PERMAT (N,IA,JA,A,IWKSP,IWKSP(IB3),ISYM,LEVEL,NOUT,IER)  
      IF (IER.EQ.0) GO TO 160 
      IF (LEVEL.GE.0) WRITE (NOUT,150) IER      
  150 FORMAT ('0','*** F A T A L     E R R O R ************'/'0',   
     *   '    CALLED FROM ITPACK ROUTINE SSORSI '/' ',    
     *   '    ERROR DETECTED IN SUBROUTINE  PERMAT'/' ',  
     *   '    WHICH DOES THE RED-BLACK PERMUTATION'/' ','    IER = ',I5)
      GO TO 380   
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
     *   '    CALLED FROM ITPACK ROUTINE SSORSI '/' ',    
     *   '    ERROR DETECTED IN SUBROUTINE  SCAL  '/' ',  
     *   '    WHICH SCALES THE SYSTEM   '/' ','    IER = ',I5)      
      GO TO 380   
  190 IF (LEVEL.LE.2) GO TO 210       
      WRITE (NOUT,200)      
  200 FORMAT (///1X,'IN THE FOLLOWING, RHO AND GAMMA ARE',
     *   ' ACCELERATION PARAMETERS')  
  210 IF (IPARM(11).NE.0) GO TO 220   
      TIMI1 = TIMER(DUMMY)  
C       
C ... SPECIAL PROCEDURE FOR FULLY ADAPTIVE CASE.
C       
  220 CONTINUE    
      IF (.NOT.ADAPT) GO TO 240       
      IF (.NOT.BETADT) GO TO 230      
      CALL VFILL (N,WKSP(IB1),1.D0)   
      BETNEW = PBETA(N,IA,JA,A,WKSP(IB1),WKSP(IB2),WKSP(IB3))/      
     *   DBLE(FLOAT(N))     
      BETAB = DMAX1(BETAB,.25D0,BETNEW) 
  230 CALL OMEG (0.D0,1)    
      IS = 0      
C       
C ... ITERATION SEQUENCE    
C       
  240 ITMAX1 = ITMAX+1      
      DO 260 LOOP = 1,ITMAX1
         IN = LOOP-1
         IF (MOD(IN,2).EQ.1) GO TO 250
C       
C ... CODE FOR THE EVEN ITERATIONS.   
C       
C     U           = U(IN)   
C     WKSP(IB1)   = U(IN-1) 
C       
         CALL ITSRSI (N,IA,JA,A,RHS,U,WKSP(IB1),WKSP(IB2),WKSP(IB3),
     *      WKSP(IB4),WKSP(IB5))      
C       
         IF (HALT) GO TO 290
         GO TO 260
C       
C ... CODE FOR THE ODD ITERATIONS.    
C       
C     U           = U(IN-1) 
C     WKSP(IB1)   = U(IN)   
C       
  250    CALL ITSRSI (N,IA,JA,A,RHS,WKSP(IB1),U,WKSP(IB2),WKSP(IB3),
     *      WKSP(IB4),WKSP(IB5))      
C       
         IF (HALT) GO TO 290
  260 CONTINUE    
C       
C ... ITMAX HAS BEEN REACHED
C       
      IF (IPARM(11).NE.0) GO TO 270   
      TIMI2 = TIMER(DUMMY)  
      TIME1 = DBLE(TIMI2-TIMI1)       
  270 IF (LEVEL.GE.1) WRITE (NOUT,280) ITMAX    
  280 FORMAT ('0','*** W A R N I N G ************'/'0',   
     *   '    IN ITPACK ROUTINE SSORSI'/' ','    FAILURE TO CONVERGE IN'
     *   ,I5,' ITERATIONS') 
      IER = 53    
      IF (IPARM(3).EQ.0) RPARM(1) = STPTST      
      GO TO 320   
C       
C ... METHOD HAS CONVERGED  
C       
  290 IF (IPARM(11).NE.0) GO TO 300   
      TIMI2 = TIMER(DUMMY)  
      TIME1 = DBLE(TIMI2-TIMI1)       
  300 IF (LEVEL.GE.1) WRITE (NOUT,310) IN       
  310 FORMAT (/1X,'SSORSI  HAS CONVERGED IN ',I5,' ITERATIONS')     
C       
C ... PUT SOLUTION INTO U IF NOT ALREADY THERE. 
C       
  320 CONTINUE    
      IF (MOD(IN,2).EQ.1) CALL DCOPY (N,WKSP(IB1),1,U,1)  
C       
C ... UNSCALE THE MATRIX, SOLUTION, AND RHS VECTORS.      
C       
      CALL UNSCAL (N,IA,JA,A,RHS,U,WKSP)
C       
C ... UN-PERMUTE MATRIX,RHS, AND SOLUTION       
C       
      IF (IPARM(9).LT.0) GO TO 350    
      CALL PERMAT (N,IA,JA,A,IWKSP(IB2),IWKSP(IB3),ISYM,LEVEL,NOUT, 
     *   IERPER)  
      IF (IERPER.EQ.0) GO TO 340      
      IF (LEVEL.GE.0) WRITE (NOUT,330) IERPER   
  330 FORMAT ('0','*** F A T A L     E R R O R ************'/'0',   
     *   '    CALLED FROM ITPACK ROUTINE SSORSI '/' ',    
     *   '    ERROR DETECTED IN SUBROUTINE  PERMAT'/' ',  
     *   '    WHICH UNDOES THE RED-BLACK PERMUTATION   '/' ',       
     *   '    IER = ',I5)   
      IF (IER.EQ.0) IER = IERPER      
      GO TO 380   
  340 CALL PERVEC (N,RHS,IWKSP(IB2))  
      CALL PERVEC (N,U,IWKSP(IB2))    
C       
C ... OPTIONAL ERROR ANALYSIS 
C       
  350 IDGTS = IPARM(12)     
      IF (IDGTS.LT.0) GO TO 360       
      IF (IPARM(2).LE.0) IDGTS = 0    
      CALL PERR (N,IA,JA,A,RHS,U,WKSP,DIGIT1,DIGIT2,IDGTS)
C       
C ... SET RETURN PARAMETERS IN IPARM AND RPARM  
C       
  360 IF (IPARM(11).NE.0) GO TO 370   
      TIMJ2 = TIMER(DUMMY)  
      TIME2 = DBLE(TIMJ2-TIMJ1)       
  370 IF (IPARM(3).NE.0) GO TO 380    
      IPARM(1) = IN 
      IPARM(9) = NB 
      RPARM(2) = CME
      RPARM(3) = SME
      RPARM(5) = OMEGA      
      RPARM(6) = SPECR      
      RPARM(7) = BETAB      
      RPARM(9) = TIME1      
      RPARM(10) = TIME2     
      RPARM(11) = DIGIT1    
      RPARM(12) = DIGIT2    
C       
  380 CONTINUE    
      IERR = IER  
      IF (LEVEL.GE.3) CALL ECHALL (N,IA,JA,A,RHS,IPARM,RPARM,2)     
C       
      RETURN      
      END 

      SUBROUTINE ITSRSI (NN,IA,JA,A,RHS,U,U1,C,D,CTWD,WK) 
C       
C ... FUNCTION:   
C       
C          THIS SUBROUTINE, ITSRSI, PERFORMS ONE ITERATION OF THE   
C          SYMMETRIC SOR SEMI-ITERATION ALGORITHM.  IT IS CALLED BY 
C          SSORSI.
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
C          U      INPUT D.P. VECTOR.  CONTAINS THE ESTIMATE OF THE  
C                 SOLUTION VECTOR AFTER IN ITERATIONS.    
C          U1     INPUT/OUTPUT D.P. VECTOR.  ON INPUT, U1 CONTAINS THE
C                 THE ESTIMATE FOR THE SOLUTION AFTER IN-1 ITERATIONS.
C                 ON OUTPUT, U1 CONTAINS THE UPDATED ESTIMATE.      
C          C      D.P. VECTOR.  IS USED TO COMPUTE THE FORWARD PSEUDO-
C                 RESIDUAL VECTOR FOR THE CURRENT ITERATION.
C          D      D.P. VECTOR.  IS USED TO COMPUTE THE BACKWARD PSEUDO- 
C                 RESIDUAL VECTOR FOR THE CURRENT ITERATION.
C          CTWD   D.P. VECTOR.  IS USED IN THE COMPUTATIONS OF THE  
C                 ACCELERATION PARAMETERS.      
C          WK     D.P. VECTOR.  WORKING SPACE OF LENGTH N.
C       
C ... SPECIFICATIONS FOR ARGUMENTS    
C       
      INTEGER IA(1),JA(1),NN
      DOUBLE PRECISION A(1),RHS(NN),U(NN),U1(NN),C(NN),D(NN),WK(NN),
     *   CTWD(NN) 
C       
C ... SPECIFICATIONS FOR LOCAL VARIABLES
C       
      INTEGER N   
      DOUBLE PRECISION BETNEW,CON,C1,C2,C3,DNRM 
      LOGICAL Q1  
C       
C ... SPECIFICATIONS FOR FUNCTION SUBPROGRAMS   
C       
      DOUBLE PRECISION DDOT,PBETA,PVTBV 
      LOGICAL OMGSTR,TSTCHG 
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
C     DESCRIPTION OF VARIABLES IN COMMON BLOCKS IN SUBROUTINE SSORSI
C       
C ... COMPUTE PSEUDO-RESIDUALS (FORWARD AND BACKWARD)     
C       
      N = NN      
      CALL DCOPY (N,RHS,1,WK,1)       
      CALL DCOPY (N,U,1,CTWD,1)       
      CALL PSSOR1 (N,IA,JA,A,CTWD,WK,C,D)       
C       
C ... COMPUTE U(IN+1) -- CONTAINED IN THE VECTOR U1.      
C       
      CALL PARSI (C1,C2,C3,3) 
      CALL SUM3 (N,C1,D,C2,U,C3,U1)   
C       
C ... TEST FOR STOPPING     
C       
      BDELNM = DDOT(N,D,1,D,1)
      DNRM = BDELNM 
      CON = SPECR 
      CALL PSTOP (N,U,DNRM,CON,1,Q1)  
      IF (HALT.OR..NOT.(ADAPT.OR.PARTAD)) GO TO 40
C       
C ... ADAPTIVE PROCEDURE    
C       
      IF (OMGSTR(1)) GO TO 40 
      DELNNM = DDOT(N,C,1,C,1)
      IF (IN.EQ.IS) DELSNM = DELNNM   
      IF (IN.EQ.0.OR..NOT.TSTCHG(1)) GO TO 40   
C       
C ... IT HAS BEEN DECIDED TO CHANGE PARAMETERS. 
C ...    (1) COMPUTE CTWD   
C       
      CALL DCOPY (N,D,1,CTWD,1)       
      CALL VFILL (N,WK,0.D0)
      CALL PFSOR (N,IA,JA,A,CTWD,WK)  
      CALL VEVPW (N,CTWD,C) 
      CALL VEVMW (N,CTWD,D) 
C       
C ...    (2) COMPUTE NEW SPECTRAL RADIUS FOR CURRENT OMEGA. 
C       
      DNRM = DDOT(N,C,1,CTWD,1)       
      CALL CHGSI (DNRM,3)   
      IF (.NOT.ADAPT) GO TO 40
C       
C ...    (3) COMPUTE NEW BETAB IF BETADT = .TRUE. 
C       
      IF (.NOT.BETADT) GO TO 10       
      BETNEW = PBETA(N,IA,JA,A,D,WK,CTWD)/BDELNM
      BETAB = DMAX1(BETAB,.25D0,BETNEW) 
C       
C ...    (4) COMPUTE NEW CME, OMEGA, AND SPECR. 
C       
   10 CONTINUE    
      IF (CASEII) GO TO 20  
      DNRM = PVTBV(N,IA,JA,A,D)       
      GO TO 30    
   20 CALL VFILL (N,WK,0.D0)
      CALL PJAC (N,IA,JA,A,D,WK)      
      DNRM = DDOT(N,WK,1,WK,1)
   30 CALL OMEG (DNRM,3)    
C       
C ... OUTPUT INTERMEDIATE INFORMATION 
C       
   40 CALL ITERM (N,A,U,WK,5) 
C       
      RETURN      
      END 
