/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the class library                   */
/*       SoPlex --- the Sequential object-oriented simPlex.                  */
/*                                                                           */
/*    Copyright (C) 1997-1999 Roland Wunderling                              */
/*                  1997-2002 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SoPlex is distributed under the terms of the ZIB Academic Licence.       */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SoPlex; see the file COPYING. If not email to soplex@zib.de.  */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma ident "@(#) $Id: spxgeometsc.h,v 1.3 2003/01/12 13:09:40 bzfkocht Exp $"

/**@file  spxgeometsc.h
 * @brief LP geometric mean scaling.
 */
#ifndef _SPXGEOMETSC_H_
#define _SPXGEOMETSC_H_

#include <assert.h>

#include "spxdefines.h"
#include "spxscaler.h"

namespace soplex
{
/**@brief Geometric mean row/column scaling.
   @ingroup Algo

   This #SPxScaler implementation performs geometric mean scaling of the 
   LPs rows and columns.
*/
class SPxGeometSC : public SPxScaler
{
protected:
   const int  m_maxIterations;    ///< maximum number of scaling iterations.
   const Real m_minImprovement;   ///< improvement nesseccary to carry on.
   const Real m_goodEnoughRatio;  ///< no scaling needed if ratio is less than this.

   ///@return \f$\sqrt{\mbox{mini}\cdot\mbox{maxi}}\f$.
   virtual Real computeScale(Real mini, Real maxi) const;

public:
   /// Scale the loaded #SPxLP.
   virtual void scale(SPxLP& lp);

   /// default constructor.
   explicit SPxGeometSC(bool colFirst = true, int maxIters = 8, Real minImpr = 0.9, Real goodEnough = 1e2);
};
} // namespace soplex
#endif // _SPXGEOMETSC_H_

//-----------------------------------------------------------------------------
//Emacs Local Variables:
//Emacs mode:c++
//Emacs c-basic-offset:3
//Emacs tab-width:8
//Emacs indent-tabs-mode:nil
//Emacs End:
//-----------------------------------------------------------------------------