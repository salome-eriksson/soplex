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
#pragma ident "@(#) $Id: lpstat.cpp,v 1.2 2003/01/05 17:50:15 bzfkocht Exp $"

#include <assert.h>
#include <iostream>
#include <fstream>

#include "spxdefines.h"
#include "spxlp.h"

using namespace soplex;

int main(int argc, char **argv)
{
   const char* banner =
   "************************************************************************\n"
   "*                                                                      *\n"
   "*       LPStat --- Print Statistics about LPs.                         *\n"
   "*                  Release 1.0.0                                       *\n"
   "*    Copyright (C) 2002 Konrad-Zuse-Zentrum                            *\n"
   "*                       fuer Informationstechnik Berlin                *\n"
   "*                                                                      *\n"
   "*  LPStat is distributed under the terms of the ZIB Academic Licence.  *\n"
   "*  You should have received a copy of the ZIB Academic License         *\n"
   "*  along with SoPlex; If not email to soplex@zib.de.                   *\n"
   "*                                                                      *\n"
   "************************************************************************\n"
   ;

   const char* usage =
   "[options] input-file\n\n"
   "          input-file can be either in MPS or LPF format\n\n"
   "options:  (*) indicates default\n" 
   " -vLevel   set verbosity Level [0-3], default 1\n"
   " -V        show program version\n"
   " -h        show this help\n"
   ;

   int verbose = 1;
   int optidx;

   for(optidx = 1; optidx < argc; optidx++)
   {
      if (*argv[optidx] != '-')
         break;

      switch(argv[optidx][1])
      {
      case 'v' :
         verbose = atoi(&argv[optidx][2]);
         break;
      case 'V' :
         std::cout << banner << std::endl;
         exit(0);
      case 'h' :
      case '?' :
         std::cout << banner << std::endl;
         /*FALLTHROUGH*/
      default :
         std::cerr << "usage: " << argv[0] << " " << usage << std::endl;
         exit(0);
      }
   }
   if ((argc - optidx) < 1)
   {
      std::cerr << "usage: " << argv[0] << " " << usage << std::endl;
      exit(0);
   }
   const char* inpfile  = argv[optidx];

   Param::setVerbose(verbose);

   SPxLP       lp;
   NameSet     rownames;
   NameSet     colnames;
   DIdxSet     intvars;

   std::ifstream ifile(inpfile);

   if (!ifile)
   {
      std::cerr << "Can't open file: " << inpfile << std::endl;
      exit(1);
   }
   if (!lp.read(ifile, &rownames, &colnames, &intvars))
   {
      std::cerr << "Error while reading file: " << inpfile << std::endl;
      exit(1);
   }
   //
   DataArray<bool> is_int(lp.nCols());
   DataArray<bool> is_bin(lp.nCols());

   for(int i = 0; i < lp.nCols(); ++i)
   {
      is_int[i] = false;
      is_bin[i] = false;
   }
   //
   int vars  = lp.nCols();
   int vbins = 0;
   int vints = 0;
   int boxed = 0;
   int frees = 0;

   for(int i = 0; i < lp.nCols(); ++i)
   {
      if (lp.lower(i) > -infinity && lp.upper(i) < infinity)
         boxed++;
      else if (lp.lower(i) <= -infinity && lp.upper(i) >= infinity)
         frees++;
   }
   for(int i = 0; i < intvars.size(); ++i)
   {
      int j = intvars.index(i);

      if (EQ(lp.lower(j), 0.0) && EQ(lp.upper(j), 1.0))
      {
         is_bin[j] = true;
         vbins++;
      }
      else
      {
         is_int[j] = true;
         vints++;
      }
   }
   std::cout << "Variables  : " << std::setw(8) << vars  << std::endl
             << "     binary: " << std::setw(8) << vbins << std::endl
             << "    integer: " << std::setw(8) << vints << std::endl
             << "      boxed: " << std::setw(8) << boxed << std::endl
             << "       free: " << std::setw(8) << frees << std::endl
             << std::endl;
   //
   int cons  = lp.nRows();
   int cbins = 0;
   int cints = 0;
   int mixed = 0;
   int conts = 0;
   int rngs  = 0;
   int equls = 0;
   int grets = 0;
   int lests = 0;
   int bin28 = 0;
   int sos3s = 0;

   for(int i = 0; i < lp.nRows(); ++i)
   {
      if (lp.lhs(i) == lp.rhs(i))
         equls++;
      else if (lp.lhs(i) > -infinity && lp.rhs(i) < infinity)
         rngs++;
      else if (lp.lhs(i) > -infinity)
         grets++;
      else
         lests++;

      SVector row     = lp.rowVector(i);
      int     bin_cnt = 0;
      int     int_cnt = 0;
      int     con_cnt = 0;

      for(int j = 0; j < row.size(); ++j)
      {
         int k = row.index(j);
                  
         if (is_int[k])
            int_cnt++;
         else if (is_bin[k])
            bin_cnt++;
         else
            con_cnt++;
      }
      if (int_cnt == 0 && con_cnt == 0 && bin_cnt > 0)
      {
         if (lp.rhs(i) == 1.0)
            sos3s++;
         else
         {
            cbins++;

            if (bin_cnt <= 28)
               bin28++;
         }
      }
      else if (int_cnt > 0 && con_cnt == 0)
         cints++;
      else if (int_cnt == 0 && con_cnt > 0 && bin_cnt == 0)
         conts++;
      else
         mixed++;
   }
   std::cout << "Constraints: " << std::setw(8) << cons  << std::endl
             << "     binary: " << std::setw(8) << cbins << " " << bin28 << std::endl
             << "     SOS-T3: " << std::setw(8) << sos3s << std::endl
             << "    integer: " << std::setw(8) << cints << std::endl
             << "      mixed: " << std::setw(8) << mixed << std::endl
             << "  continous: " << std::setw(8) << conts << std::endl
             << "         ==: " << std::setw(8) << equls << std::endl
             << "         >=: " << std::setw(8) << grets << std::endl
             << "         <=: " << std::setw(8) << lests << std::endl
             << "     ranged: " << std::setw(8) << rngs  << std::endl
             << std::endl;

   return 0;
}




