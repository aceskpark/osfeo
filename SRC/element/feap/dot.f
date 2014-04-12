c$Id: dot.f,v 1.2 2004/03/12 20:52:23 rlt Exp $
      function   dot(a,b,n)

c      * * F E A P * * A Finite Element Analysis Program

c....  Copyright (c) 1984-2004: Regents of the University of California
c                               All rights reserved

c-----[--.----+----.----+----.-----------------------------------------]
c      Purpose: dot (scalar) product of two vectors

c      Inputs:
c         a(*)  - Vector 1
c         b(*)  - Vector 2
c         nn    - length of vectors

c      Outputs:
c         dot   - Scalar product
c-----[--.----+----.----+----.-----------------------------------------]
      implicit none

      integer  i,n
      real*8   dot, a(*),b(*)

      save

      dot = 0.0d0
      do i = 1,n
        dot = dot + a(i)*b(i)
      end do ! i

      end
