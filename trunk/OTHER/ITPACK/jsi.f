      SUBROUTINE JSI (NN,IA,JA,A,RHS,U,IWKSP,NW,WKSP,IPARM,RPARM,IERR)
C       
C     ITPACK 2C MAIN SUBROUTINE  JSI  (JACOBI SEMI-ITERATIVE)       
C     EACH OF THE MAIN SUBROUTINES:   
C           JCG, JSI, SOR, SSORCG, SSORSI, RSCG, RSSI     
C     CAN BE USED INDEPENDENTLY OF THE OTHERS   
C       
C ... FUNCTION:   
C       
C          THIS SUBROUTINE, JSI, DRIVES THE JACOBI SEMI-  
C          ITERATION ALGORITHM.       
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
C          WKSP   D.P. VECTOR USED FOR WORKING SPACE.  JACOBI SI    
C                 NEEDS THIS TO BE IN LENGTH AT LEAST     
C                 2*N       
C          IPARM  INTEGER VECTOR OF LENGTH 12.  ALLOWS USER TO SPECIFY
C                 SOME INTEGER PARAMETERS WHICH AFFECT THE METHOD.  
C          RPARM  D.P. VECTOR OF LENGTH 12.  ALLOWS USER TO SPECIFY SOME
C                 D.P. PARAMETERS WHICH AFFECT THE METHOD.
C          IER    OUTPUT INTEGER.  ERROR FLAG. (= IERR)   
C       
C ... JSI SUBPROGRAM REFERENCES:      
C       
C          FROM ITPACK   BISRCH, CHEBY, CHGSI, CHGSME, DFAULT, ECHALL,
C                        ECHOUT, ITERM, TIMER, ITJSI, IVFILL, PAR   
C                        PERMAT, PERR, PERVEC, PJAC, PMULT, PRBNDX, 
C                        PSTOP, PVTBV, QSORT, DAXPY, SBELM, SCAL,   
C                        DCOPY, DDOT, SUM3, TSTCHG, UNSCAL, VEVMW,  
C                        VFILL, VOUT, WEVMW     
C          SYSTEM        DABS, DLOG10, DBLE(AMAX0), DMAX1, DBLE(FLOAT), 
C                        MOD,DSQRT    
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
      INTEGER IB1,IB2,IB3,ICNT,IDGTS,IER,IERPER,ITMAX1,LOOP,N,NB,N3 
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
   10 FORMAT ('0'///1X,'BEGINNING OF ITPACK SOLUTION MODULE  JSI')  
      IER = 0     
      IF (IPARM(1).LE.0) RETURN       
      N = NN      
      IF (IPARM(11).EQ.0) TIMJ1 = TIMER(DUMMY)  
      IF (LEVEL.GE.3) GO TO 20
      CALL ECHOUT (IPARM,RPARM,2)     
      GO TO 30    
   20 CALL ECHALL (N,IA,JA,A,RHS,IPARM,RPARM,1) 
   30 TEMP = 5.0D2*DRELPR   
      IF (ZETA.GE.TEMP) GO TO 50      
      IF (LEVEL.GE.1) WRITE (NOUT,40) ZETA,DRELPR,TEMP    
   40 FORMAT ('0','*** W A R N I N G ************'/'0',   
     *   '    IN ITPACK ROUTINE JSI'/' ','    RPARM(1) =',D10.3,    
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
      IER = 21    
      IF (LEVEL.GE.0) WRITE (NOUT,60) N 
   60 FORMAT ('0','*** F A T A L     E R R O R ************'/'0',   
     *   '    CALLED FROM ITPACK ROUTINE JSI '/' ',       
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
     *   '    CALLED FROM ITPACK ROUTINE JSI '/' ',       
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
      IPARM(8) = 2*N
      IF (NW.GE.IPARM(8)) GO TO 110   
      IER = 22    
      IF (LEVEL.GE.0) WRITE (NOUT,100) NW,IPARM(8)
  100 FORMAT ('0','*** F A T A L     E R R O R ************'/'0',   
     *   '    CALLED FROM ITPACK ROUTINE JSI '/' ',       
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
     *   '    CALLED FROM ITPACK ROUTINE JSI '/' ',       
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
     *   '    CALLED FROM ITPACK ROUTINE JSI '/' ',       
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
     *   '    CALLED FROM ITPACK ROUTINE JSI '/' ',       
     *   '    ERROR DETECTED IN SUBROUTINE  SCAL  '/' ',  
     *   '    WHICH SCALES THE SYSTEM   '/' ','    IER = ',I5)      
      GO TO 360   
  190 IF (LEVEL.LE.2) GO TO 210       
      WRITE (NOUT,200)      
  200 FORMAT (///1X,'IN THE FOLLOWING, RHO AND GAMMA ARE',
     *   ' ACCELERATION PARAMETERS')  
  210 IF (IPARM(11).NE.0) GO TO 220   
      TIMI1 = TIMER(DUMMY)  
C       
C ... ITERATION SEQUENCE    
C       
  220 ITMAX1 = ITMAX+1      
      DO 240 LOOP = 1,ITMAX1
         IN = LOOP-1
         IF (MOD(IN,2).EQ.1) GO TO 230
C       
C ... CODE FOR THE EVEN ITERATIONS.   
C       
C     U           = U(IN)   
C     WKSP(IB1)   = U(IN-1) 
C       
         CALL ITJSI (N,IA,JA,A,RHS,U,WKSP(IB1),WKSP(IB2),ICNT)      
C       
         IF (HALT) GO TO 270
         GO TO 240
C       
C ... CODE FOR THE ODD ITERATIONS.    
C       
C     U           = U(IN-1) 
C     WKSP(IB1)   = U(IN)   
C       
  230    CALL ITJSI (N,IA,JA,A,RHS,WKSP(IB1),U,WKSP(IB2),ICNT)      
C       
         IF (HALT) GO TO 270
  240 CONTINUE    
C       
C ... ITMAX HAS BEEN REACHED
C       
      IF (IPARM(11).NE.0) GO TO 250   
      TIMI2 = TIMER(DUMMY)  
      TIME1 = DBLE(TIMI2-TIMI1)       
  250 IER = 23    
      IF (LEVEL.GE.1) WRITE (NOUT,260) ITMAX    
  260 FORMAT ('0','*** W A R N I N G ************'/'0',   
     *   '    IN ITPACK ROUTINE JSI'/' ','    FAILURE TO CONVERGE IN',I5
     *   ,' ITERATIONS')    
      IF (IPARM(3).EQ.0) RPARM(1) = STPTST      
      GO TO 300   
C       
C ... METHOD HAS CONVERGED  
C       
  270 IF (IPARM(11).NE.0) GO TO 280   
      TIMI2 = TIMER(DUMMY)  
      TIME1 = DBLE(TIMI2-TIMI1)       
  280 IF (LEVEL.GE.1) WRITE (NOUT,290) IN       
  290 FORMAT (/1X,'JSI  HAS CONVERGED IN ',I5,' ITERATIONS')
C       
C ... PUT SOLUTION INTO U IF NOT ALREADY THERE. 
C       
  300 CONTINUE    
      IF (MOD(IN,2).EQ.1) CALL DCOPY (N,WKSP(IB1),1,U,1)  
C       
C ... UNSCALE THE MATRIX, SOLUTION, AND RHS VECTORS.      
C       
      CALL UNSCAL (N,IA,JA,A,RHS,U,WKSP)
C       
C ... UN-PERMUTE MATRIX,RHS, AND SOLUTION       
C       
      IF (IPARM(9).LT.0) GO TO 330    
      CALL PERMAT (N,IA,JA,A,IWKSP(IB2),IWKSP(IB3),ISYM,LEVEL,NOUT, 
     *   IERPER)  
      IF (IERPER.EQ.0) GO TO 320      
      IF (LEVEL.GE.0) WRITE (NOUT,310) IERPER   
  310 FORMAT ('0','*** F A T A L     E R R O R ************'/'0',   
     *   '    CALLED FROM ITPACK ROUTINE JSI '/' ',       
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

      SUBROUTINE ITJSI (NN,IA,JA,A,RHS,U,U1,D,ICNT)       
C       
C ... FUNCTION:   
C       
C          THIS SUBROUTINE, ITJSI, PERFORMS ONE ITERATION OF THE    
C          JACOBI SEMI-ITERATIVE ALGORITHM.  IT IS CALLED BY JSI.   
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
C          U      INPUT D.P. VECTOR.  CONTAINS THE ESTIMATE FOR THE 
C                 SOLUTION VECTOR AFTER IN ITERATIONS.    
C          U1     INPUT/OUTPUT D.P. VECTOR.  ON INPUT, U1 CONTAINS THE
C                 SOLUTION VECTOR AFTER IN-1 ITERATIONS.  ON OUTPUT,
C                 IT WILL CONTAIN THE NEWEST ESTIMATE FOR THE SOLUTION
C                 VECTOR.   
C          D      D.P. ARRAY.  D IS USED FOR THE COMPUTATION OF THE 
C                 PSEUDO-RESIDUAL ARRAY FOR THE CURRENT ITERATION.  
C          ICNT   NUMBER OF ITERATIONS SINCE LAST CHANGE OF SME     
C       
C ... SPECIFICATIONS OF ARGUMENTS     
C       
      INTEGER IA(1),JA(1),NN,ICNT     
      DOUBLE PRECISION A(1),RHS(NN),U(NN),U1(NN),D(NN)    
C       
C ... SPECIFICATIONS OF LOCAL VARIABLES 
C       
      INTEGER N   
      DOUBLE PRECISION CON,C1,C2,C3,DNRM,DTNRM,OLDNRM     
      LOGICAL Q1  
C       
C ... SPECIFICATIONS OF FUNCTION SUBPROGRAMS    
C       
      DOUBLE PRECISION DDOT,PVTBV     
      LOGICAL TSTCHG,CHGSME 
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
C     DESCRIPTION OF VARIABLES IN COMMON BLOCKS IN SUBROUTINE JSI   
C       
      N = NN      
      IF (IN.EQ.0) ICNT = 0 
C       
C ... COMPUTE PSEUDO-RESIDUALS
C       
      CALL DCOPY (N,RHS,1,D,1)
      CALL PJAC (N,IA,JA,A,U,D)       
      CALL VEVMW (N,D,U)    
C       
C ... STOPPING AND ADAPTIVE CHANGE TESTS
C       
      OLDNRM = DELNNM       
      DELNNM = DDOT(N,D,1,D,1)
      DNRM = DELNNM 
      CON = CME   
      CALL PSTOP (N,U,DNRM,CON,1,Q1)  
      IF (HALT) GO TO 40    
      IF (.NOT.ADAPT) GO TO 30
      IF (.NOT.TSTCHG(1)) GO TO 10    
C       
C ... CHANGE ITERATIVE PARAMETERS (CME) 
C       
      DTNRM = PVTBV(N,IA,JA,A,D)      
      CALL CHGSI (DTNRM,1)  
      IF (.NOT.ADAPT) GO TO 30
      GO TO 20    
C       
C ... TEST IF SME NEEDS TO BE CHANGED AND CHANGE IF NECESSARY.      
C       
   10 CONTINUE    
      IF (CASEII) GO TO 30  
      IF (.NOT.CHGSME(OLDNRM,ICNT)) GO TO 30    
      ICNT = 0    
C       
C ... COMPUTE U(IN+1) AFTER CHANGE OF PARAMETERS
C       
   20 CALL DCOPY (N,U,1,U1,1) 
      CALL DAXPY (N,GAMMA,D,1,U1,1)   
      GO TO 40    
C       
C ... COMPUTE U(IN+1) WITHOUT CHANGE OF PARAMETERS
C       
   30 CALL PARSI (C1,C2,C3,1) 
      CALL SUM3 (N,C1,D,C2,U,C3,U1)   
C       
C ... OUTPUT INTERMEDIATE INFORMATION 
C       
   40 CALL ITERM (N,A,U,D,2)
C       
      RETURN      
      END 
